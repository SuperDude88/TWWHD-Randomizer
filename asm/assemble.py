from typing import List
from collections import OrderedDict
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

from elf import ELF, ELFRelocation

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

# Allow yaml to dump OrderedDicts for diffs
yaml.Dumper.add_representer(
  OrderedDict,
  lambda dumper, data: dumper.represent_dict(data.items())
)

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

custom_symbols = OrderedDict()
to_relocate = OrderedDict()
  
free_space_start_offset = 0x028f87f4
next_free_space_offset = free_space_start_offset
  
section_addresses = OrderedDict({"": 0, ".text": 0x02000000, ".data": 0x10000000})

# only handles relocations for things that are in .text, pointing to .data
def get_relocations_for_elf(o_path, start_addr, symbol_addresses):
    elf = ELF()
    elf.read_from_file(o_path)

    relocations = []

    for relocation in elf.relocations.get(".rela.text", []):
      symbol = elf.symbols[".symtab"][relocation.symbol_index]
      if symbol.name in to_relocate.keys():
        reloc = ELFRelocation()
        reloc_section_sym = to_relocate[symbol.name]
        reloc.relocation_offset = start_addr + relocation.relocation_offset
        reloc.symbol_index = list(section_addresses.keys()).index(reloc_section_sym)
        reloc.type = relocation.type
        reloc.addend = (int(symbol_addresses[symbol.name], base=16)) - section_addresses[reloc_section_sym]
        relocations.append(reloc)

    return relocations
  
try:
  # First delete any old patch diffs.
  for diff_path in glob.glob(glob.escape(asm_dir) + '/patch_diffs/*_diff.yaml'):
    os.remove(diff_path)
    
  for reloc_path in glob.glob(glob.escape(asm_dir) + '/patch_diffs/*_reloc.yaml'):
    os.remove(reloc_path)
  
  with open(asm_dir + "/linker.ld") as f:
    linker_script = f.read()

  with open(asm_dir + "/symbols_to_relocate.yaml") as f:
    to_relocate = yaml.safe_load(f)
  
  all_asm_file_paths = glob.glob(glob.escape(asm_dir) + "/patches/*.asm")
  all_asm_files = [os.path.basename(asm_path).lower() for asm_path in all_asm_file_paths] #lowercase names to make sort more consistent
  all_asm_files.sort()
  
  # Don't need custom_data
  #all_asm_files.remove("custom_data.asm")
  #all_asm_files = ["custom_data.asm", "custom_funcs.asm"] + all_asm_files
  all_asm_files.remove("custom_funcs.asm")
  all_asm_files = ["custom_funcs.asm"] + all_asm_files
  
  code_chunks = OrderedDict()
  temp_linker_script = ""
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
      code_chunks[patch_name] = OrderedDict()
      
      most_recent_org_offset = None
      for line in asm.splitlines():
        line = re.sub(r";.+$", "", line)
        line = line.strip()
        
        #don't need to use .open, patches are always in cking.rpx
        #still include .close for some cross-patch-file checking
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
        elif line == ".close":
          print("Unnecessary close directive in %s" % patch_filename)
          continue
        elif line.find(".global") != -1 and (not isinstance(most_recent_org_offset, str) or most_recent_org_offset.find("@FreeSpace") == -1):
          raise Exception("Declared new symbol %s inside original code space at %s" % (line, most_recent_org_offset))
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
  
  temp_linker_script = linker_script + "\n" + temp_linker_script

  # populate dict for relocation stuff
  linker_dict = OrderedDict()
  for line in linker_script.splitlines():
      if not line:
          continue
      if line.find("=") != -1: # basic check to find a symbol on the line
          name = line.split(" = ")[0]
          addr = line.split(" = ")[1].replace(";", "")
          linker_dict[name] = addr

  last_patch = ""
  for patch_name, code_chunks_for_file in code_chunks.items():
      if last_patch != patch_name:
        print("Generating patch " + patch_name)
        last_patch = patch_filename
      
      diffs = OrderedDict()
      relocations_for_patch = []
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
      # And add any local branches inside this file.
      
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
          "-T", temp_linker_name,
          "-Map=" + map_name,
          o_name,
          "-o", bin_name,
          "--oformat=binary"
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
              if not line:
                break
              match = re.search(r"^ +0x(?:00000000)?([0-9a-f]{8}) {16,}(\S+)", line)
              if not match:
                continue

              symbol_address = int(match.group(1), 16)
              symbol_name = match.group(2)
              custom_symbols[symbol_name] = symbol_address
              temp_linker_script += "%s = 0x%08X;\n" % (symbol_name, symbol_address)
              linker_dict[symbol_name] = "0x%08X" % symbol_address
        
        if org_offset in diffs:
          raise Exception("Duplicate .org directive within a single asm patch: %X" % org_offset)
        
        with open(bin_name, "rb") as f:
          binary_data = f.read()
        
        code_chunk_size_in_bytes = len(binary_data)
        
        if using_free_space:
          next_free_space_offset += code_chunk_size_in_bytes
        
        diffs[org_offset] = list(struct.unpack("B"*code_chunk_size_in_bytes, binary_data))

        # generate relocations for the patch
        o_name = os.path.join(temp_dir, "tmp_" + patch_name + "_%08X.o" % org_offset)
        relocations_for_patch += get_relocations_for_elf(o_name, org_offset, linker_dict)

      diff_path = os.path.join(asm_dir, "patch_diffs", patch_name + "_diff.yaml")
      with open(diff_path, "w", newline='\n') as f:
        # Don't put every element on a separate line
        yaml.Dumper.add_representer(
          list,
          lambda dumper, data: dumper.represent_sequence(u"tag:yaml.org,2002:seq", data, flow_style=True)
        )
        f.write(yaml.dump(diffs, Dumper=yaml.Dumper, default_flow_style=False))

      if len(relocations_for_patch) != 0:
          reloc_path = os.path.join(asm_dir, "patch_diffs", patch_name + "_reloc.yaml")
          with open(reloc_path, "w", newline='\n') as f:
            # Put each reloc on one line
            yaml.Dumper.add_representer(
              list,
              lambda dumper, data: dumper.represent_sequence(u"tag:yaml.org,2002:seq", data, flow_style=False)
            )
            f.write(yaml.dump(relocations_for_patch, Dumper=yaml.Dumper, default_flow_style=False))

  with open(asm_dir + "/custom_symbols.yaml", "w", newline='\n') as f:
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
  