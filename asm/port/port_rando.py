#!/usr/bin/env python3
"""Ports TWWHD Randomizer's executable patches (USA) to the EUR or JPN release.

The randomizer's patches (asm/patches/*.asm) are written for the USA cking.rpx:
`.org` lines at USA addresses, literal branch targets, and the game's symbols in asm/linker.ld. This tool maps every
one of them onto the other release with port_addresses.py (functions matched by their instructions,
instructions inside changed functions by alignment, data by the relocations of matched code), checks that the code or
data each patch replaces is the same there (branches compared by their mapped targets, relocated fields by their mapped
addresses), writes the ported sources and assembles them with the randomizer's own asm/assemble.py (free space after
that release's .text). It also maps the USA addresses the randomizer's C++ code and location data write to
(address_map.txt, read by utility/region.cpp) and checks their bytes.

usage: port_rando.py USA.rpx OTHER.rpx eur|jpn OUT_DIR
  OUT_DIR gets custom_symbols.yaml, patch_diffs/, address_map.txt, report.txt (any mismatch fails the run).
  Needs $DEVKITPPC (powerpc-eabi-gcc / -ld; devkitppc/README.md has stand-ins over GNU binutils).
  The results go to asm/eur and asm/jpn: python3 asm/port/port_rando.py USA.rpx EUR.rpx eur asm/eur
"""
import os
import re
import shutil
import subprocess
import sys
import tempfile

import yaml

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from port_addresses import Mapper, match  # noqa: E402
import offsets  # noqa: E402
from rpx import Rpx  # noqa: E402

RANDO = os.path.join(HERE, "..", "..")
ASM = os.path.join(RANDO, "asm")
ORG = re.compile(r"^(\s*\.org\s+)0x([0-9A-Fa-f]+)(.*)$")
BRANCH = re.compile(r"^(\s*b[a-z+\-]*\s+(?:[^;,]*,\s*)?)0x([0-9A-Fa-f]{7,8})\b(.*)$", re.I)
SYM = re.compile(r"^(\s*)([A-Za-z_][\w$.]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;(.*)$")
DISP = re.compile(r"\b0x([0-9A-Fa-f]{3,4})\s*\(\s*r?(\d+|sp)\s*\)", re.I)
TEXT_LO, DATA_LO, DATA_HI = 0x02000000, 0x10000000, 0x10800000  # (.bss ends below DATA_HI)


class Port:
    def __init__(self, usa, other):
        self.m = Mapper(usa, other)
        self.ru, self.ro = Rpx(usa), Rpx(other)
        self.problems = []
        self.text_hi_u = self.m.b.p.text_hi
        self.text_hi_o = self.m.o.p.text_hi
        # structure fields at other displacements in that release (offsets.py): d -> other d, when at least 80 % of
        # the game's own aligned instructions with d use the other one
        self.shift = {}
        for d, c in offsets.evidence(self.m).items():
            o, n = c.most_common(1)[0]
            if o != d and n >= 2 and n >= 0.8 * sum(c.values()):
                self.shift[d] = o
        # then the functions matched again with those fields taken back to the USA displacements: a function that
        # differs only by them is the same, and inside changed ones the alignment can't pair one moved field's
        # instruction with another field's (USA 0x5B60 and JPN 0x5B60 are different fields)
        if self.shift:
            back = {}
            for d, o in self.shift.items():
                back.setdefault(o, []).append(d)
            back = {o: ds[0] for o, ds in back.items() if len(ds) == 1}
            o_side, plain = self.m.o, self.m.o.norm_word

            def norm(a):
                w = plain(a)
                f = offsets.dform(o_side.p.word(a))
                if f and a not in o_side.reloc_at and f[3] in back:
                    w = (w & 0xFFFF0000) | back[f[3]]
                return w
            o_side.norm_word = norm
            o_side.norm = {}
            self.m.fmap, self.m.changed = match(self.m.b, o_side)
            self.m._data = None

    # ---- bytes of either release
    @staticmethod
    def _read(r, a, n):
        for s in r.sections:
            d = s.data or b""
            if s.addr and s.addr <= a and a + n <= s.addr + len(d):
                return d[a - s.addr:a - s.addr + n]
        return None

    def is_code(self, a):
        return TEXT_LO <= a < self.text_hi_u

    def map(self, a):
        """the other release's address of USA address a (code: the same instruction; data: the same object), or None"""
        if self.is_code(a):
            o = self.m.code(a & ~3)  # (a byte inside an instruction: the same byte of the other's)
            return None if o is None else o + (a & 3)
        if DATA_LO <= a < DATA_HI:
            return self.m.data(a)
        return None

    def map_symbol(self, a):
        if self.is_code(a):
            r = self.m.code(a)
            # a function entry: the matched function's entry (the alignment may not cover a changed prologue)
            i = self.m.func_index(self.m.b, a)
            if i is not None and i in self.m.fmap and self.m.b.entries[i] == a:
                r = self.m.o.entries[self.m.fmap[i][0]]
            return r
        if not DATA_LO <= a < DATA_HI:
            return a  # (an address outside the executable, e.g. in the heap: the same in every release)
        return self.m.data(a)

    def _branch_target(self, side, a, w):
        op = w >> 26
        if op == 18:
            d = w & 0x03FFFFFC
            if d & 0x02000000:
                d -= 0x04000000
            return (d if w & 2 else a + d) & 0xFFFFFFFF
        if op == 16:
            d = w & 0xFFFC
            if d & 0x8000:
                d -= 0x10000
            return (d if w & 2 else a + d) & 0xFFFFFFFF
        return None

    def same_code(self, ua, n):
        """the USA instructions [ua, ua+n) and the other release's at their mapped places are the same program;
        returns (other address, why not or None)"""
        oa = self.map(ua)
        if oa is None:
            return None, "no mapping"
        for k in range(0, n, 4):
            u, o = ua + k, oa + k
            if self.map(u) != o:
                return oa, "not contiguous at +%X (USA %08X -> %s)" % (k, u, "%08X" % self.map(u) if self.map(u) else "?")
            wu, wo = self.m.b.p.word(u), self.m.o.p.word(o)
            if wu == wo:
                continue
            tu, to = self._branch_target("u", u, wu), self._branch_target("o", o, wo)
            if tu is not None and to is not None and (wu & ~0x03FFFFFC if wu >> 26 == 18 else wu & ~0xFFFC) == \
                    (wo & ~0x03FFFFFC if wo >> 26 == 18 else wo & ~0xFFFC):
                mt = self.map(tu)
                if mt == to:
                    continue
                return oa, "branch at +%X: USA -> %08X (maps to %s), other -> %08X" % (k, tu, "%08X" % mt if mt else "?", to)
            fu, fo = offsets.dform(wu), offsets.dform(wo)
            if fu and fo and fu[:3] == fo[:3] and self.shift.get(fu[3]) == fo[3]:
                continue  # (the same field, moved in that release's structure)
            ru, ro = self.m.b.reloc_at.get(u), self.m.o.reloc_at.get(o)
            if ru and ro and (wu & 0xFFFF0000) == (wo & 0xFFFF0000) and ru[1] is not None and ro[1] is not None:
                if self.m.data(ru[1]) == ro[1] or self.map(ru[1]) == ro[1]:
                    continue
            return oa, "word at +%X: USA %08X, other %08X" % (k, wu, wo)
        return oa, None

    def same_data(self, ua, n):
        oa = self.map(ua)
        if oa is None:
            return None, "no mapping"
        bu, bo = self._read(self.ru, ua, n), self._read(self.ro, oa, n)
        if bu is None or bo is None:
            return oa, "outside the sections"
        if bu != bo:
            return oa, "bytes differ: USA %s, other %s" % (bu.hex(), bo.hex())
        return oa, None


def port_asm(p, work, region):
    """ported copy of asm/ in `work`; returns the report lines"""
    report = []
    # each patched USA range: the shipped USA diffs give the lengths
    lengths = {}
    for f in os.listdir(os.path.join(ASM, "patch_diffs")):
        d = yaml.safe_load(open(os.path.join(ASM, "patch_diffs", f))) or {}
        for a, v in (d.get("Data") or {}).items():
            lengths[a] = len(v)
    shutil.copytree(ASM, work, ignore=lambda d, names: ["port", "eur", "jpn"] if os.path.samefile(d, ASM) else [])
    for name in sorted(os.listdir(os.path.join(work, "patches"))):
        if not name.endswith(".asm"):
            continue
        path = os.path.join(work, "patches", name)
        out = []
        for ln, line in enumerate(open(path).read().split("\n"), 1):
            code, sep, comment = line.partition(";")
            m = ORG.match(code)
            if m:
                ua = int(m.group(2), 16)
                n = lengths.get(ua, 4)
                if p.is_code(ua):
                    oa, why = p.same_code(ua, n)
                else:
                    oa, why = p.same_data(ua, n)
                if why:
                    p.problems.append("%s:%d .org %08X (+%X): %s" % (name, ln, ua, n, why))
                report.append("org %08X %s %s:%d" % (ua, "%08X" % oa if oa else "?", name, ln))
                if oa is not None:
                    code = "%s0x%08X%s" % (m.group(1), oa, m.group(3))
            else:
                m = BRANCH.match(code)
                if m:
                    ua = int(m.group(2), 16)
                    oa = p.map(ua)
                    if oa is None:
                        p.problems.append("%s:%d branch to %08X: no mapping" % (name, ln, ua))
                    else:
                        code = "%s0x%08X%s" % (m.group(1), oa, m.group(3))
                    report.append("branch %08X %s %s:%d" % (ua, "%08X" % oa if oa else "?", name, ln))
            # structure fields that moved in that release (r1 stack, r2 / r13 small data excluded)
            def disp(mm):
                d = int(mm.group(1), 16)
                if mm.group(2) in ("1", "2", "13", "sp") or d not in p.shift:
                    return mm.group(0)
                report.append("field %04X -> %04X %s:%d" % (d, p.shift[d], name, ln))
                return "0x%X(%s" % (p.shift[d], mm.group(0)[mm.group(0).index("(") + 1:])
            code = DISP.sub(disp, code)
            out.append(code + sep + comment)
        open(path, "w").write("\n".join(out))
    # the game's symbols
    out = []
    for line in open(os.path.join(work, "linker.ld")).read().split("\n"):
        m = SYM.match(line)
        if m:
            ua = int(m.group(3), 16)
            oa = p.map_symbol(ua)
            if oa is None:
                p.problems.append("linker.ld %s %08X: no mapping" % (m.group(2), ua))
            else:
                # a function's first instruction or a data object's first bytes must be the same
                why = p.same_code(ua, 4)[1] if p.is_code(ua) else p.same_data(ua, 4)[1]
                if why and p.is_code(ua):
                    p.problems.append("linker.ld %s %08X -> %08X: %s" % (m.group(2), ua, oa, why))
                elif why:
                    report.append("note: linker.ld %s (data) %08X -> %08X: %s" % (m.group(2), ua, oa, why))
                line = "%s%s = 0x%08x;%s" % (m.group(1), m.group(2), oa, m.group(4))
            report.append("symbol %s %08X %s" % (m.group(2), ua, "%08X" % oa if oa else "?"))
        out.append(line)
    open(os.path.join(work, "linker.ld"), "w").write("\n".join(out))
    # free space after this release's .text
    s = open(os.path.join(work, "assemble.py")).read()
    s = s.replace("FREE_SPACE_START = 0x%08X" % p.text_hi_u, "FREE_SPACE_START = 0x%08X" % p.text_hi_o)
    assert "FREE_SPACE_START = 0x%08X" % p.text_hi_o in s, "assemble.py's FREE_SPACE_START not found"
    open(os.path.join(work, "assemble.py"), "w").write(s)
    return report


# ---- the USA addresses the C++ code and the location data write to
CPP_FILES = ["tweaks.cpp", "command/WriteLocations.cpp", "command/WriteEntrances.cpp", "command/WriteCharts.cpp",
             "randomizer.cpp", "text_replacements.cpp", "logic/Location.cpp"]
LIT = re.compile(r"0x(0?2[0-9A-Fa-f]{6}|10[0-9A-Fa-f]{6})\b")


def cpp_addresses():
    found = {}
    for f in CPP_FILES:
        path = os.path.join(RANDO, f)
        if not os.path.exists(path):
            continue
        for ln, line in enumerate(open(path, encoding="utf-8", errors="replace"), 1):
            for m in LIT.finditer(line):
                found.setdefault(int(m.group(1), 16), "%s:%d" % (f, ln))
    # the logic data names executable addresses under several keys (Offsets, Paths: code/cking.rpx@RPX@ELF)
    for f in sorted(os.listdir(os.path.join(RANDO, "logic", "data"))):
        if not f.endswith(".yaml") or "example" in f:
            continue
        for ln, line in enumerate(open(os.path.join(RANDO, "logic", "data", f), encoding="utf-8"), 1):
            for m in LIT.finditer(line):
                found.setdefault(int(m.group(1), 16), "%s:%d" % (f, ln))
    return found


# data tables the C++ code indexes (base, entry size, entries): checked whole, pointers by their mapped targets
TABLES = [(0x101E4674, 0x24, 256, "item resources"), (0x101E6A74, 0x1C, 256, "field item resources"),
          (0x101E8674, 4, 256, "item info"), (0x101EAEA4, 0x10, 14, "shop item data"),
          (0x1003A930, 0x20, 256, "shop item display data"), (0x101CB424, 0x9C, 13, "block move data")]
# .rela.text entries the C++ code changes or removes by their byte offset in the table (tweaks.cpp)
RELA_TEXT = [0x14EC04, 0x14EC4C, 0x75170, 0x153B00, 0x153B48, 0x148820, 0x1482A4, 0x148430, 0x148310, 0x14E2D4,
             0x1C0AE8, 0x160224, 0x199854, 0x1C1ED4]  # (the last four: entries the randomizer removes)
# .rodata offsets the C++ code writes as relocation addends (with RELA_TEXT)
RODATA_OFFSETS = [0x355C4, 0x10FFC, 0x35AAC, 0x358D8, 0x35124, 0x35530]


def check_tables(p, notes):
    for base, size, count, name in TABLES:
        ob = p.map(base)
        if ob is None:
            p.problems.append("table %s %08X: no mapping" % (name, base))
            continue
        n = size * count
        bu, bo = p._read(p.ru, base, n), p._read(p.ro, ob, n)
        if bu is None or bo is None:
            p.problems.append("table %s %08X: outside the sections" % (name, base))
            continue
        bad = 0
        for k in range(0, n - 3, 4):
            wu = int.from_bytes(bu[k:k + 4], "big")
            wo = int.from_bytes(bo[k:k + 4], "big")
            if wu == wo:
                continue
            if (TEXT_LO <= wu < DATA_HI) and p.map(wu) == wo:
                continue  # a pointer, to the same thing
            bad += 1
            if bad <= 3:
                notes.append("note: table %s +%X: USA %08X, other %08X" % (name, k, wu, wo))
        if bad:
            p.problems.append("table %s %08X -> %08X: %d words differ" % (name, base, ob, bad))
        else:
            notes.append("table %s %08X -> %08X ok (%d entries)" % (name, base, ob, count))


def rela_entries(r):
    s = r.by_name[".rela.text"]
    return [(k, int.from_bytes(s.data[k:k + 4], "big"), int.from_bytes(s.data[k + 4:k + 8], "big"))
            for k in range(0, len(s.data) - 11, 12)]


def map_rela(p, amap):
    ou = rela_entries(p.ru)
    by_target = {}
    for k, off, info in rela_entries(p.ro):
        by_target.setdefault((off, info & 0xFF), []).append(k)
    for t in RELA_TEXT:
        k0, field = t - t % 12, t % 12
        _, off, info = ou[k0 // 12]
        oo = p.map(off & ~3)
        cand = by_target.get(((oo + (off & 3)) if oo is not None else -1, info & 0xFF), [])
        if len(cand) != 1:
            p.problems.append(".rela.text +%X (USA r_offset %08X): %d entries in the other release" % (t, off, len(cand)))
            continue
        amap.append("rela %08X %08X" % (t, cand[0] + field))
    for v in RODATA_OFFSETS:
        o = p.map(DATA_LO + v)
        if o is None:
            p.problems.append(".rodata +%X: no mapping" % v)
            continue
        amap.append("at %08X %08X" % (DATA_LO + v, o))


def main():
    if len(sys.argv) != 5:
        print(__doc__)
        sys.exit(1)
    usa, other, region, out = sys.argv[1:]
    p = Port(usa, other)
    tmp = tempfile.mkdtemp()
    work = os.path.join(tmp, "asm")
    report = port_asm(p, work, region)
    # C++ addresses: explicit lines (the C++ side uses them first, then the ranges for computed addresses)
    amap = ["# USA -> %s addresses of TWWHD Randomizer's C++ code and location data (asm/port/port_rando.py)" % region.upper(),
            "text_end %08X %08X" % (p.text_hi_u, p.text_hi_o)]
    for a, where in sorted(cpp_addresses().items()):
        oa = p.map(a)
        if oa is None:
            # (sections' start addresses and constants that only look like addresses: handled in the C++ code)
            report.append("note: C++ %08X (%s): no mapping" % (a, where))
            continue
        why = p.same_code(a & ~3, 4)[1] if p.is_code(a) else p.same_data(a, 4)[1]
        if why:
            report.append("note: C++ %08X -> %08X (%s): %s" % (a, oa, where, why))
        amap.append("at %08X %08X" % (a, oa))
    check_tables(p, report)
    map_rela(p, amap)
    # ranges for computed addresses (table entries): data offsets by the relocation pairs, code by matched functions
    d = p.m.data_pairs()
    last = None
    for a, o in d:
        if last is None or o - a != last:
            amap.append("data %08X %+X" % (a, o - a))
            last = o - a
    for i, (j, same) in sorted(p.m.fmap.items()):
        if same:
            amap.append("func %08X %08X %X" % (p.m.b.entries[i], p.m.o.entries[j], p.m.b.ends[i] - p.m.b.entries[i]))
    os.makedirs(out, exist_ok=True)
    open(os.path.join(out, "address_map.txt"), "w").write("\n".join(amap) + "\n")
    open(os.path.join(out, "report.txt"), "w").write("\n".join(p.problems + [""] + report) + "\n")
    if p.problems:
        print("%d problems (see %s/report.txt):" % (len(p.problems), out))
        for x in p.problems[:60]:
            print("  " + x)
        sys.exit(2)
    r = subprocess.run([sys.executable, "assemble.py"], cwd=work, capture_output=True, text=True)
    if r.returncode != 0:
        print(r.stdout[-3000:], r.stderr[-3000:])
        sys.exit(3)
    if os.path.exists(os.path.join(out, "patch_diffs")):
        shutil.rmtree(os.path.join(out, "patch_diffs"))
    shutil.copytree(os.path.join(work, "patch_diffs"), os.path.join(out, "patch_diffs"))
    shutil.copy(os.path.join(work, "custom_symbols.yaml"), out)
    shutil.rmtree(tmp)
    print("ok: %s (free space from %08X)" % (out, p.text_hi_o))


if __name__ == "__main__":
    main()
