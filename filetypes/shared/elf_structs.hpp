#pragma once

#include <cstdint>
#include <string>



enum struct SectionType : uint32_t // sh_type
{
    // https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
    SHT_NULL = 0,
    SHT_PROGBITS = 1,
    SHT_SYMTAB = 2,
    SHT_STRTAB = 3,
    SHT_RELA = 4,
    SHT_HASH = 5,
    SHT_DYNAMIC = 6,
    SHT_NOTE = 7,
    SHT_NOBITS = 8,
    SHT_REL = 9,
    SHT_SHLIB = 10,
    SHT_DYNSYM = 11,
    SHT_INIT_ARRAY = 14,
    SHT_FINI_ARRAY = 15,
    SHT_PREINIT_ARRAY = 16,
    SHT_GROUP = 17,
    SHT_SYMTAB_SHNDX = 18,

    // https://refspecs.linuxfoundation.org/LSB_2.1.0/LSB-Core-generic/LSB-Core-generic/elftypes.html
    SHT_LOPROC = 0x70000000,
    SHT_HIPROC = 0x7fffffff,
    SHT_LOUSER = 0x80000000,
    SHT_HIUSER = 0xffffffff,

    // from https://gist.github.com/exjam/b4290ad23828cbc04db4 and looking at the rpx itself
    SHT_RPL_EXPORTS = 0x80000001,
    SHT_RPL_IMPORTS = 0x80000002,
    SHT_RPL_CRCS = 0x80000003,
    SHT_RPL_FILEINFO = 0x80000004
};

enum struct SectionFlags : uint32_t {
    // https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
    SHF_WRITE = 0x1,
    SHF_ALLOC = 0x2,
    SHF_EXECINSTR = 0x4,

    // from https://gist.github.com/exjam/b4290ad23828cbc04db4 and looking at the rpx itself
    SHF_DEFLATED = 0x08000000
};

struct Elf32_Ehdr {
    uint8_t e_ident[0x10];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};

struct Elf32_Shdr {
    uint32_t sh_name;
    SectionType sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;

    std::string data; //not used for rpx
};

struct Elf32_Sym {
    uint32_t st_name;
    uint32_t st_value;
    uint32_t st_size;
    uint8_t  st_info;
    uint8_t  st_other;
    uint16_t st_shndx;
};

struct Elf32_Rela {
    uint32_t r_offset;
    uint32_t r_info;
    int32_t r_addend;
};

struct offset_t {
    uint16_t shdrIdx = 0;
    uint32_t offset = 0;

    explicit operator bool() const {
        if(shdrIdx == 0 && offset == 0) return false;
        return true;
    }
};

using shdr_index_t = std::pair<uint16_t, Elf32_Shdr>;
