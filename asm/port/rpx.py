"""Wii U RPX/RPL container: sections, symbols, relocations."""
import struct
import zlib

SHT_SYMTAB, SHT_RELA, SHT_NOBITS = 2, 4, 8
SHT_RPL_IMPORTS = 0x80000002
SHF_RPL_ZLIB = 0x08000000

R_PPC_ADDR32, R_PPC_ADDR16_LO, R_PPC_ADDR16_HI, R_PPC_ADDR16_HA, R_PPC_REL24 = 1, 4, 5, 6, 10


class Section:
    pass


class Symbol:
    pass


class Rpx:
    def __init__(self, path):
        d = open(path, "rb").read()
        shoff, = struct.unpack_from(">I", d, 0x20)
        shentsize, shnum, shstrndx = struct.unpack_from(">HHH", d, 0x2E)
        self.sections = []
        for i in range(shnum):
            s = Section()
            (s.name_off, s.type, s.flags, s.addr, s.offset, s.file_size,
             s.link, s.info, s.align, s.entsize) = struct.unpack_from(">10I", d, shoff + i * shentsize)
            s.index = i
            if s.type == SHT_NOBITS or s.file_size == 0:
                s.data = b""
                s.size = s.file_size
            else:
                raw = d[s.offset:s.offset + s.file_size]
                if s.flags & SHF_RPL_ZLIB:
                    raw = zlib.decompress(raw[4:])
                s.data = raw
                s.size = len(raw)
            self.sections.append(s)
        shstr = self.sections[shstrndx].data
        for s in self.sections:
            s.name = _cstr(shstr, s.name_off)
        self.by_name = {s.name: s for s in self.sections}
        self._load_symbols()
        self._load_relocs()

    def _load_symbols(self):
        self.symbols = []
        for s in self.sections:
            if s.type != SHT_SYMTAB:
                continue
            strtab = self.sections[s.link].data
            for j in range(len(s.data) // 16):
                sym = Symbol()
                name, sym.value, sym.size, info, _other, sym.shndx = struct.unpack_from(">IIIBBH", s.data, j * 16)
                sym.name = _cstr(strtab, name)
                sym.type, sym.bind = info & 0xF, info >> 4
                sym.section = self.sections[sym.shndx] if 0 < sym.shndx < len(self.sections) else None
                sym.import_lib = None
                if sym.section is not None and sym.section.type == SHT_RPL_IMPORTS:
                    sym.import_lib = sym.section.name.split("_", 1)[1]
                    sym.import_kind = sym.section.name[1]  # 'f' function, 'd' data
                self.symbols.append(sym)

    def _load_relocs(self):
        """list of (target section, offset-address, type, symbol, addend)"""
        self.relocs = []
        for s in self.sections:
            if s.type != SHT_RELA:
                continue
            target = self.sections[s.info]
            for k in range(len(s.data) // 12):
                off, info, addend = struct.unpack_from(">IIi", s.data, k * 12)
                self.relocs.append((target, off, info & 0xFF, self.symbols[info >> 8], addend))


def _cstr(buf, off):
    return buf[off:buf.index(b"\0", off)].decode("utf-8", "replace")
