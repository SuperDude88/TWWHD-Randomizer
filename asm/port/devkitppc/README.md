Stand-ins for devkitPPC's gcc and ld as TWWHD Randomizer's asm/assemble.py calls them, over GNU binutils for
PowerPC (Ubuntu's binutils-powerpc-linux-gnu; no install needed):

    apt download binutils-powerpc-linux-gnu $(apt-cache depends binutils-powerpc-linux-gnu | awk '/Depends/{print $2}' | grep powerpc)
    for d in *.deb; do dpkg -x $d root; done
    PPC_BINUTILS=$PWD/root DEVKITPPC=$PWD/asm/port/devkitppc python3 asm/port/port_rando.py ...

Reassembling the USA patches with them gives the randomizer's shipped patch_diffs byte for byte.
