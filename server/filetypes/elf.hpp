#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

#include "../utility/byteswap.hpp"



enum struct ELFError
{
	NONE = 0,
	COULD_NOT_OPEN,
	NOT_ELF,
	REACHED_EOF,
	HEADER_DATA_NOT_LOADED,
	SECTION_DATA_NOT_LOADED,
	NOBITS_SECTION_NOT_EMPTY,
	USED_SECTION_IS_EMTPY,
	UNKNOWN,
	COUNT
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

struct Elf32_shdr {
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_addralign;
	uint32_t sh_entsize;
	std::string data; //Not sure if there's a better way to store this
};

struct Elf32_Rela {
	uint32_t r_offset;
	uint32_t r_info; //2 unk bytes, 1 byte for .symtab index, 1 byte for relocation type (04 for low half, 06 for high half, 01 for word)
	int32_t r_addend;
};

namespace FileTypes {

	const char* ELFErrorGetName(ELFError err);

	class ELF {
	public:
		Elf32_Ehdr ehdr;
		std::vector<std::pair<int, Elf32_shdr>> shdr_table; //std::pair so we can have index + section header without the "sort" struct

		ELF();
		static ELF createNew(const std::string& filename);
		ELFError loadFromBinary(std::istream& elf);
		ELFError loadFromFile(const std::string& filePath);
		ELFError extend_section(int index, const std::string& newData);
		ELFError extend_section(int index, uint32_t startAddr, const std::string& newData);
		ELFError writeToStream(std::ostream& out);
		ELFError writeToFile(const std::string& outFilePath);
	private:
		bool isEmpty = true;
		void initNew();
	};

}
