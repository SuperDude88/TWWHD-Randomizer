#pragma once

#include <string>


//pulled from https://github.com/devkitPro/wut-tools/blob/adf91e060754dc7c64e4727d55f74ea2d6c0a448/src/common/elf.h#L62
enum struct SectionType : uint32_t // sh_type
{
	SHT_NULL = 0,                 // No associated section (inactive entry).
	SHT_PROGBITS = 1,             // Program-defined contents.
	SHT_SYMTAB = 2,               // Symbol table.
	SHT_STRTAB = 3,               // String table.
	SHT_RELA = 4,                 // Relocation entries; explicit addends.
	SHT_HASH = 5,                 // Symbol hash table.
	SHT_DYNAMIC = 6,              // Information for dynamic linking.
	SHT_NOTE = 7,                 // Information about the file.
	SHT_NOBITS = 8,               // Data occupies no space in the file.
	SHT_REL = 9,                  // Relocation entries; no explicit addends.
	SHT_SHLIB = 10,               // Reserved.
	SHT_DYNSYM = 11,              // Symbol table.
	SHT_INIT_ARRAY = 14,          // Pointers to initialization functions.
	SHT_FINI_ARRAY = 15,          // Pointers to termination functions.
	SHT_PREINIT_ARRAY = 16,       // Pointers to pre-init functions.
	SHT_GROUP = 17,               // Section group.
	SHT_SYMTAB_SHNDX = 18,        // Indices for SHN_XINDEX entries.
	SHT_LOPROC = 0x70000000,      // Lowest processor arch-specific type.
	SHT_HIPROC = 0x7fffffff,      // Highest processor arch-specific type.
	SHT_LOUSER = 0x80000000,      // Lowest type reserved for applications.
	SHT_RPL_EXPORTS = 0x80000001, // RPL Exports
	SHT_RPL_IMPORTS = 0x80000002, // RPL Imports
	SHT_RPL_CRCS = 0x80000003,    // RPL CRCs
	SHT_RPL_FILEINFO = 0x80000004,// RPL FileInfo
	SHT_HIUSER = 0xffffffff       // Highest type reserved for applications.
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
	uint16_t shdrIdx;
	uint32_t offset;

	explicit operator bool() {
		if(shdrIdx == 0 && offset == 0) return false;
		return true;
	}
};

typedef std::pair<uint16_t, Elf32_Shdr> shdr_index_t;
