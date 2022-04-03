#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "./shared/elf_structs.hpp"



enum struct [[nodiscard]] ELFError
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

namespace FileTypes {

	const char* ELFErrorGetName(ELFError err);

	class ELF {
	public:
		Elf32_Ehdr ehdr;
		std::vector<shdr_index_t> shdr_table; //std::pair so we can have index + section header without the "sort" struct

		ELF();
		static ELF createNew(const std::string& filename);
		ELFError loadFromBinary(std::istream& elf);
		ELFError loadFromFile(const std::string& filePath);
		ELFError extend_section(const uint16_t index, const std::string& newData);
		ELFError extend_section(const uint16_t index, const uint32_t startAddr, const std::string& newData);
		ELFError writeToStream(std::ostream& out);
		ELFError writeToFile(const std::string& outFilePath);
	private:
		bool isEmpty = true;
		void initNew();
	};

}
