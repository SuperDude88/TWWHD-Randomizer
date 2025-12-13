#include "elf.hpp"

#include <cstring>
#include <algorithm>

#include <utility/endian.hpp>
#include <utility/common.hpp>
#include <utility/file.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;

namespace FileTypes {

    const char* ELFErrorGetName(const ELFError err) {
        switch (err) {
            case ELFError::NONE:
                return "NONE";
            case ELFError::COULD_NOT_OPEN:
                return "COULD_NOT_OPEN";
            case ELFError::NOT_ELF:
                return "NOT_ELF";
            case ELFError::REACHED_EOF:
                return "REACHED_EOF";
            case ELFError::HEADER_DATA_NOT_LOADED:
                return "HEADER_DATA_NOT_LOADED";
            case ELFError::SECTION_DATA_NOT_LOADED:
                return "SECTION_DATA_NOT_LOADED";
            case ELFError::NOBITS_SECTION_NOT_EMPTY:
                return "NOBITS_SECTION_NOT_EMPTY";
            case ELFError::USED_SECTION_IS_EMTPY:
                return "USED_SECTION_IS_EMPTY";
            case ELFError::INDEX_OUT_OF_RANGE:
                return "INDEX_OUT_OF_RANGE";
            default:
                return "UNKNOWN";
        }
    }

    void ELF::initNew() {
        isEmpty = false;
        memcpy(ehdr.e_ident, "\x7F\x45\x4C\x46\x01\x02\x01\xCA\xFE\x00\x00\x00\x00\x00\x00\x00", 0x10);
        ehdr.e_type = 0xFE01;
        ehdr.e_machine = 0x0014;
        ehdr.e_version = 1;
        ehdr.e_entry = 0; // Not sure if this should be the default
        ehdr.e_phoff = 0; // Wii U doesn't have a program header
        ehdr.e_shoff = 0x40;
        ehdr.e_flags = 0;
        ehdr.e_ehsize = 0x34;
        ehdr.e_phentsize = 0;
        ehdr.e_phnum = 0;
        ehdr.e_shentsize = 0x28;
        ehdr.e_shnum = 0;
        ehdr.e_shstrndx = 0x1e;

        shdr_table = {};
    }

    ELF ELF::createNew() {
        ELF newELF{};
        newELF.initNew();
        return newELF;
    }

    ELFError ELF::loadFromBinary(std::istream& elf) {
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_ident), sizeof(ehdr.e_ident))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }
        if(memcmp(ehdr.e_ident, "\x7F\x45\x4C\x46", 4) != 0) {
            LOG_ERR_AND_RETURN(ELFError::NOT_ELF);
        }
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_type), sizeof(ehdr.e_type))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_machine), sizeof(ehdr.e_machine))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_version), sizeof(ehdr.e_version))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_entry), sizeof(ehdr.e_entry))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_phoff), sizeof(ehdr.e_phoff))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_shoff), sizeof(ehdr.e_shoff))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_flags), sizeof(ehdr.e_flags))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_ehsize), sizeof(ehdr.e_ehsize))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_phentsize), sizeof(ehdr.e_phentsize))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_phnum), sizeof(ehdr.e_phnum))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_shentsize), sizeof(ehdr.e_shentsize))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_shnum), sizeof(ehdr.e_shnum))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }
        if(!elf.read(reinterpret_cast<char*>(&ehdr.e_shstrndx), sizeof(ehdr.e_shstrndx))) {
            LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_type);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_machine);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_version);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_entry);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_phoff);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_shoff);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_flags);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_ehsize);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_phentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_phnum);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_shentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_shnum);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_shstrndx);

        shdr_table.reserve(ehdr.e_shnum); // Allocate the memory from the start to minimize copies
        for (unsigned int i = 0; i < ehdr.e_shnum; i++) {
            Elf32_Shdr shdr;
            elf.seekg(ehdr.e_shoff + ehdr.e_shentsize * i, std::ios::beg);
            if(!elf.read(reinterpret_cast<char*>(&shdr.sh_name), sizeof(shdr.sh_name))) {
                LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
            }
            if(!elf.read(reinterpret_cast<char*>(&shdr.sh_type), sizeof(shdr.sh_type))) {
                LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
            }
            if(!elf.read(reinterpret_cast<char*>(&shdr.sh_flags), sizeof(shdr.sh_flags))) {
                LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
            }
            if(!elf.read(reinterpret_cast<char*>(&shdr.sh_addr), sizeof(shdr.sh_addr))) {
                LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
            }
            if(!elf.read(reinterpret_cast<char*>(&shdr.sh_offset), sizeof(shdr.sh_offset))) {
                LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
            }
            if(!elf.read(reinterpret_cast<char*>(&shdr.sh_size), sizeof(shdr.sh_size))) {
                LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
            }
            if(!elf.read(reinterpret_cast<char*>(&shdr.sh_link), sizeof(shdr.sh_link))) {
                LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
            }
            if(!elf.read(reinterpret_cast<char*>(&shdr.sh_info), sizeof(shdr.sh_info))) {
                LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
            }
            if(!elf.read(reinterpret_cast<char*>(&shdr.sh_addralign), sizeof(shdr.sh_addralign))) {
                LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
            }
            if(!elf.read(reinterpret_cast<char*>(&shdr.sh_entsize), sizeof(shdr.sh_entsize))) {
                LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
            }

            Utility::Endian::toPlatform_inplace(eType::Big, shdr.sh_name);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr.sh_type);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr.sh_flags);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr.sh_addr);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr.sh_offset);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr.sh_size);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr.sh_link);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr.sh_info);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr.sh_addralign);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr.sh_entsize);

            if (shdr.sh_type != SectionType::SHT_NOBITS) {
                elf.seekg(shdr.sh_offset, std::ios::beg);
                shdr.data.resize(shdr.sh_size);
                if(!elf.read(&shdr.data[0], shdr.sh_size)) {
                    LOG_ERR_AND_RETURN(ELFError::REACHED_EOF);
                }
                if(shdr.sh_type != SectionType::SHT_NULL && shdr.sh_size == 0) { // If type isn't nobits or NULL and the size is 0, should not happen for WWHD
                    LOG_ERR_AND_RETURN(ELFError::USED_SECTION_IS_EMTPY);
                }
            }
            else { // Don't read data if section is nobits
                if(shdr.sh_offset != 0) {
                    LOG_ERR_AND_RETURN(ELFError::NOBITS_SECTION_NOT_EMPTY); // Offset in file should always be 0
                }
            }
            shdr_table.emplace_back(i, shdr);
        }

        isEmpty = false;

        return ELFError::NONE;
    }

    ELFError ELF::loadFromFile(const fspath& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if(!file.is_open()) {
            LOG_ERR_AND_RETURN(ELFError::COULD_NOT_OPEN);
        }
        return loadFromBinary(file);
    }

    ELFError ELF::extend_section(uint16_t index, const std::string& newData) { // newData is data to append, not replace
        if (isEmpty == true) {
            LOG_ERR_AND_RETURN(ELFError::HEADER_DATA_NOT_LOADED);
        }

        // Assume items are sorted by index, always should be
        if (shdr_table[index].second.data.empty()) { // Can't append data to an empty section
            LOG_ERR_AND_RETURN(ELFError::SECTION_DATA_NOT_LOADED);
        }

        shdr_table[index].second.sh_size += newData.size();
        shdr_table[index].second.data.append(newData);
        return ELFError::NONE;
    }

    ELFError ELF::extend_section(uint16_t index, uint32_t startAddr, const std::string& newData) { // Add new data starting at an offset
        if (isEmpty == true) {
            LOG_ERR_AND_RETURN(ELFError::HEADER_DATA_NOT_LOADED);
        }

        // Assume items are sorted by index, always should be
        if (shdr_table[index].second.data.empty()) { // Wouldn't append data to a section that didn't already have some
            LOG_ERR_AND_RETURN(ELFError::SECTION_DATA_NOT_LOADED);
        }
        const uint32_t sectionOffset = startAddr - shdr_table[index].second.sh_addr;
        const uint32_t sizeToData = sectionOffset - shdr_table[index].second.sh_size;
        shdr_table[index].second.sh_size += sizeToData + newData.size();
        shdr_table[index].second.data.resize(shdr_table[index].second.sh_size);
        shdr_table[index].second.data.replace(sectionOffset, newData.size(), newData);
        return ELFError::NONE;
    }

    ELFError ELF::writeToStream(std::ostream& out) {
        if (isEmpty == true) {
            LOG_ERR_AND_RETURN(ELFError::HEADER_DATA_NOT_LOADED);
        }
        ehdr.e_shnum = shdr_table.size();
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_type);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_machine);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_version);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_entry);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_phoff);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_shoff);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_flags);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_ehsize);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_phentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_phnum);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_shentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_shnum);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_shstrndx);

        out.write(reinterpret_cast<const char*>(&ehdr.e_ident), sizeof(ehdr.e_ident));
        out.write(reinterpret_cast<const char*>(&ehdr.e_type), sizeof(ehdr.e_type));
        out.write(reinterpret_cast<const char*>(&ehdr.e_machine), sizeof(ehdr.e_machine));
        out.write(reinterpret_cast<const char*>(&ehdr.e_version), sizeof(ehdr.e_version));
        out.write(reinterpret_cast<const char*>(&ehdr.e_entry), sizeof(ehdr.e_entry));
        out.write(reinterpret_cast<const char*>(&ehdr.e_phoff), sizeof(ehdr.e_phoff));
        out.write(reinterpret_cast<const char*>(&ehdr.e_shoff), sizeof(ehdr.e_shoff));
        out.write(reinterpret_cast<const char*>(&ehdr.e_flags), sizeof(ehdr.e_flags));
        out.write(reinterpret_cast<const char*>(&ehdr.e_ehsize), sizeof(ehdr.e_ehsize));
        out.write(reinterpret_cast<const char*>(&ehdr.e_phentsize), sizeof(ehdr.e_phentsize));
        out.write(reinterpret_cast<const char*>(&ehdr.e_phnum), sizeof(ehdr.e_phnum));
        out.write(reinterpret_cast<const char*>(&ehdr.e_shentsize), sizeof(ehdr.e_shentsize));
        out.write(reinterpret_cast<const char*>(&ehdr.e_shnum), sizeof(ehdr.e_shnum));
        out.write(reinterpret_cast<const char*>(&ehdr.e_shstrndx), sizeof(ehdr.e_shstrndx));

        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_type); // Swap ehdr data back so the data is correct for use later, byteswap inplace so things can be more easily converted to cross-platform
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_machine);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_version);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_entry);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_phoff);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_shoff);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_flags);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_ehsize);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_phentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_phnum);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_shentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_shnum);
        Utility::Endian::toPlatform_inplace(eType::Big, ehdr.e_shstrndx);

        std::ranges::sort(shdr_table, [](const shdr_index_t& a, const shdr_index_t& b) { return a.second.sh_offset < b.second.sh_offset; });

        Utility::seek(out, ehdr.e_shoff + ehdr.e_shentsize * ehdr.e_shnum, std::ios::beg);
        for (auto& [index, section] : shdr_table) {
            if (section.sh_type != SectionType::SHT_NOBITS && section.sh_type != SectionType::SHT_NULL) { // Ignore null or nobits sections since their offsets are 0 and have no data in the file
                padToLen(out, section.sh_addralign);

                section.sh_offset = out.tellp();
                section.sh_size = section.data.size();
                out.write(&section.data[0], section.data.size());
            }
        }
        // Sort again so they are written by index, to update offsets we needed to write the data first
        std::ranges::sort(shdr_table, [](const shdr_index_t& a, const shdr_index_t& b) { return a.first < b.first; });

        Utility::seek(out, ehdr.e_shoff, std::ios::beg);
        for (auto& [index, section] : shdr_table) {
            Utility::Endian::toPlatform_inplace(eType::Big, section.sh_name);
            Utility::Endian::toPlatform_inplace(eType::Big, section.sh_type);
            Utility::Endian::toPlatform_inplace(eType::Big, section.sh_flags);
            Utility::Endian::toPlatform_inplace(eType::Big, section.sh_addr);
            Utility::Endian::toPlatform_inplace(eType::Big, section.sh_offset);
            Utility::Endian::toPlatform_inplace(eType::Big, section.sh_size);
            Utility::Endian::toPlatform_inplace(eType::Big, section.sh_link);
            Utility::Endian::toPlatform_inplace(eType::Big, section.sh_info);
            Utility::Endian::toPlatform_inplace(eType::Big, section.sh_addralign);
            Utility::Endian::toPlatform_inplace(eType::Big, section.sh_entsize);

            out.write(reinterpret_cast<const char*>(&section.sh_name), sizeof(section.sh_name));
            out.write(reinterpret_cast<const char*>(&section.sh_type), sizeof(section.sh_type));
            out.write(reinterpret_cast<const char*>(&section.sh_flags), sizeof(section.sh_flags));
            out.write(reinterpret_cast<const char*>(&section.sh_addr), sizeof(section.sh_addr));
            out.write(reinterpret_cast<const char*>(&section.sh_offset), sizeof(section.sh_offset));
            out.write(reinterpret_cast<const char*>(&section.sh_size), sizeof(section.sh_size));
            out.write(reinterpret_cast<const char*>(&section.sh_link), sizeof(section.sh_link));
            out.write(reinterpret_cast<const char*>(&section.sh_info), sizeof(section.sh_info));
            out.write(reinterpret_cast<const char*>(&section.sh_addralign), sizeof(section.sh_addralign));
            out.write(reinterpret_cast<const char*>(&section.sh_entsize), sizeof(section.sh_entsize));
        }

        return ELFError::NONE;
    }

    ELFError ELF::writeToFile(const fspath& outFilePath) {
        std::ofstream outFile(outFilePath, std::ios::binary);
        if(!outFile.is_open()) {
            LOG_ERR_AND_RETURN(ELFError::COULD_NOT_OPEN);
        }
        return writeToStream(outFile);
    }
}
