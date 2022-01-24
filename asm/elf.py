# Mostly copied from the SD rando code, reads sections to generate relocations

from enum import Enum
import struct
from io import BytesIO
from collections import OrderedDict



def read_bytes(data, offset, length):
  data.seek(offset)
  return data.read(length) 

def read_str_until_null_character(data, offset):
  data_length = data.seek(0, 2)
  if offset > data_length:
    raise InvalidOffsetError("Offset 0x%X is past the end of the data (length 0x%X)." % (offset, data_length))
  
  temp_offset = offset
  str_length = 0
  while temp_offset < data_length:
    data.seek(temp_offset)
    char = data.read(1)
    if char == b"\0":
      break
    else:
      str_length += 1
    temp_offset += 1
  
  data.seek(offset)
  str = data.read(str_length).decode("shift_jis")
  
  return str

def read_u8(data, offset):
  data.seek(offset)
  return struct.unpack(">B", data.read(1))[0]

def read_u16(data, offset):
  data.seek(offset)
  return struct.unpack(">H", data.read(2))[0]

def read_u32(data, offset):
  data.seek(offset)
  return struct.unpack(">I", data.read(4))[0]

def read_float(data, offset):
  data.seek(offset)
  return struct.unpack(">f", data.read(4))[0]

def read_s8(data, offset):
  data.seek(offset)
  return struct.unpack(">b", data.read(1))[0]

def read_s16(data, offset):
  data.seek(offset)
  return struct.unpack(">h", data.read(2))[0]

def read_s32(data, offset):
  data.seek(offset)
  return struct.unpack(">i", data.read(4))[0]



class ELF:
  def read_from_file(self, file_path):
    with open(file_path, "rb") as f:
      self.data = BytesIO(f.read())
    
    self.section_headers_table_offset = read_u32(self.data, 0x20)
    self.num_section_headers = read_u16(self.data, 0x30)
    self.section_header_string_table_section_index = read_u16(self.data, 0x32)
    
    self.sections = []
    for i in range(self.num_section_headers):
      section = ELFSection()
      section.read(self.data, self.section_headers_table_offset + i*ELFSection.ENTRY_SIZE)
      self.sections.append(section)
    
    section_header_string_table_offset = self.sections[self.section_header_string_table_section_index].section_offset
    for section in self.sections:
      section.name = read_str_until_null_character(self.data, section_header_string_table_offset+section.name_offset)
    
    self.sections_by_name = OrderedDict()
    for section in self.sections:
      self.sections_by_name[section.name] = section
    
    self.relocations = OrderedDict()
    for section in self.sections:
      if section.type == ELFSectionType.SHT_RELA:
        self.relocations[section.name] = []
        for i in range(section.size//ELFRelocation.ENTRY_SIZE):
          relocation = ELFRelocation()
          relocation.read(self.data, section.section_offset + i*ELFRelocation.ENTRY_SIZE)
          self.relocations[section.name].append(relocation)
    
    self.symbols = OrderedDict()
    for section in self.sections:
      if section.type == ELFSectionType.SHT_SYMTAB:
        self.symbols[section.name] = []
        for i in range(section.size//ELFSymbol.ENTRY_SIZE):
          symbol = ELFSymbol()
          symbol.read(self.data, section.section_offset + i*ELFSymbol.ENTRY_SIZE)
          symbol.name = self.read_string_from_table(symbol.name_offset)
          self.symbols[section.name].append(symbol)
  
  def read_string_from_table(self, string_offset):
    offset = self.sections_by_name[".strtab"].section_offset + string_offset
    return read_str_until_null_character(self.data, offset)

class ELFSection:
  ENTRY_SIZE = 0x28
  
  def read(self, elf_data, header_offset):
    self.header_offset = header_offset
    
    self.name_offset = read_u32(elf_data, self.header_offset+0x00)
    self.type = ELFSectionType(read_u32(elf_data, self.header_offset+0x04))
    self.flags = read_u32(elf_data, self.header_offset+0x08)
    self.address = read_u32(elf_data, self.header_offset+0x0C)
    self.section_offset = read_u32(elf_data, self.header_offset+0x10)
    self.size = read_u32(elf_data, self.header_offset+0x14)
    self.link = read_u32(elf_data, self.header_offset+0x18)
    self.info = read_u32(elf_data, self.header_offset+0x1C)
    self.addr_align = read_u32(elf_data, self.header_offset+0x20)
    self.entry_size = read_u32(elf_data, self.header_offset+0x24)
    
    self.data = BytesIO(read_bytes(elf_data, self.section_offset, self.size))

class ELFRelocation:
  ENTRY_SIZE = 0xC
  
  def read(self, elf_data, offset):
    self.offset = offset
    
    self.relocation_offset = read_u32(elf_data, self.offset + 0x00)
    info = read_u32(elf_data, self.offset + 0x04)
    self.type = info & 0x000000FF
    self.symbol_index = (info & 0xFFFFFF00) >> 8
    self.addend = read_u32(elf_data, self.offset + 0x08)

class ELFSymbol:
  ENTRY_SIZE = 0x10
  
  def read(self, elf_data, offset):
    self.offset = offset
    
    self.name_offset = read_u32(elf_data, self.offset + 0x00)
    self.address = read_u32(elf_data, self.offset + 0x04)
    self.size = read_u32(elf_data, self.offset + 0x08)
    self.info = read_u8(elf_data, self.offset + 0x0C) # lower nibble is type, upper is binding
    self.other = read_u8(elf_data, self.offset + 0x0D)
    self.section_index = read_u16(elf_data, self.offset + 0x0E)

class ELFSectionType(Enum):
  SHT_NULL = 0x0
  SHT_PROGBITS = 0x1
  SHT_SYMTAB = 0x2
  SHT_STRTAB = 0x3
  SHT_RELA = 0x4
  SHT_HASH = 0x5
  SHT_DYNAMIC = 0x6
  SHT_NOTE = 0x7
  SHT_NOBITS = 0x8
  SHT_REL = 0x9
  SHT_SHLIB = 0x0A
  SHT_DYNSYM = 0x0B
  SHT_INIT_ARRAY = 0x0E
  SHT_FINI_ARRAY = 0x0F
  SHT_PREINIT_ARRAY = 0x10
  SHT_GROUP = 0x11
  SHT_SYMTAB_SHNDX = 0x12
  SHT_NUM = 0x13
  
  UNK_1 = 0x6FFFFFF5
