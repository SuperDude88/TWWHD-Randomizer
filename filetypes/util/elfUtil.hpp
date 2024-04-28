#pragma once

#include <cstdint>
#include <vector>

#include <filetypes/elf.hpp>

namespace elfUtil {
    bool containsAddress(const uint32_t& address, const uint32_t& memAddress, const uint32_t& sectionLen);

    offset_t AddressToOffset(const FileTypes::ELF& elf, const uint32_t& address);

    offset_t AddressToOffset(const FileTypes::ELF& elf, const uint32_t& address, const uint16_t& sectionIndex);

    ELFError addRelocation(FileTypes::ELF& elf, const uint16_t& shdrIdx, const Elf32_Rela& reloc);

    ELFError removeRelocation(FileTypes::ELF& elf, const offset_t& offset);

    ELFError write_u8(FileTypes::ELF& out, const offset_t& offset, const uint8_t& data);

    ELFError write_u16(FileTypes::ELF& out, const offset_t& offset, const uint16_t& data);

    ELFError write_u32(FileTypes::ELF& out, const offset_t& offset, const uint32_t& data);

    ELFError write_float(FileTypes::ELF& out, const offset_t& offset, const float& data);

    ELFError write_bytes(FileTypes::ELF& out, const offset_t& offset, const std::vector<uint8_t>& Bytes);

    uint8_t read_u8(const FileTypes::ELF& in, const offset_t& offset);

    uint16_t read_u16(const FileTypes::ELF& in, const offset_t& offset);

    uint32_t read_u32(const FileTypes::ELF& in, const offset_t& offset);

    float read_float(const FileTypes::ELF& in, const offset_t& offset);

    std::vector<uint8_t> read_bytes(const FileTypes::ELF& in, const offset_t& offset, const size_t& NumBytes);
}
