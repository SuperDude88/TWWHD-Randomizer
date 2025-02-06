"""
Custom assembler wrapper.

This script is used for invoking the PowerPC assembler used for generating
assembly patches, and then performing the necessary changes to ELF relocations
to load the code.
"""

import struct
import re
import traceback
import yaml

import glob
import tempfile
import shutil
import os
import sys
import subprocess

import elf

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Iterable, Tuple

FREE_SPACE_START = 0x028F87F4
TEXT_START = 0x02000000
DATA_START = 0x10000000

def find_tool(name: str) -> Path:
  if sys.platform == "win32":
    name += ".exe"

  if "DEVKITPPC" in os.environ:
    dkp = Path(os.environ.get("DEVKITPPC"))
  elif sys.platform == "win32":
    dkp = Path(r"C:\devkitPro\devkitPPC")
  else:
    raise AssertionError("could not find DevKitPro, which must be defined in $DEVKITPPC or found at C:\devkitPro\devkitPPC")

  path = dkp / 'bin' / name
  assert path.exists(), "could not find {path}"
  return path

def run(tool: str, args: List[Any], *, input: str = None) -> bytes:
  """
  Runs a command and returns its stdout.
  """
  if isinstance(input, str):
    input = input.encode('utf-8')
  
  for i, arg in enumerate(args):
    if not isinstance(arg, Path):
      continue
    try:
      arg = arg.relative_to(TEMP)
    except ValueError:
      if not arg.is_absolute():
        arg = arg.absolute
    args[i] = arg

  print(tool, *args)
  result = subprocess.run(
    [find_tool(tool)] + args,
    cwd=TEMP,
    input=input,
    capture_output=True,
  )

  if result.returncode != 0:
    stderr = result.stderr
    if isinstance(stderr, bytes):
      stderr = stderr.decode('utf-8')

    raise Exception(f"error running {tool}. stderr:\n{stderr}")
  return result.stdout

def preprocess(path: Path) -> Path:
  """
  Executes the C preprocessor on the given assembly file text.
  Returns a path to the result.
  """
  out = TEMP / (path.name + ".i")

  # GCC does not like the .asm suffix. It really wants it to be .S, but we
  # use .asm, so we need to specify the language with -x.
  run("powerpc-eabi-gcc", ["-E", "-x", "assembler-with-cpp", "-o", out, path])
  return out

COMMENT_PAT = re.compile(r"[#;].*$")
ORG_HEX_PAT = re.compile(r"\.org\s+0x([0-9a-fA-F]+)$")
ORG_FREE_PAT = re.compile(r"\.org\s+@NextFreeSpace$")
BRANCH_PAT  = re.compile(r"(?:b|beq|bne|blt|bgt|ble|bge)\s+0x([0-9a-fA-F]+)(?:$|\s)")

TEMP = Path(tempfile.mkdtemp())

CPP_FLAGS = []

ASSEMBLY_FLAGS = [
  "-mregnames", # Allow naming registers r0, r1, r2, etc. This is a PPC quirk.
  "-x", "assembler", # We use .asm instead of .S; this confuses GCC.
]

@dataclass
class Chunk:
  source: Path
  tag: int = None

  origin: int = None # None -> NextFreeSpace.
  code: str = ""
  labels: Dict[str, int] = field(default_factory=dict)

  gcc_flags: List[str] = field(default_factory=list)

  _object_path: Path = None
  _object: elf.ELF = None

  @staticmethod
  def parse(path: Path) -> List["Chunk"]:
    """
    Parses this file into one or more code chunks, each of which may or may
    not have their own start offset.
    """

    if path.suffix in [".asm", ".S"]:
      return Chunk.parse_asm_file(path)
    else:
      return [Chunk.parse_cpp_file(path)]

  @staticmethod
  def parse_cpp_file(path: Path) -> "Chunk":
    """
    Parses a C/C++ file into a single code chunk.
    """

    return Chunk(
      source=path,
      code=path.read_text(),
      gcc_flags=CPP_FLAGS + (['-std=c++20'] if path.suffix != '.c' else ['-std=c17'])
    )

  @staticmethod
  def parse_asm_file(path: Path) -> List["Chunk"]:
    """
    Parses an assembly file into code chunks.
    """
    index = 0
    chunks: List[Chunk] = []
    for line in preprocess(path).read_text().splitlines():
      # Vaporize any comments on this line.
      line = COMMENT_PAT.sub("", line).strip()
      if line == "":
        # Blank line
        continue

      match = ORG_HEX_PAT.match(line)
      if match:
        offset = int(match.group(1), 16)

        assert offset >= DATA_START or offset < FREE_SPACE_START, \
          f"tried to manually set the origin point to after the start of free space at {offset:#x}; use \".org @NextFreeSpace\" instead to get an automatically assigned free space offset"
        
        chunks.append(Chunk(source=path, tag=f"{offset:#x}", origin=offset))
        index += 1
        continue

      match = ORG_FREE_PAT.match(line)
      if match:
        chunks.append(Chunk(source=path, tag=f"block{index}", origin=None))
        index += 1
        continue

      assert chunks, f"found code before an .org directive"

      match = BRANCH_PAT.match(line)
      if match:
        # Replace branches to specific addresses with labels, and define the
        # address of those labels in the linker script.
        dst = int(match.group(1), 16)
        label = f".L_{dst:X}"
        chunks[-1].labels[label] = dst
        line = line.replace("0x" + match.group(1), label, 1)
      
      chunks[-1].code += "\n" + line

    for chunk in chunks:
      chunk.gcc_flags = ASSEMBLY_FLAGS

    return chunks
      
  def path(self) -> Path:
    """
    Returns a unique path for this chunk.
    """
    if self.tag is None:
      return self.source

    path = self.source
    return path.with_stem(f"{path.stem}_{self.tag}")

  def compile(self) -> elf.ELF:
    """
    Compiles this code chunk, producing an ELF object file.
    """

    if self._object is not None:
      return self._object

    # Spill the code3 into a file. This enables GCC to give us diagnostics
    # that actually have a file name in them.
    src = TEMP / self.path().name
    src.write_text(self.code)

    self._object_path = TEMP / (self.path().stem + ".o")
    run("powerpc-eabi-gcc", self.gcc_flags + ["-c", src])
    
    self._object = elf.ELF(self._object_path.read_bytes())

    for section in self._object.sections:
      # We do not currently support .data, .bss, or extra .text sections.

      if section.name.startswith('.data'):
        assert section.data == b"", "declaring globals is NYI"

      assert section.name == '.text' or not section.name.startswith('.text'), \
        "found extra .text section, probably due to template or virtual functions"

    return self._object

  def link(self, ld_script: Path) -> Tuple[bytes, List[elf.Relocation]]:
    """
    Links a code chunk, returning its relocated text section, and any
    relocations we cannot resolve statically.
    """

    binary = self._object_path.with_suffix(".elf")
    run("powerpc-eabi-ld", [
      "-Ttext", str(hex(self.origin)), # Specify where we want this chunk to begin.
                                       # All data for this chunk is in .text already.
      "--just-symbols", ld_script,     # Include the global symbols we dug up.
      "--relocatable",                 # Ask ld to generate relocations for us.
      "-o", binary,
      self._object_path,
    ])

    # Extract the text section and apply relocations to it.
    return Linker(self.origin, elf.ELF.from_file(binary)).apply_relocations()

def allocate_free_space(chunks: List[Chunk]):
  """
  Allocates free space for all chunks requesting the aforementioned space.

  It does so by assembling each chunk and measuring the size of the text
  section.
  """

  next_offset = FREE_SPACE_START
  for chunk in chunks:
    if chunk.origin is not None:
      continue

    chunk.origin = next_offset
    try:
      next_offset += len(chunk.compile().sections_by_name[".text"].data)
    except Exception as e:
      raise Exception(f'{chunk.path()}: {e}') from e

def find_globals(chunks: List[Chunk]) -> Dict[str, int]:
  """
  Calculates the desired for all global symbols, which can then
  be converted into linker directives.
  """

  table = {}
  for chunk in chunks:
    try:
      for _, syms in chunk.compile().symbols.items():
        for sym in syms:
          if sym.name in chunk.labels:
            table[sym.name] = chunk.labels[sym.name]
            continue

          if sym.binding != elf.Symbol.Binding.STB_GLOBAL:
            continue # Only care about .global symbols.
          if sym.section_index == elf.Symbol.SpecialSection.SHN_UNDEF:
            continue # Skip undefined symbols.
          
          table[sym.name] = chunk.origin + sym.address

    except Exception as e:
      raise Exception(f'{chunk.path()}: {e}') from e

  return table

def make_ld_script(base: Path, custom_symbols: Dict[str, int]) -> Path:
  """
  Constructs a custom linker script and returns the path to it.
  """

  output = TEMP / "linker.ld"
  script = base.read_text() + "\n\n" + \
            "\n".join([f"{sym} = {addr:#x};" for sym, addr in custom_symbols.items()]) + "\n"
  output.write_text(script)
  return output

class Linker:
  """
  State for linking (i.e. resolving relocations and custom asm features).
  """

  origin: int
  binary: elf.ELF

  text: elf.Cursor
  rela: elf.Section
  symtab: List[elf.Symbol]

  def __init__(self, origin: int, binary: elf.ELF):
    self.origin = origin
    self.binary = binary
    self.text = elf.Cursor(self.binary.sections_by_name[".text"].data)
    self.symtab = self.binary.symbols[".symtab"]

  def apply_relocations(self) -> Tuple[bytes, List[elf.Relocation]]:
    if ".rela.text" not in self.binary.relocations:
      return self.text.data(), []
    
    relos = []
    for relo in self.binary.relocations[".rela.text"]:
        symbol = self.symtab[relo.symbol_index]

        if self._local(relo, symbol):
          continue # No need to record this relocation.
        
        self._nonlocal(relo, symbol)
        relos.append(relo)


    return self.text.data(), relos

  def _local(self, relo: elf.Relocation, symbol: elf.Symbol) -> bool:
    # Relocate relative branches within the RPX because doing it at runtime
    # is unnecessary (and it won't have the symbols)
    src = self.origin + relo.target_offset
    assert symbol.section_index != elf.Symbol.SpecialSection.SHN_UNDEF, \
      f"tried to apply relocation against symbol {symbol.name} with undefined address"

    dst = symbol.address
    if symbol.section_index == elf.Symbol.SpecialSection.SHN_ABS:
      # Absolute address.
      dst += relo.addend
    else:
      # Symbol address is relative to the start of this patch.
      dst += self.origin

    relative_offset = (dst - src) & ~0b11 # Round to nearest instruction

    if relo.type == elf.Relocation.Type.R_PPC_REL24:
      assert -0x200_0000 <= relative_offset < 0x200_0000, \
        f"relocation failed: Cannot branch from {src:#x} to {dst:#x} with a 24-bit relative offset"
      
      inst = self.text.read_u32(offset=relo.target_offset)
      inst &= ~0x03FFFFFC
      inst |= relative_offset & 0x03FFFFFC
      self.text.write_u32(inst, offset=relo.target_offset)

      return True

    elif relo.type == elf.Relocation.Type.R_PPC_REL14:
      assert -0x8000 <= relative_offset < 0x8000, \
        f"relocation failed: Cannot branch from {src:#x} to {dst:#x} with a 14-bit relative offset"
      
      inst = self.text.read_u32(offset=relo.target_offset)
      inst &= ~0xFFFC
      inst |= relative_offset & 0xFFFC
      self.text.write_u32(inst, offset=relo.target_offset)

      return True

    return False

  def _nonlocal(self, relo: elf.Relocation, symbol: elf.Symbol):
    if symbol.section_index == elf.Symbol.SpecialSection.SHN_ABS:
      # Absolute address.
      addr = symbol.address
      if symbol.address >= DATA_START:
        base_section_addr = DATA_START
        base_section_index = 2
      elif symbol.address >= TEXT_START:
        base_section_addr = TEXT_START
        base_section_index = 1

    elif symbol.section_index == 1: # Address is relative to the start of this patch.
      base_section_index = 1
      base_section_addr = TEXT_START
      addr = self.origin + symbol.address
    
    else:
      raise Exception(f"invalid nonlocal relocation: unexpected section index {symbol.section_index:#x}")

    offset = relo.target_offset
    relo.target_offset += self.origin # Calculate offset of this instruction relative to the start of the section
    relo.symbol_index = base_section_index
    relo.addend = addr - base_section_addr

    # This shouldn't be necessary but it makes things easier to validate
    if relo.type == elf.Relocation.Type.R_PPC_ADDR32:
      self.text.write_u32(addr, offset=offset)

    elif relo.type == elf.Relocation.Type.R_PPC_ADDR16_LO:
      self.text.write_u16(addr, offset=offset)

    elif relo.type == elf.Relocation.Type.R_PPC_ADDR16_HI:
      self.text.write_u16((addr >> 16), offset=offset)

    elif relo.type == elf.Relocation.Type.R_PPC_ADDR16_HA:
      if addr & 0x8000: # Adjust for lower halfword being treated as signed.
        addr >>= 16
        addr += 1
      else:
        addr >>= 16

      self.text.write_u16(addr, offset=offset)

    else:
      raise Exception(f"Unexpected non-local relocation type {symbol.type:#x}")

class Dumper(yaml.Dumper):
  pass

Dumper.add_representer(int, lambda _, data:
  yaml.ScalarNode('tag:yaml.org,2002:int', f"0x{data:02X}")
)
Dumper.add_representer(elf.Relocation, lambda dumper, data: dumper.represent_mapping(
  'tag:yaml.org,2002:map',
  {
    "r_offset": data.target_offset,
    "r_info": (data.symbol_index << 8) | (data.type & 0xFF),
    "r_addend": data.addend,
  },
  flow_style=True,
))
Dumper.add_representer(
  bytes,
  lambda dumper, data: dumper.represent_sequence(u"tag:yaml.org,2002:seq", list(data), flow_style=True)
)

def main():
  # First delete any old patch diffs.
  root = Path(__file__).parent
  def path_glob( pattern: str) -> Iterable[Path]:
    return map(Path, glob.glob(glob.escape(root) + '/' + pattern))
  for diff_path in path_glob('patch_diffs/*_diff.yaml'):
    diff_path.unlink()
  
  patches = [
    path for path in path_glob('patches/*')
    if path.suffix not in ['.h', '.hpp']
  ]
  patches.sort(key=lambda p: str(p).lower())
  
  chunks: List[Chunk] = []
  for patch in patches:
    try:
      chunks += Chunk.parse(patch)
    except Exception as e:
      raise Exception(f'{patch}: {e}') from e
  
  allocate_free_space(chunks)

  symbols = find_globals(chunks)
  (root / 'custom_symbols.yaml').write_text(
    yaml.dump(
      {sym: addr for sym, addr in symbols.items() if not sym.startswith(".L_")},
      Dumper=Dumper, default_flow_style=False) + '\n'
  )
  
  ld_script = make_ld_script(root / 'linker.ld', symbols)

  # Aggregate chunks by source.
  chunks_by_source: Dict[Path, List[Chunk]] = {}
  for chunk in chunks:
    if chunk.source not in chunks_by_source:
      chunks_by_source[chunk.source] = []
    chunks_by_source[chunk.source].append(chunk)

  for source, chunks in chunks_by_source.items():
    diff = {
      "Data": {},
      "Relocations": []
    }

    for chunk in chunks:
      try:
        text, relos = chunk.link(ld_script)
      except Exception as e:
        raise Exception(f'{chunk.path()}: {e}') from e
      diff["Data"][chunk.origin] = text
      diff["Relocations"] += relos

    if diff["Relocations"] == []:
      del diff["Relocations"]

    diff_yaml = root / 'patch_diffs' / (source.stem + "_diff.yaml")
    diff_yaml.write_text(yaml.dump(diff, Dumper=Dumper, default_flow_style=False))
    print(f"wrote {diff_yaml}")

if __name__ == '__main__':
  try:
    main()

  except Exception as e:
    stack_trace = traceback.format_exc()
    error_message = str(e) + "\n\n" + stack_trace
    print(error_message)
    sys.exit(1)
    
  finally:
    shutil.rmtree(TEMP)
  