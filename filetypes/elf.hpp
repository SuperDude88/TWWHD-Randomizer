//ELF files are program binaries

#pragma once

#include <vector>
#include <string>

#include <filetypes/shared/elf_structs.hpp>
#include <filetypes/baseFiletype.hpp>



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
    INDEX_OUT_OF_RANGE,
    UNKNOWN,
    COUNT
};

namespace FileTypes {

    const char* ELFErrorGetName(ELFError err);

    class ELF final : public FileType {
    public:
        Elf32_Ehdr ehdr;
        std::vector<shdr_index_t> shdr_table;

        ELF() = default;
        static ELF createNew();
        ELFError loadFromBinary(std::istream& elf);
        ELFError loadFromFile(const fspath& filePath);
        ELFError extend_section(uint16_t index, const std::string& newData);
        ELFError extend_section(uint16_t index, uint32_t startAddr, const std::string& newData);
        ELFError writeToStream(std::ostream& out);
        ELFError writeToFile(const fspath& outFilePath);
    private:
        bool isEmpty = true;
        void initNew() override;
    };

}
