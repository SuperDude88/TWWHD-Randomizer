"""Function discovery for a Wii U RPX.

Seeds function entry points from
  - the ELF entry point
  - direct call targets (bl)
  - relocations that point into .text (function pointers, vtables, jump tables)
and then assigns every instruction in .text to the function whose entry
precedes it. Branch targets that land in a different function are tail calls.
"""
import bisect
import struct
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rpx import Rpx, R_PPC_ADDR32, R_PPC_ADDR16_LO, R_PPC_ADDR16_HA, R_PPC_ADDR16_HI, R_PPC_REL24, SHT_NOBITS


def sext(v, bits):
    return v - (1 << bits) if v & (1 << (bits - 1)) else v


class Program:
    def __init__(self, path):
        self.rpx = r = Rpx(path)
        self.text = r.by_name[".text"]
        self.text_lo = self.text.addr
        self.text_hi = self.text.addr + self.text.size
        self.words = struct.unpack(">%dI" % (self.text.size // 4), self.text.data)
        d = open(path, "rb").read()
        self.entry, = struct.unpack_from(">I", d, 0x18)

        # imports: call sites (REL24 into an import section) -> (lib, name)
        self.import_calls = {}
        self.import_data = {}       # instruction address -> import symbol (data refs)
        self.undef_calls = set()
        # relocations that point into .text (address-taken code)
        self.code_refs = {}         # target -> list of (reloc site, kind)
        for sec, addr, typ, sym, add in r.relocs:
            if sym.import_lib:
                if typ == R_PPC_REL24:
                    self.import_calls[addr] = (sym.import_lib, sym.name, sym.value)
                else:
                    self.import_data[addr] = (sym.import_lib, sym.name, sym.value, typ)
                continue
            if typ == R_PPC_REL24 and sym.name == "$UNDEF":
                self.undef_calls.add(addr)
                continue
            if typ == R_PPC_REL24:
                continue
            tgt = (sym.value + add) & 0xFFFFFFFF
            if self.text_lo <= tgt < self.text_hi and typ in (R_PPC_ADDR32, R_PPC_ADDR16_LO):
                self.code_refs.setdefault(tgt, []).append((addr, sec.name, typ))

    def word(self, addr):
        return self.words[(addr - self.text_lo) >> 2]

    def in_text(self, a):
        return self.text_lo <= a < self.text_hi

    def discover(self):
        entries = {self.entry}
        self.call_targets = set()
        for i, w in enumerate(self.words):
            a = self.text_lo + 4 * i
            op = w >> 26
            if op == 18 and (w & 1):  # bl / bla
                if a in self.import_calls or a in self.undef_calls:
                    continue
                t = (sext(w & 0x03FFFFFC, 26) + (0 if w & 2 else a)) & 0xFFFFFFFF
                if self.in_text(t):
                    self.call_targets.add(t)
            elif op == 16 and (w & 1):  # conditional call (bcl)
                t = (sext(w & 0xFFFC, 16) + (0 if w & 2 else a)) & 0xFFFFFFFF
                if self.in_text(t):
                    self.call_targets.add(t)
        entries |= self.call_targets
        # address-taken code (vtables, pointer-to-member constants, function
        # pointers, constructor tables). Analysis of cking.rpx showed no switch
        # jump tables in data, so every referenced code address is a function entry.
        self.addr_taken = set(self.code_refs)
        self.find_jump_tables()
        entries |= self.addr_taken - set(t for t, _ in self.jump_tables.values())
        self.entries = sorted(entries)
        return self.entries

    def find_jump_tables(self):
        """Switch statements compiled as a table of `b` instructions placed right
        after the dispatching `bctr`:
            cmplwi rX, N ; bgt default ; slwi ; addis/addi rY -> table ; mtctr rY ; bctr
            table: b case0 ; b case1 ; ...
        Returns {bctr address: (table address, entry count)}."""
        self.jump_tables = {}
        for t, refs in self.code_refs.items():
            # the table address must be formed inside the dispatch sequence itself
            if not any(sec == ".text" and t - 40 <= site < t for site, sec, _ in refs):
                continue
            if t - 4 < self.text_lo or self.word(t - 4) != 0x4E800420:  # preceded by bctr
                continue
            # bound from the closest preceding cmplwi (op 10) within the dispatch sequence
            count = None
            for k in range(2, 12):
                w = self.word(t - 4 * k)
                if (w >> 26) == 10:
                    count = (w & 0xFFFF) + 1
                    break
            if count is None:  # fall back to the run of consecutive `b` instructions
                count = 0
                while self.in_text(t + 4 * count) and (self.word(t + 4 * count) >> 26) == 18 and not (self.word(t + 4 * count) & 3):
                    count += 1
            self.jump_tables[t - 4] = (t, count)


if __name__ == "__main__":
    p = Program(sys.argv[1])
    e = p.discover()
    print("entry point %08x" % p.entry)
    print("import call sites", len(p.import_calls), "import data refs", len(p.import_data))
    print("bl targets", len(p.call_targets))
    print("address-taken code targets", len(p.code_refs),
          "(from rodata-only:", sum(1 for t, r in p.code_refs.items() if all(s == ".rodata" for _, s, _ in r)), ")")
    print("jump tables", len(p.jump_tables))
    print("function entries", len(e))
    sizes = [b - a for a, b in zip(e, e[1:] + [p.text_hi])]
    sizes.sort()
    print("size median %d bytes, max %d" % (sizes[len(sizes) // 2], sizes[-1]))
