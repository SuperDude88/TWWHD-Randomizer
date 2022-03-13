#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "../elf.hpp"

namespace elfUtil {
    bool containsAddress(const uint32_t& address, const uint32_t& memAddress, const uint32_t& sectionLen);

    std::pair<uint32_t, uint32_t> AddressToOffset(const FileTypes::ELF& elf, const uint32_t& address);

    std::pair<uint32_t, uint32_t> AddressToOffset(const FileTypes::ELF& elf, const uint32_t& address, const unsigned int& sectionIndex);

    void write_u8(FileTypes::ELF& out, const std::pair<int, int>& offset, const uint8_t& data);

    void write_u16(FileTypes::ELF& out, const std::pair<int, int>& offset, const uint16_t& data);

    void write_u32(FileTypes::ELF& out, const std::pair<int, int>& offset, const uint32_t& data);

    void write_float(FileTypes::ELF& out, const std::pair<int, int>& offset, const float& data);

    void write_bytes(FileTypes::ELF& out, const std::pair<int, int>& offset, const std::vector<uint8_t>& Bytes);

    uint8_t read_u8(const FileTypes::ELF& in, const std::pair<int, int>& offset);

	uint16_t read_u16(const FileTypes::ELF& in, const std::pair<int, int>& offset);

    uint32_t read_u32(const FileTypes::ELF& in, const std::pair<int, int>& offset);

    float read_float(const FileTypes::ELF& in, const std::pair<int, int>& offset);

    std::vector<uint8_t> read_bytes(const FileTypes::ELF& in, const std::pair<int, int>& offset, const unsigned int& NumBytes);
}
