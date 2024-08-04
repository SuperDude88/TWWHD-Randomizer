from typing import List
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

from elf import *

def read_all_bytes(data):
  data.seek(0)
  return data.read()

def write_u16(data, offset, new_value):
  new_value = struct.pack(">H", new_value)
  data.seek(offset)
  data.write(new_value)

def write_u32(data, offset, new_value):
  new_value = struct.pack(">I", new_value)
  data.seek(offset)
  data.write(new_value)

asm_dir = os.path.dirname(__file__)

if sys.platform == "win32":
  devkitbasepath = r"C:\devkitPro\devkitPPC\bin"
else:
  if not "DEVKITPPC" in os.environ:
    raise Exception(r"Could not find devkitPPC. Path to devkitPPC should be in the DEVKITPPC env var")
  devkitbasepath = os.environ.get("DEVKITPPC") + "/bin"
  
def get_bin(name):
  if not sys.platform == "win32":
    return os.path.join(devkitbasepath, name)
  return os.path.join(devkitbasepath, name + ".exe")

if not os.path.isfile(get_bin("powerpc-eabi-as")):
  raise Exception(r"Failed to assemble code: Could not find devkitPPC. devkitPPC should be installed to: C:\devkitPro\devkitPPC")

# Output integers as hexadecimal
yaml.Dumper.add_representer(
  int,
  lambda dumper, data: yaml.ScalarNode('tag:yaml.org,2002:int', "0x%02X" % data)
)

# Output ELFRelocation class
def reloc_to_dict(data):
  return {"r_offset": data.relocation_offset, "r_info": (data.symbol_index << 8) | (data.type & 0xFF), "r_addend": data.addend}

yaml.Dumper.add_representer(
  ELFRelocation,
  lambda dumper, data: dumper.represent_mapping('tag:yaml.org,2002:map', reloc_to_dict(data), flow_style=True)
)

temp_dir = tempfile.mkdtemp()

custom_symbols = {}
  
free_space_start_offset = 0x028F87F4
next_free_space_offset = free_space_start_offset

def get_code_and_relocations_from_elf(bin_name):
  elf = ELF()
  elf.read_from_file(bin_name)

  relocations_in_elf = []

  for elf_section in elf.sections:
    if elf_section.name == ".text":
      # Get the code and overwrite the ELF file with just the raw binary code
      with open(bin_name, "wb") as f:
        f.write(read_all_bytes(elf_section.data))
    elif elf_section.type == ELFSectionType.SHT_RELA:
      # Get the relocations
      assert(elf_section.name == ".rela.text")
      
      for elf_relocation in elf.relocations[elf_section.name]:
        elf_symbol = elf.symbols[".symtab"][elf_relocation.symbol_index]
        is_local_relocation = try_apply_local_relocation(bin_name, elf_relocation, elf_symbol)

        if not is_local_relocation:
          if not try_fix_nonlocal_relocation(bin_name, elf_relocation, elf_symbol):
            raise Exception("Encountered invalid non-local relocation") # TODO: make this error message better
          
          relocations_in_elf.append(elf_relocation)

  return relocations_in_elf

def try_apply_local_relocation(bin_name, elf_relocation, elf_symbol):
  # Relocate relative branches within the RPX because doing it at runtime is unnecessary (and it won't have the symbols)

  branch_src_offset = org_offset + elf_relocation.relocation_offset
  if elf_symbol.section_index == 0xFFF1: # symbol address is absolute (0xFFF1 -> SHN_ABS)
    branch_dest_offset = elf_symbol.address + elf_relocation.addend
  elif elf_symbol.section_index == 0: # did not have a defined address
    raise Exception("Tried to apply relocation against symbol with undefined address")
  else: # symbol address is relative to the start of this patch
    branch_dest_offset = org_offset + elf_symbol.address

  relative_branch_offset = ((branch_dest_offset - branch_src_offset) // 4) << 2 # round down to nearest instruction

  if elf_relocation.type == ELFRelocationType.R_PPC_REL24:
    if relative_branch_offset > 0x1FFFFFF or relative_branch_offset < -0x2000000:
      raise Exception("Relocation failed: Cannot branch from %X to %X with a 24-bit relative offset." % (branch_src_offset, branch_dest_offset))
    
    with open(bin_name, "r+b") as f:
      instruction = read_u32(f, elf_relocation.relocation_offset)
      instruction &= ~0x03FFFFFC
      instruction |= relative_branch_offset & 0x03FFFFFC
      write_u32(f, elf_relocation.relocation_offset, instruction)

    return True

  elif elf_relocation.type == ELFRelocationType.R_PPC_REL14:
    if relative_branch_offset > 0x7FFF or relative_branch_offset < -0x8000:
      raise Exception("Relocation failed: Cannot branch from %X to %X with a 14-bit relative offset." % (branch_src_offset, branch_dest_offset))
    
    with open(bin_name, "r+b") as f:
      instruction = read_u32(f, elf_relocation.relocation_offset)
      instruction &= ~0x0000FFFC
      instruction |= relative_branch_offset & 0x0000FFFC
      write_u32(f, elf_relocation.relocation_offset, instruction)

    return True

  return False

section_addresses = [0, 0x02000000, 0x10000000] # null, .text, .data

def try_fix_nonlocal_relocation(bin_name, elf_relocation, elf_symbol):
  if elf_symbol.section_index == 0xFFF1: # address is absolute (0xFFF1 -> SHN_ABS)
    if elf_symbol.address >= 0x10000000:
      base_section_index = 2
      absolute_symbol_address = elf_symbol.address
    elif elf_symbol.address >= 0x02000000:
      base_section_index = 1
      absolute_symbol_address = elf_symbol.address
  elif elf_symbol.section_index == 1: # address is relative to the start of this patch
    base_section_index = 1
    absolute_symbol_address = org_offset + elf_symbol.address
  else:
    raise Exception("Invalid nonlocal relocation: unexpected section index %X" % (elf_symbol.section_index))

  relative_relocation_offset = elf_relocation.relocation_offset
  elf_relocation.relocation_offset += org_offset # Calculate offset of this instruction relative to the start of the section
  elf_relocation.symbol_index = base_section_index
  elf_relocation.addend = absolute_symbol_address - section_addresses[base_section_index]

  # This shouldn't be necessary but it makes things easier to validate
  match elf_relocation.type:
    case ELFRelocationType.R_PPC_ADDR32:
      with open(bin_name, "r+b") as f:
        write_u32(f, relative_relocation_offset, absolute_symbol_address)
    case ELFRelocationType.R_PPC_ADDR16_LO:
      with open(bin_name, "r+b") as f:
        write_u16(f, relative_relocation_offset, absolute_symbol_address & 0x0000FFFF)
    case ELFRelocationType.R_PPC_ADDR16_HI:
      with open(bin_name, "r+b") as f:
        write_u16(f, relative_relocation_offset, (absolute_symbol_address >> 16) & 0x0000FFFF)
    case ELFRelocationType.R_PPC_ADDR16_HA:
      with open(bin_name, "r+b") as f:
        if absolute_symbol_address & 0x8000: # adjust for lower halfword being treated as signed
          adjusted_address = ((absolute_symbol_address >> 16) + 1) & 0x0000FFFF
        else:
          adjusted_address = (absolute_symbol_address >> 16) & 0x0000FFFF
        write_u16(f, relative_relocation_offset, adjusted_address)
    case _:
      raise Exception("Unexpected non-local relocation type %X" % (elf_relocation.type))

  return True

try:
  # First delete any old patch diffs.
  for diff_path in glob.glob(glob.escape(asm_dir) + '/patch_diffs/*_diff.yaml'):
    os.remove(diff_path)
  
  with open(asm_dir + "/linker.ld") as f:
    linker_script = f.read()
  
  all_asm_file_paths = glob.glob(glob.escape(asm_dir) + "/patches/*.asm")
  all_asm_files = [os.path.basename(asm_path).lower() for asm_path in all_asm_file_paths] # lowercase names to make sort more consistent
  all_asm_files.sort()

  all_asm_files.remove("custom_funcs.asm")
  all_asm_files = ["custom_funcs.asm"] + all_asm_files
  
  code_chunks = {}
  temp_linker_script = linker_script + "\n"
  next_free_space_id = 1
  
  for patch_filename in all_asm_files:
    patch_path = os.path.join(asm_dir, "patches", patch_filename)
    preprocess_path = os.path.join(temp_dir, "preprocess_" + patch_filename)

    command = [
      get_bin("powerpc-eabi-gcc"),
      "-E",
      "-xassembler-with-cpp",
      patch_path,
      "-o", preprocess_path
    ]
    result = subprocess.call(command)
    if result != 0:
      raise Exception("Preprocessor call failed")
    
    with open(preprocess_path) as f:
      asm = f.read()
  
    patch_name = os.path.splitext(patch_filename)[0]
    code_chunks[patch_name] = {}
    
    most_recent_org_offset = None
    for line in asm.splitlines():
      line = re.sub(r";.+$", "", line)
      line = line.strip()

      org_match = re.match(r"\.org\s+0x([0-9a-f]+)$", line, re.IGNORECASE)
      org_symbol_match = re.match(r"\.org\s+([\._a-z][\._a-z0-9]+|@NextFreeSpace)$", line, re.IGNORECASE)
      branch_match = re.match(r"(?:b|beq|bne|blt|bgt|ble|bge)\s+0x([0-9a-f]+)(?:$|\s)", line, re.IGNORECASE)
      if org_match:
        org_offset = int(org_match.group(1), 16)
        if org_offset >= 0x10000000: #0x10000000 or above is in .data instead of .text, do not raise exception
            pass
        elif org_offset >= free_space_start_offset:
          raise Exception("Tried to manually set the origin point to after the start of free space.\n.org offset: 0x%X\nUse \".org @NextFreeSpace\" instead to get an automatically assigned free space offset." % org_offset)
        
        code_chunks[patch_name][org_offset] = ""
        most_recent_org_offset = org_offset
        continue
      elif org_symbol_match:
        org_symbol = org_symbol_match.group(1)

        if org_symbol == "@NextFreeSpace":
        # Need to make each instance of @NextFreeSpace into a unique label.
          org_symbol = "@FreeSpace_%d" % next_free_space_id
          next_free_space_id += 1

        code_chunks[patch_name][org_symbol] = ""
        most_recent_org_offset = org_symbol
        continue
      elif branch_match:
        # Replace branches to specific addresses with labels, and define the address of those labels in the linker script.
        branch_dest = int(branch_match.group(1), 16)
        branch_temp_label = "branch_label_%X" % branch_dest
        temp_linker_script += "%s = 0x%X;\n" % (branch_temp_label, branch_dest)
        line = re.sub(r"0x" + branch_match.group(1), branch_temp_label, line, 1)
      elif not line:
        # Blank line
        continue
      
      if most_recent_org_offset is None:
        if line[0] == ";" or line[0] == "#":
          # Comment
          continue
        raise Exception("Found code before any .org directive")
      
      code_chunks[patch_name][most_recent_org_offset] += line + "\n"
    
    if not code_chunks[patch_name]:
      raise Exception("No code found")

    most_recent_org_offset = None

  last_patch = ""
  for patch_name, code_chunks_for_file in code_chunks.items():
    if last_patch != patch_name:
      print("Generating patch " + patch_name)
      last_patch = patch_filename
    
    diffs = {}
    diffs["Data"] = {}
    # Sort code chunks in this patch so that free space chunks come first.
    # This is necessary so non-free-space chunks can branch to the free space chunks.
    def free_space_org_list_sorter(code_chunk_tuple):
      org_offset_or_symbol, temp_asm = code_chunk_tuple
      if isinstance(org_offset_or_symbol, int):
        return 0
      else:
        org_symbol = org_offset_or_symbol
        free_space_match = re.search(r"@FreeSpace_\d+", org_symbol)
        if free_space_match:
          return -1
        else:
          return 0
  
    code_chunks_for_file_sorted = list(code_chunks_for_file.items())
    code_chunks_for_file_sorted.sort(key=free_space_org_list_sorter)

    # Add custom symbols in the current file to the temporary linker script.
    for symbol_name, symbol_address in custom_symbols.items():
      temp_linker_script += "%s = %s;\n" % (symbol_name, symbol_address)
    
    for org_offset_or_symbol, temp_asm in code_chunks_for_file_sorted:
      using_free_space = False
      if isinstance(org_offset_or_symbol, int):
        org_offset = org_offset_or_symbol
      else:
        org_symbol = org_offset_or_symbol
        free_space_match = re.search(r"@FreeSpace_\d+", org_symbol)
        if free_space_match:
          org_offset = next_free_space_offset
          using_free_space = True
        else:
          if org_symbol not in custom_symbols:
            raise Exception(".org specified an invalid custom symbol: %s" % org_symbol)
          org_offset = custom_symbols[org_symbol]

      temp_linker_name = os.path.join(temp_dir, "tmp_linker.ld")
      with open(temp_linker_name, "w") as f:
        f.write(temp_linker_script)
      
      temp_asm_name = os.path.join(temp_dir, "tmp_" + patch_name + "_%08X.asm" % org_offset)
      with open(temp_asm_name, "w") as f:
        f.write("\n")
        f.write(temp_asm)
      
      o_name = os.path.join(temp_dir, "tmp_" + patch_name + "_%08X.o" % org_offset)
      command = [
        get_bin("powerpc-eabi-as"),
        "-mregnames",
        temp_asm_name,
        "-o", o_name
      ]
      result = subprocess.call(command)
      if result != 0:
        raise Exception("Assembler call failed")
  
      map_name = os.path.join(temp_dir, "tmp_" + patch_name + ".map")
      bin_name = os.path.join(temp_dir, "tmp_" + patch_name + ".bin")

      command = [
        get_bin("powerpc-eabi-ld"),
        "-Ttext", str(hex(org_offset)),
        "--just-symbols=", temp_linker_name,
        "-Map=" + map_name,
        o_name,
        "-o", bin_name,
        "--relocatable"
      ]
      result = subprocess.call(command)
      if result != 0:
        raise Exception("Linker call failed")

      with open(map_name) as f:
        on_custom_symbols = False
        for line in f.read().splitlines():
          if re.search(r"^ .\S+ +0x", line):
            on_custom_symbols = True
            continue
          
          if on_custom_symbols:
            match = re.search(r"^ +0x(?:00000000)?([0-9a-f]{8}) {16,}(\S+)", line)
            if not match:
              continue

            symbol_address = int(match.group(1), 16)
            symbol_name = match.group(2)
            custom_symbols[symbol_name] = symbol_address
            temp_linker_script += "%s = 0x%08X;\n" % (symbol_name, symbol_address)

      relocations = get_code_and_relocations_from_elf(bin_name)
      
      if org_offset in diffs["Data"]:
        raise Exception("Duplicate .org directive within a single asm patch: %X" % org_offset)
      
      with open(bin_name, "rb") as f:
        binary_data = f.read()
      
      code_chunk_size_in_bytes = len(binary_data)
      
      if using_free_space:
        next_free_space_offset += code_chunk_size_in_bytes
      
      diffs["Data"][org_offset] = list(struct.unpack("B"*code_chunk_size_in_bytes, binary_data))
      if relocations:
        if "Relocations" not in diffs:
          diffs["Relocations"] = []

        diffs["Relocations"] += relocations

    diff_path = os.path.join(asm_dir, "patch_diffs", patch_name + "_diff.yaml")
    with open(diff_path, "w") as f:
      # Don't put every element on a separate line
      yaml.Dumper.add_representer(
        list,
        lambda dumper, data: dumper.represent_sequence(u"tag:yaml.org,2002:seq", data, flow_style=True)
      )

      f.write(yaml.dump(diffs, Dumper=yaml.Dumper, default_flow_style=False))

  with open(asm_dir + "/custom_symbols.yaml", "w") as f:
    f.write(yaml.dump(custom_symbols, Dumper=yaml.Dumper, default_flow_style=False) + '\n')
    print("Dumped custom symbols")

  print("Finished generating diffs")

except Exception as e:
  stack_trace = traceback.format_exc()
  error_message = str(e) + "\n\n" + stack_trace
  print(error_message)
  input()
  raise
  
finally:
  shutil.rmtree(temp_dir)
  