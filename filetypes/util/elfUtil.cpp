#include "elfUtil.hpp"

#include <utility/endian.hpp>
#include <command/Log.hpp>

#define CHECK_OFFSET_RANGES(elf, offset) {    \
    if(offset.shdrIdx > elf.shdr_table.size() - 1) LOG_ERR_AND_RETURN(ELFError::INDEX_OUT_OF_RANGE);    \
    if(offset.offset > elf.shdr_table[offset.shdrIdx].second.data.size() - 1) LOG_ERR_AND_RETURN(ELFError::INDEX_OUT_OF_RANGE);    \
}

using eType = Utility::Endian::Type;

namespace elfUtil {
    bool containsAddress(const uint32_t& address, const uint32_t& memAddress, const uint32_t& sectionLen) {
        if (memAddress <= address && address < memAddress + sectionLen) {
            return true;
        }
        return false;
    }

    offset_t AddressToOffset(const FileTypes::ELF& elf, const uint32_t& address) { // calculates offset into section, returns first value as section index and second as offset
        for (const uint16_t index : {2, 3, 4}) { // only check a few sections that might be written to
            if (containsAddress(address, elf.shdr_table[index].second.sh_addr, elf.shdr_table[index].second.sh_size)) {
                return {index, address - elf.shdr_table[index].second.sh_addr};
            }
        }
        return {0, 0};
    }

    offset_t AddressToOffset(const FileTypes::ELF& elf, const uint32_t& address, const uint16_t& sectionIndex) {
        if (!containsAddress(address, elf.shdr_table[sectionIndex].second.sh_addr, elf.shdr_table[sectionIndex].second.sh_size)) {
            return { 0, 0 };
        }
        return { sectionIndex, address - elf.shdr_table[sectionIndex].second.sh_addr };
    }

    ELFError addRelocation(FileTypes::ELF& elf, const uint16_t& shdrIdx, const Elf32_Rela& reloc) {
        uint32_t offset_BE = Utility::Endian::toPlatform(eType::Big, reloc.r_offset);
        uint32_t info_BE = Utility::Endian::toPlatform(eType::Big, reloc.r_info);
        int32_t addend_BE = Utility::Endian::toPlatform(eType::Big, reloc.r_addend);

        std::string entry(12, '\0');
        entry.replace(0, 4, reinterpret_cast<const char*>(&offset_BE), 4);
        entry.replace(4, 4, reinterpret_cast<const char*>(&info_BE), 4);
        entry.replace(8, 4, reinterpret_cast<const char*>(&addend_BE), 4);
        return elf.extend_section(shdrIdx, entry);
    }

    ELFError removeRelocation(FileTypes::ELF& elf, const offset_t& offset) {
        CHECK_OFFSET_RANGES(elf, offset);
        elf.shdr_table[offset.shdrIdx].second.data.replace(offset.offset, 0xC, 0xC, '\0');

        return ELFError::NONE;
    }

    ELFError write_u8(FileTypes::ELF& out, const offset_t& offset, const uint8_t& data) {
        CHECK_OFFSET_RANGES(out, offset);
        out.shdr_table[offset.shdrIdx].second.data[offset.offset] = reinterpret_cast<const char&>(data);

        return ELFError::NONE;
    }

    ELFError write_u16(FileTypes::ELF& out, const offset_t& offset, const uint16_t& data) {
        const uint16_t toWrite = Utility::Endian::toPlatform(eType::Big, data);
        
        CHECK_OFFSET_RANGES(out, offset);
        out.shdr_table[offset.shdrIdx].second.data.replace(offset.offset, 2, reinterpret_cast<const char*>(&toWrite), 2);

        return ELFError::NONE;
    }

    ELFError write_u32(FileTypes::ELF& out, const offset_t& offset, const uint32_t& data) {
        const uint32_t toWrite = Utility::Endian::toPlatform(eType::Big, data);
        
        CHECK_OFFSET_RANGES(out, offset);
        out.shdr_table[offset.shdrIdx].second.data.replace(offset.offset, 4, reinterpret_cast<const char*>(&toWrite), 4);
        
        return ELFError::NONE;
    }

    ELFError write_float(FileTypes::ELF& out, const offset_t& offset, const float& data) {
        const uint32_t toWrite = Utility::Endian::toPlatform(eType::Big, std::bit_cast<const uint32_t>(data));
        
        CHECK_OFFSET_RANGES(out, offset);
        out.shdr_table[offset.shdrIdx].second.data.replace(offset.offset, 4, reinterpret_cast<const char*>(&toWrite), 4);
        
        return ELFError::NONE;
    }

    ELFError write_bytes(FileTypes::ELF& out, const offset_t& offset, const std::vector<uint8_t>& bytes) {
        CHECK_OFFSET_RANGES(out, offset);
        for (unsigned int i = 0; i < bytes.size(); i++) {
            out.shdr_table[offset.shdrIdx].second.data[offset.offset + i] = reinterpret_cast<const char&>(bytes[i]);
        }
        
        return ELFError::NONE;
    }

    uint8_t read_u8(const FileTypes::ELF& in, const offset_t& offset) {
        return *reinterpret_cast<const uint8_t*>(&in.shdr_table[offset.shdrIdx].second.data[offset.offset]);
    }

    uint16_t read_u16(const FileTypes::ELF& in, const offset_t& offset) {
        return *reinterpret_cast<const uint16_t*>(&in.shdr_table[offset.shdrIdx].second.data[offset.offset]);
    }

    uint32_t read_u32(const FileTypes::ELF& in, const offset_t& offset) {
        return *reinterpret_cast<const uint32_t*>(&in.shdr_table[offset.shdrIdx].second.data[offset.offset]);
    }

    float read_float(const FileTypes::ELF& in, const offset_t& offset) {
        return *reinterpret_cast<const float*>(&in.shdr_table[offset.shdrIdx].second.data[offset.offset]);
    }

    std::vector<uint8_t> read_bytes(const FileTypes::ELF& in, const offset_t& offset, const size_t& numBytes) {
        std::vector<uint8_t> bytes;

        bytes.reserve(numBytes); // avoid reallocations
        for (size_t i = 0; i < numBytes; i++) {
            bytes.emplace_back(static_cast<uint8_t>(in.shdr_table[offset.shdrIdx].second.data[offset.offset + i]));
        }

        return bytes;
    }
}
