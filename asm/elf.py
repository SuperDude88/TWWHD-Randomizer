"""
Bare-bones ELF manipulation library.
"""

import struct

from enum import IntEnum
from io import BytesIO
from typing import List, Dict

class Cursor:
  """
  Cursor is a helper for reading structured data out of a byte blob.

  It provides functions for seeking to an offset and reading individual ints
  and floats.
  """

  _data: BytesIO

  def __init__(self, bytes: bytes):
    self._data = BytesIO(bytes)

  def len(self) -> int:
    return len(self._data.getbuffer())

  def data(self) -> bytes:
    return self._data.getvalue()

  def tell(self) -> int:
    """
    Returns the current offset the cursor is at.
    """
    return self._data.tell()

  def seek(self, offset: int):
    """
    Seeks the cursor to the given offset.
    """
    self._data.seek(offset)
    
  def read(self, length: int) -> bytes:
    """
    Reads a blob of bytes at the given offset and length.
    """
    return self._data.read(length)

  def read_u8(self, *, offset: int = None) -> int:
    if offset is not None:
      self.seek(offset)
    return self.read(1)[0]
  
  def read_u16(self, *, offset: int = None) -> int:
    if offset is not None:
      self.seek(offset)
    return struct.unpack(">H", self.read(2))[0]

  def write_u16(self, value: int, *, offset: int = None):
    if offset is not None:
      self.seek(offset)
    self._data.write(struct.pack(">H", value & 0xFFFF))

  def read_u32(self, *, offset: int = None) -> int:
    if offset is not None:
      self.seek(offset)
    return struct.unpack(">I", self.read(4))[0]
  
  def write_u32(self, value: int, *, offset: int = None):
    if offset is not None:
      self.seek(offset)
    self._data.write(struct.pack(">I", value))
  
  def read_jis_str(self, *, offset: int = None) -> str:
    """
    Decodes a JIS-encoded, NUL-terminated string at the given offset.
    """
    len = self.len()
    if offset > len:
      raise Exception(f"offset {offset:#x} is past the end of the data (length {len:#x})")
    
    if offset is None:
      offset = self._data.tell()
    else:
      self.seek(offset)

    end = offset
    while end < len:
      if self.read_u8() == 0:
        break
      end += 1
    
    self.seek(offset)
    return self.read(end - offset).decode("shift_jis")

class ELF:
  """
  A parsed ELF file.
  """

  data: bytes

  sections: List["Section"]
  sections_by_name: Dict[str, "Section"]
  relocations: Dict[str, List["Relocation"]]
  symbols: Dict[str, List["Symbol"]]
  
  @staticmethod
  def from_file(path: str) -> "ELF":
    with open(path, "rb") as f:
      data = f.read()
    return ELF(data)

  def __init__(self, data: bytes = None):
    """
    Parses an ELF file from the given data.
    """
    if data is None:
      return
    self.load(data)

  def load(self, input: bytes):
    """
    Overwrites the contents of this file with the result of parsing
    the given data.
    """
    self.data = input
    self.sections = []
    self.sections_by_name = {}
    self.relocations = {}
    self.symbols = {}

    data = Cursor(input)
    
    offset = data.read_u32(offset=0x20)
    num_sections = data.read_u16(offset=0x30)
    for _ in range(num_sections):
      data.seek(offset)
      self.sections.append(Section(data))
      offset += Section.ENTRY_SIZE
    
    shstrtab_idx = data.read_u16(offset=0x32)
    shstrtab = Cursor(self.sections[shstrtab_idx].data)
    for section in self.sections:
      section.name = shstrtab.read_jis_str(offset=section.name_offset)
      self.sections_by_name[section.name] = section

    strtab = Cursor(self.sections_by_name[".strtab"].data)
    for section in self.sections:
      if section.type == Section.Type.SHT_RELA:
        cursor = Cursor(section.data)
        relos = len(section.data)//Relocation.ENTRY_SIZE

        self.relocations[section.name] = [Relocation(cursor) for _ in range(relos)]
        continue

      if section.type == Section.Type.SHT_SYMTAB:
        cursor = Cursor(section.data)
        syms = len(section.data)//Symbol.ENTRY_SIZE
        
        symbols = [Symbol(cursor) for _ in range(syms)]
        for symbol in symbols:
          symbol.name = strtab.read_jis_str(offset=symbol.name_offset)
        self.symbols[section.name] = symbols
        continue


class Section:
  """
  A parsed ELF section.
  """

  ENTRY_SIZE: int = 0x28
  
  class Type(IntEnum):
    """
    A type of ELF section.
    """

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
  
  class Flag(IntEnum):
    """
    A flag or mask for the flags field.
    """

    SHF_WRITE            = 0x00000001
    SHF_ALLOC            = 0x00000002
    SHF_EXECINSTR        = 0x00000004
    SHF_MERGE            = 0x00000010
    SHF_STRINGS          = 0x00000020
    SHF_INFO_LINK        = 0x00000040
    SHF_LINK_ORDER       = 0x00000080
    SHF_OS_NONCONFORMING = 0x00000100
    SHF_GROUP            = 0x00000200
    SHF_TLS              = 0x00000400
    SHF_MASKOS           = 0x0FF00000
    SHF_MASKPROC         = 0xF0000000
    SHF_ORDERED          = 0x04000000
    SHF_EXCLUDE          = 0x08000000

  offset: int
  name: str
  name_offset: int
  type: Type
  flags: int
  address: int
  section_offset: int
  link: int
  info: int
  addr_align: int
  entry_size: int

  data: bytes

  def __init__(self, data: Cursor):
    """
    Parses a section header from the given cursor position. Leaves the cursor
    at an unspecified position.
    """
    self.offset = data.tell()
    
    self.name = None
    self.name_offset = data.read_u32()
    self.type = self.Type(data.read_u32())
    self.flags = data.read_u32()
    self.address = data.read_u32()
    self.section_offset = data.read_u32()
    size = data.read_u32()
    self.link = data.read_u32()
    self.info = data.read_u32()
    self.addr_align = data.read_u32()
    self.entry_size = data.read_u32()
    
    data.seek(self.section_offset)
    self.data = data.read(size)


class Relocation:
  """
  A parsed ELF relocation.
  """

  ENTRY_SIZE = 0xC
  
  class Type(IntEnum):
    """
    A type of ELF-PPC relocation.
    """

    R_PPC_NONE = 0x00
    R_PPC_ADDR32 = 0x01
    R_PPC_ADDR24 = 0x02
    R_PPC_ADDR16 = 0x03
    R_PPC_ADDR16_LO = 0x04
    R_PPC_ADDR16_HI = 0x05
    R_PPC_ADDR16_HA = 0x06
    R_PPC_ADDR14 = 0x07
    R_PPC_ADDR14_BRTAKEN = 0x08
    R_PPC_ADDR14_BRNTAKEN = 0x09
    R_PPC_REL24 = 0x0A
    R_PPC_REL14 = 0x0B
    R_PPC_REL14_BRTAKEN = 0x0C
    R_PPC_REL14_BRNTAKEN = 0x0D
    
    R_PPC_REL32 = 0x1A

  offset: int

  target_offset: int
  type: Type
  symbol_index: int
  addend: int

  data: bytes
    
  def __init__(self, data: Cursor):
    """
    Parses a single relocation. Leaves the cursor pointing just after it.
    """
    self.offset = data.tell()
    
    self.target_offset = data.read_u32()
    
    info = data.read_u32()
    self.type = self.Type(info & 0xff)
    self.symbol_index = info >> 8
    self.addend = data.read_u32()

class Symbol:
  """
  A parsed ELF symbol.
  """

  ENTRY_SIZE = 0x10

  class Type(IntEnum):
    """
    A type of ELF symbol.
    """

    STT_NOTYPE         = 0
    STT_OBJECT         = 1
    STT_FUNC           = 2
    STT_SECTION        = 3
    STT_FILE           = 4
    STT_COMMON         = 5
    STT_TLS            = 6
    STT_LOOS           = 10
    STT_HIOS           = 12
    STT_LOPROC         = 13
    STT_SPARC_REGISTER = 13
    STT_HIPROC         = 15

  class Binding(IntEnum):
    STB_LOCAL  = 0
    STB_GLOBAL = 1
    STB_WEAK   = 2
    STB_LOOS   = 10
    STB_HIOS   = 12
    STB_LOPROC = 13
    STB_HIPROC = 15
    
  class SpecialSection(IntEnum):
    SHN_UNDEF     = 0x0000
    SHN_LORESERVE = 0xFF00
    SHN_LOPROC    = 0xFF00
    SHN_HIPROC    = 0xFF1F
    SHN_LOOS      = 0xFF20
    SHN_HIOS      = 0xFF3F
    SHN_ABS       = 0xFFF1
    SHN_COMMON    = 0xFFF2
    SHN_XINDEX    = 0xFFFF
    SHN_HIRESERVE = 0xFFFF
  
  offset: int

  name: str
  name_offset: int
  address: int
  size: int
  type: Type
  binding: Binding
  other: int
  section_index: int

  def __init__(self, data: Cursor):
    """
    Parses a single symbol. Leaves the cursor pointing just after it.
    """
    self.offset = data.tell()
    
    self.name = None
    self.name_offset = data.read_u32()
    self.address = data.read_u32()
    self.size = data.read_u32()

    info = data.read_u8()
    self.type = self.Type(info & 0x0F)
    self.binding = self.Binding(info >> 4)
    self.other = data.read_u8() 
    self.section_index = data.read_u16()