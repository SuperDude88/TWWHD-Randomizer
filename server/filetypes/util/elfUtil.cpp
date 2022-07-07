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

	offset_t AddressToOffset(const FileTypes::ELF& elf, const uint32_t& address) { //calculates offset into section, returns first value as section index and second as offset
	for (uint16_t index : {2, 3, 4}) { //only check a few sections that might be written to
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
		std::string entry(12, '\0');
		entry.replace(0, 4, reinterpret_cast<const char*>(&reloc.r_offset), 4);
		entry.replace(4, 4, reinterpret_cast<const char*>(&reloc.r_info), 4);
		entry.replace(8, 4, reinterpret_cast<const char*>(&reloc.r_addend), 4);
		return elf.extend_section(shdrIdx, entry);
	}

    void removeRelocation(FileTypes::ELF& elf, const offset_t& offset) {
		elf.shdr_table[offset.shdrIdx].second.data.replace(offset.offset, 0xC, 0xC, '\0');
	}

	void write_u8(FileTypes::ELF& out, const offset_t& offset, const uint8_t& data) {
		out.shdr_table[offset.shdrIdx].second.data[offset.offset] = reinterpret_cast<const char&>(data);
		return;
	}

	void write_u16(FileTypes::ELF& out, const offset_t& offset, const uint16_t& data) {
		uint16_t toWrite = Utility::Endian::toPlatform(eType::Big, data);
		out.shdr_table[offset.shdrIdx].second.data.replace(offset.offset, 2, reinterpret_cast<const char*>(&toWrite), 2);
		return;
	}

	void write_u32(FileTypes::ELF& out, const offset_t& offset, const uint32_t& data) {
		uint32_t toWrite = Utility::Endian::toPlatform(eType::Big, data);
		out.shdr_table[offset.shdrIdx].second.data.replace(offset.offset, 4, reinterpret_cast<const char*>(&toWrite), 4);
		return;
	}

	void write_float(FileTypes::ELF& out, const offset_t& offset, const float& data) {
		float toWrite = Utility::Endian::toPlatform(eType::Big, data);
		out.shdr_table[offset.shdrIdx].second.data.replace(offset.offset, 4, reinterpret_cast<const char*>(&toWrite), 4);
		return;
	}

	void write_bytes(FileTypes::ELF& out, const offset_t& offset, const std::vector<uint8_t>& bytes) {
		for (unsigned int i = 0; i < bytes.size(); i++) {
			out.shdr_table[offset.shdrIdx].second.data[offset.offset + i] = reinterpret_cast<const char&>(bytes[i]);
		}
		return;
	}

	uint8_t read_u8(const FileTypes::ELF& in, const offset_t& offset) {
		return *(uint8_t*)&in.shdr_table[offset.shdrIdx].second.data[offset.offset];
	}

	uint16_t read_u16(const FileTypes::ELF& in, const offset_t& offset) {
		return *(uint16_t*)&in.shdr_table[offset.shdrIdx].second.data[offset.offset];
	}

	uint32_t read_u32(const FileTypes::ELF& in, const offset_t& offset) {
		return *(uint32_t*)&in.shdr_table[offset.shdrIdx].second.data[offset.offset];
	}

	float read_float(const FileTypes::ELF& in, const offset_t& offset) {
		return *(float*)&in.shdr_table[offset.shdrIdx].second.data[offset.offset];
	}

	std::vector<uint8_t> read_bytes(const FileTypes::ELF& in, const offset_t& offset, const size_t& numBytes) {
		uint8_t buffer = 0x0;
		std::vector<uint8_t> bytes;

		bytes.reserve(numBytes); //avoid reallocations
		for (size_t i = 0; i < numBytes; i++) {
			buffer = reinterpret_cast<const uint8_t&>(in.shdr_table[offset.shdrIdx].second.data[offset.offset + i]);
			bytes.push_back(buffer);
		}

		return bytes;
	}
}
