#!/usr/bin/env python3
"""Structure offsets that differ between the USA release and another one, learnt from the game's own code, and the
places in TWWHD Randomizer's patches that use them.

The randomizer's code reaches game structures by fixed displacements (`lhz r0, 0x5B60(r3)`). Where another release
lays a structure out differently, its own code uses other displacements at the same instructions: aligned instruction
pairs of matched functions (port_addresses.Mapper) that differ only in their 16-bit displacement give USA -> other
displacement evidence per register context. Each D-form instruction of the randomizer's USA patches (the shipped
patch diffs) whose displacement is shifted in that evidence, and never seen unchanged, is listed.

usage: offsets.py USA.rpx OTHER.rpx [OUT]   prints the shifted displacements and the patch instructions using them
"""
import collections
import difflib
import os
import sys

import yaml

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from port_addresses import Mapper  # noqa: E402

ASM = os.path.join(HERE, "..")
# D-form: addi(14), lwz(32) lwzu(33) lbz(34) lbzu(35) stw(36) stwu(37) stb(38) stbu(39) lhz(40) lhzu(41) lha(42)
# lhau(43) sth(44) sthu(45) lmw(46) stmw(47) lfs(48) lfsu(49) lfd(50) lfdu(51) stfs(52) stfsu(53) stfd(54) stfdu(55)
DFORM = {14} | set(range(32, 56))


def dform(w):
    op = w >> 26
    if op not in DFORM:
        return None
    ra = (w >> 16) & 31
    d = w & 0xFFFF
    if ra in (1, 2, 13):  # stack, small data: not structures
        return None
    if op == 14 and ra == 0:  # li
        return None
    return op, (w >> 21) & 31, ra, d


def evidence(m):
    """displacement d -> Counter of other-release displacements at aligned instructions (same op and registers)"""
    ev = collections.defaultdict(collections.Counter)
    for i, (j, same) in m.fmap.items():
        wb, wo = m.b.func_words(i), m.o.func_words(j)
        a0, b0 = m.b.entries[i], m.o.entries[j]
        if same:
            pairs = [(a0 + 4 * k, b0 + 4 * k) for k in range(len(wb))]
        else:
            pairs = []
            sm = difflib.SequenceMatcher(None, wb, wo, autojunk=False)
            for tag, i1, i2, j1, j2 in sm.get_opcodes():
                if tag == "equal" or (tag == "replace" and i2 - i1 == j2 - j1):
                    pairs += [(a0 + 4 * (i1 + k), b0 + 4 * (j1 + k)) for k in range(i2 - i1)]
        for a, b in pairs:
            if a in m.b.reloc_at:  # address halves, not structure offsets
                continue
            fb, fo = dform(m.b.p.word(a)), dform(m.o.p.word(b))
            if fb and fo and fb[:3] == fo[:3]:
                ev[fb[3]][fo[3]] += 1
    return ev


def patch_words():
    """(address, word, file) of every instruction the USA patch diffs write"""
    out = []
    for f in sorted(os.listdir(os.path.join(ASM, "patch_diffs"))):
        d = yaml.safe_load(open(os.path.join(ASM, "patch_diffs", f))) or {}
        for a, v in (d.get("Data") or {}).items():
            if a >= 0x10000000:
                continue
            for k in range(0, len(v) - 3, 4):
                out.append((a + k, v[k] << 24 | v[k + 1] << 16 | v[k + 2] << 8 | v[k + 3], f))
    return out


def main():
    m = Mapper(sys.argv[1], sys.argv[2])
    ev = evidence(m)
    shifted = {}
    for d, c in ev.items():
        total = sum(c.values())
        if c.get(d, 0) == 0 and total >= 2:
            other, n = c.most_common(1)[0]
            shifted[d] = (other, n, total)
    lines = ["# displacements shifted in the other release (USA -> other, seen / total)"]
    for d in sorted(shifted):
        o, n, t = shifted[d]
        lines.append("shift %04X %04X %d/%d" % (d, o, n, t))
    mixed = {d: c for d, c in ev.items() if c.get(d, 0) and sum(c.values()) - c[d] >= 2}
    hits = []
    for a, w, f in patch_words():
        x = dform(w)
        if not x:
            continue
        d = x[3]
        if d in shifted:
            hits.append("USE %08X %08X %s: %04X -> %04X (%d/%d)" % (a, w, f, d, *shifted[d]))
        elif d in mixed and d >= 0x100:
            c = mixed[d]
            hits.append("MIXED %08X %08X %s: %04X seen as %s" % (a, w, f, d, dict(c.most_common(4))))
    lines += hits
    text = "\n".join(lines) + "\n"
    if len(sys.argv) > 3:
        open(sys.argv[3], "w").write(text)
    print("%d shifted displacements, %d patch instructions use shifted ones, %d mixed" %
          (len(shifted), sum(1 for h in hits if h.startswith("USE")), sum(1 for h in hits if h.startswith("MIXED"))))
    for h in hits:
        print(h)


if __name__ == "__main__":
    main()
