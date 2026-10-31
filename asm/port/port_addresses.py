#!/usr/bin/env python3
"""Maps addresses of one cking.rpx release onto another (e.g. USA -> EUR).

Both releases are the same program with code and data shifted in places. Functions are matched in
order by their instructions with relocated fields masked (branch displacements, address halves),
instructions inside differing functions by an alignment, and data addresses through the
relocations of matched instructions (piecewise constant offsets).

usage:
  port_addresses.py BASE.rpx OTHER.rpx functions OUT       every function: "BASE OTHER" lines
  port_addresses.py BASE.rpx OTHER.rpx code ADDR...        code addresses (any instruction)
  port_addresses.py BASE.rpx OTHER.rpx data ADDR...        data addresses
  port_addresses.py BASE.rpx OTHER.rpx stats               match quality
"""
import bisect
import difflib
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from analyze import Program  # noqa: E402
from rpx import R_PPC_REL24  # noqa: E402


class Side:
    def __init__(self, path):
        self.p = p = Program(path)
        self.entries = p.discover()
        self.ends = self.entries[1:] + [p.text_hi]
        # instruction sites whose immediate is relocated (the field may sit at +2), with the target
        self.reloc_at = {}
        for sec, addr, typ, sym, add in p.rpx.relocs:
            if sec.name != ".text":
                continue
            site = addr & ~3
            tgt = None if (typ == R_PPC_REL24 or sym.import_lib) else (sym.value + add) & 0xFFFFFFFF
            self.reloc_at[site] = (typ, tgt)
        self.norm = {}

    def norm_word(self, a):
        w = self.p.word(a)
        op = w >> 26
        if a in self.reloc_at:
            return w & 0xFFFF0000 if op != 18 else w & 0xFC000003
        if op == 18:  # b / bl: target differs when code moved
            return w & 0xFC000003
        return w

    def func_words(self, i):
        a, e = self.entries[i], self.ends[i]
        key = i
        if key not in self.norm:
            self.norm[key] = tuple(self.norm_word(x) for x in range(a, e, 4))
        return self.norm[key]


def match(base, other):
    """function index maps base -> other"""
    hb = [hash(base.func_words(i)) for i in range(len(base.entries))]
    ho = [hash(other.func_words(i)) for i in range(len(other.entries))]
    sm = difflib.SequenceMatcher(None, hb, ho, autojunk=False)
    fmap, changed = {}, []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            for k in range(i2 - i1):
                fmap[i1 + k] = (j1 + k, True)
        elif tag == "replace":
            # same count: pair in order (changed functions); otherwise pair by best similarity
            if i2 - i1 == j2 - j1:
                for k in range(i2 - i1):
                    fmap[i1 + k] = (j1 + k, False)
                    changed.append(i1 + k)
            else:
                for i in range(i1, i2):
                    best, bj = 0.0, None
                    for j in range(j1, j2):
                        r = difflib.SequenceMatcher(None, base.func_words(i), other.func_words(j), autojunk=False).quick_ratio()
                        if r > best:
                            best, bj = r, j
                    if bj is not None and best > 0.6:
                        fmap[i] = (bj, False)
                        changed.append(i)
    return fmap, changed


class Mapper:
    def __init__(self, base_path, other_path):
        self.b, self.o = Side(base_path), Side(other_path)
        self.fmap, self.changed = match(self.b, self.o)
        self._data = None

    def func_index(self, side, a):
        i = bisect.bisect_right(side.entries, a) - 1
        return i if i >= 0 and a < side.ends[i] else None

    def code(self, a):
        """other address of the instruction at base address a, or None"""
        i = self.func_index(self.b, a)
        if i is None or i not in self.fmap:
            return None
        j, same = self.fmap[i]
        off = a - self.b.entries[i]
        if same:
            return self.o.entries[j] + off
        wb, wo = self.b.func_words(i), self.o.func_words(j)
        sm = difflib.SequenceMatcher(None, wb, wo, autojunk=False)
        k = off // 4
        for tag, i1, i2, j1, j2 in sm.get_opcodes():
            if i1 <= k < i2:
                if tag == "equal" or (tag == "replace" and i2 - i1 == j2 - j1):
                    return self.o.entries[j] + 4 * (j1 + k - i1)
                return None
        return None

    def data_pairs(self):
        """(base target, other target) of relocated addresses in matched code, sorted"""
        if self._data is None:
            pairs = {}
            for i, (j, same) in self.fmap.items():
                if not same:
                    continue
                a0, b0 = self.b.entries[i], self.o.entries[j]
                for a in range(a0, self.b.ends[i], 4):
                    rb = self.b.reloc_at.get(a)
                    ro = self.o.reloc_at.get(b0 + a - a0)
                    if rb and ro and rb[1] is not None and ro[1] is not None and rb[1] >= 0x10000000:
                        pairs.setdefault(rb[1], set()).add(ro[1])
            self._data = sorted((k, min(v)) for k, v in pairs.items() if len(v) == 1)
        return self._data

    def data(self, a):
        """other address of base data address a: the offset of the nearest relocated target at or
        below it (struct fields and array elements share their base's offset)"""
        d = self.data_pairs()
        k = bisect.bisect_right(d, (a, 0xFFFFFFFF)) - 1
        if k < 0:
            return None
        lo, lo_o = d[k]
        delta = lo_o - lo
        # sanity: the next pair agrees, or the address is close to the one below
        if k + 1 < len(d) and d[k + 1][1] - d[k + 1][0] != delta and a - lo > 0x10000:
            return None
        return (a + delta) & 0xFFFFFFFF


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        sys.exit(1)
    m = Mapper(sys.argv[1], sys.argv[2])
    cmd = sys.argv[3]
    if cmd == "stats":
        n = len(m.b.entries)
        same = sum(1 for v in m.fmap.values() if v[1])
        print("functions: base %d, other %d; matched %d (identical %d, changed %d), unmatched %d"
              % (n, len(m.o.entries), len(m.fmap), same, len(m.changed), n - len(m.fmap)))
        d = m.data_pairs()
        deltas = []
        for a, o in d:
            if not deltas or deltas[-1][1] != o - a:
                deltas.append((a, o - a))
        print("data: %d relocated targets, %d offset ranges" % (len(d), len(deltas)))
        for a, dl in deltas[:40]:
            print("  from %08X: %+X" % (a, dl))
    elif cmd == "functions":
        with open(sys.argv[4], "w") as f:
            for i, (j, same) in sorted(m.fmap.items()):
                f.write("%08X %08X%s\n" % (m.b.entries[i], m.o.entries[j], "" if same else " changed"))
    elif cmd in ("code", "data"):
        for s in sys.argv[4:]:
            a = int(s, 16)
            r = m.code(a) if cmd == "code" else m.data(a)
            print("%08X %s" % (a, "%08X" % r if r is not None else "?"))


if __name__ == "__main__":
    main()
