import glob
import re
import os
import shutil
from collections import OrderedDict
import struct
import json
import traceback

import subprocess
import tempfile

from elf import *

temp_dir = tempfile.mkdtemp()

custom_symbols = OrderedDict()
to_relocate = OrderedDict()
  
free_space_start_offset = 0x028f87f4
next_free_space_offset = free_space_start_offset
  
devkitbasepath = os.environ.get("DEVKITPPC") + "/bin"

# only handles relocations for things that are in .text, pointing to .data
def get_relocations_for_elf(o_path, start_addr, symbol_addresses):
    elf = ELF()
    elf.read_from_file(o_path)
    relocations = []


    symtab_symbols = elf.symbols.get(".symtab", [])    
    for relocation in elf.relocations.get(".rela.text", []):
        symbol = symtab_symbols[relocation.symbol_index]
        if symbol.name in to_relocate.keys():
            if to_relocate[symbol.name] == ".data":
                reloc = OrderedDict() #formatted like this for json dumping later
                reloc["r_offset"] = "0x%08X" % (start_addr + relocation.relocation_offset)
                reloc["r_info"] = "0x%08X" % (0x00000200 | relocation.type)
                addend = (int(symbol_addresses[symbol.name], base=16)) - 0x10000000
                reloc["r_addend"] = "0x%08X" % addend
                relocations.append(reloc)
            if to_relocate[symbol.name] == ".text":
                reloc = OrderedDict() #formatted like this for json dumping later
                reloc["r_offset"] = "0x%08X" % (start_addr + relocation.relocation_offset)
                reloc["r_info"] = "0x%08X" % (0x00000100 | relocation.type)
                addend = (int(symbol_addresses[symbol.name], base=16)) - 0x02000000
                reloc["r_addend"] = "0x%08X" % addend
                relocations.append(reloc)

    return relocations

def main():
  global temp_dir

  global custom_symbols
  global to_relocate

  global free_space_start_offset
  global next_free_space_offset

  global devkitbasepath
  
  try:
    
    for diff_path in glob.glob('./patch_diffs/*_diff.json'):
      os.remove(diff_path)
      
    for reloc_path in glob.glob('./patch_diffs/*_reloc.json'):
      os.remove(reloc_path)
    
    with open("linker.ld") as f:
        linker_script = f.read()
  
    with open("symbols_to_relocate.json") as f:
      to_relocate = json.loads(f.read(), object_pairs_hook=OrderedDict)
    
    all_asm_file_paths = glob.glob('./patches/*.asm')
    all_asm_files = [os.path.basename(asm_path) for asm_path in all_asm_file_paths]
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
        patch_path = os.path.join(".", "patches", patch_filename)
        with open(patch_path) as f:
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
            most_recent_org_offset = None
            continue
          elif line.find(".global") != -1 and (not isinstance(most_recent_org_offset, str) or most_recent_org_offset.find("@FreeSpace") == -1):
            raise Exception("Declared new symbol %s inside original code space at %s" % (line, most_recent_org_offset))
          elif not line:
            # Blank line
            continue
          
          if most_recent_org_offset is None:
            if line[0] == ";":
              # Comment
              continue
            raise Exception("Found code before any .org directive")
          
          code_chunks[patch_name][most_recent_org_offset] += line + "\n"
        
        if not code_chunks[patch_name]:
          raise Exception("No code found")
  
        if most_recent_org_offset is not None:
          raise Exception("File was not closed before the end of the file")
    
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
            devkitbasepath + "/powerpc-eabi-as",
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
            devkitbasepath + "/powerpc-eabi-ld",
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
              if line.startswith(" .text          "):
                on_custom_symbols = True
                continue
              
              if on_custom_symbols:
                if not line:
                  break
                match = re.search(r" +0x(?:00000000)?([0-9a-f]{8}) +(\S+)", line)
                symbol_address = int(match.group(1), 16)
                symbol_name = match.group(2)
                custom_symbols[symbol_name] = "0x%08X" % symbol_address
                temp_linker_script += "%s = 0x%08X;\n" % (symbol_name, symbol_address)
                linker_dict[symbol_name] = "0x%08X" % symbol_address
          
          # Keep track of changed bytes.
          if org_offset in diffs:
            raise Exception("Duplicate .org directive within a single asm patch: %X" % org_offset)
          
          with open(bin_name, "rb") as f:
            binary_data = f.read()
          
          code_chunk_size_in_bytes = len(binary_data)
          
          if using_free_space:
            next_free_space_offset += code_chunk_size_in_bytes
          
          bytes = list(struct.unpack("B"*code_chunk_size_in_bytes, binary_data))
          bytes_hex = list()
          for byte in bytes:
              bytes_hex.append("0x%02X" % byte)
          diffs[org_offset] = bytes_hex
  
          # generate relocations for the patch
          o_name = os.path.join(temp_dir, "tmp_" + patch_name + "_%08X.o" % org_offset)
          relocations_for_patch += get_relocations_for_elf(o_name, org_offset, linker_dict)
     
        diffs_hex = OrderedDict()
        for org, bytes in diffs.items():
            diffs_hex["0x%08X" % org] = bytes
  
        diff_path = os.path.join(".", "patch_diffs", patch_name + "_diff.json")
        with open(diff_path, "w") as f:
          f.write(json.dumps(diffs_hex) + "\n")
  
        if len(relocations_for_patch) != 0:
            reloc_path = os.path.join(".", "patch_diffs", patch_name + "_reloc.json")
            with open(reloc_path, "w") as f:
             f.write(json.dumps(relocations_for_patch) + "\n")
          
  
    with open("./custom_symbols.json", "w") as f:
      f.write(json.dumps(custom_symbols, indent=2) + "\n")
      print("Dumped custom symbols")
  
    shutil.rmtree(temp_dir)
    print("Finished generating diffs")
  
  except Exception as e:
    stack_trace = traceback.format_exc()
    error_message = str(e) + "\n\n" + stack_trace
    print(error_message)
    input()

if __name__ == "__main__":
    main()
  