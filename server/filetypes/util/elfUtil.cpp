#include "elfUtil.hpp"
#include "../../utility/endian.hpp"

using eType = Utility::Endian::Type;

namespace elfUtil {
	bool containsAddress(const uint32_t& address, const uint32_t& memAddress, const uint32_t& sectionLen) {
		if (memAddress <= address && address < memAddress + sectionLen) {
			return true;
		}
		else {
			return false;
		}
	}

	std::pair<uint32_t, uint32_t> AddressToOffset(const FileTypes::ELF& elf, const uint32_t& address) { //calculates offset into section, returns first value as section index and second as offset
	for (unsigned int index : {2, 3, 4}) { //only check a few sections that might be written to
		if (containsAddress(address, elf.shdr_table[index].second.sh_addr, elf.shdr_table[index].second.sh_size)) {
			return {index, address - elf.shdr_table[index].second.sh_addr};
		}
	}
	return {0, 0};
	}

	std::pair<uint32_t, uint32_t> AddressToOffset(const FileTypes::ELF& elf, const uint32_t& address, const unsigned int& sectionIndex) {
		if (!containsAddress(address, elf.shdr_table[sectionIndex].second.sh_addr, elf.shdr_table[sectionIndex].second.sh_size)) {
			return { 0, 0 };
		}
		return { sectionIndex, address - elf.shdr_table[sectionIndex].second.sh_addr };
	}

	void write_u8(FileTypes::ELF& out, const std::pair<int, int>& offset, const uint8_t& data) {
		out.shdr_table[offset.first].second.data[offset.second] = reinterpret_cast<const char&>(data);
		return;
	}

	void write_u16(FileTypes::ELF& out, const std::pair<int, int>& offset, const uint16_t& data) {
		uint16_t toWrite = Utility::Endian::toPlatform(eType::Big, data);
		out.shdr_table[offset.first].second.data.replace(offset.second, 2, reinterpret_cast<const char*>(&toWrite), 2);
		return;
	}

	void write_u32(FileTypes::ELF& out, const std::pair<int, int>& offset, const uint32_t& data) {
		uint32_t toWrite = Utility::Endian::toPlatform(eType::Big, data);
		out.shdr_table[offset.first].second.data.replace(offset.second, 4, reinterpret_cast<const char*>(&toWrite), 4);
		return;
	}

	void write_float(FileTypes::ELF& out, const std::pair<int, int>& offset, const float& data) {
		float toWrite = Utility::Endian::toPlatform(eType::Big, data);
		out.shdr_table[offset.first].second.data.replace(offset.second, 4, reinterpret_cast<const char*>(&toWrite), 4);
		return;
	}

	void write_bytes(FileTypes::ELF& out, const std::pair<int, int>& offset, const std::vector<uint8_t>& bytes) {
		for (unsigned int i = 0; i < bytes.size(); i++) {
			out.shdr_table[offset.first].second.data[offset.second + i] = reinterpret_cast<const char&>(bytes[i]);
		}
		return;
	}

	uint8_t read_u8(const FileTypes::ELF& in, const std::pair<int, int>& offset) {
		return *(uint8_t*)&in.shdr_table[offset.first].second.data[offset.second];
	}

	uint16_t read_u16(const FileTypes::ELF& in, const std::pair<int, int>& offset) {
		return *(uint16_t*)&in.shdr_table[offset.first].second.data[offset.second];
	}

	uint32_t read_u32(const FileTypes::ELF& in, const std::pair<int, int>& offset) {
		return *(uint32_t*)&in.shdr_table[offset.first].second.data[offset.second];
	}

	float read_float(const FileTypes::ELF& in, const std::pair<int, int>& offset) {
		return *(float*)&in.shdr_table[offset.first].second.data[offset.second];
	}

	std::vector<uint8_t> read_bytes(const FileTypes::ELF& in, const std::pair<int, int>& offset, const unsigned int& numBytes) {
		uint8_t buffer = 0x0;
		std::vector<uint8_t> bytes;

		bytes.reserve(numBytes); //avoid reallocations
		for (unsigned int i = 0; i < numBytes; i++) {
			buffer = reinterpret_cast<const uint8_t&>(in.shdr_table[offset.first].second.data[offset.second + i]);
			bytes.push_back(buffer);
		}

		return bytes;
	}
}
