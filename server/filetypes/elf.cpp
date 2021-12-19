#include "elf.hpp"

namespace FileTypes {

	const char* ELFErrorGetName(ELFError err) {
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
			case ELFError::COUNT:
				return "COUNT";
			default:
				return "UNKNOWN";
		}
	}

	ELF::ELF() {
	
	}

	void ELF::initNew() {
		isEmpty = false;
		memcpy(ehdr.e_ident, "\x7F\x45\x4C\x46\x01\x02\x01\xCA\xFE\x00\x00\x00\x00\x00\x00\x00", 0x10);
		ehdr.e_type = 0xFE01;
		ehdr.e_machine = 0x0014;
		ehdr.e_version = 1;
		ehdr.e_entry = 0; //Not sure if this should be the default
		ehdr.e_phoff = 0; //Wii U doesn't have a program header
		ehdr.e_shoff = 0x40;
		ehdr.e_flags = 0;
		ehdr.e_ehsize = 0x34;
		ehdr.e_phentsize = 0;
		ehdr.e_phnum = 0;
		ehdr.e_shentsize = 0x28;
		ehdr.e_shnum = 0;
		ehdr.e_shstrndx = 0x1e;

		shdr_table = {};
		return;
	}

	ELF ELF::createNew(const std::string& filename) {
		ELF newELF{};
		newELF.initNew();
		return newELF;
	}

	ELFError ELF::loadFromBinary(std::istream& elf) {
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_ident), sizeof(ehdr.e_ident))) {
			return ELFError::REACHED_EOF;
		}
		if(memcmp(ehdr.e_ident, "\x7F\x45\x4C\x46", 4) != 0) {
			return ELFError::NOT_ELF;
		}
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_type), sizeof(ehdr.e_type))) {
			return ELFError::REACHED_EOF;
		}
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_machine), sizeof(ehdr.e_machine))) {
            return ELFError::REACHED_EOF;
        }
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_version), sizeof(ehdr.e_version))) {
            return ELFError::REACHED_EOF;
        }
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_entry), sizeof(ehdr.e_entry))) {
            return ELFError::REACHED_EOF;
        }
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_phoff), sizeof(ehdr.e_phoff))) {
            return ELFError::REACHED_EOF;
        }
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_shoff), sizeof(ehdr.e_shoff))) {
            return ELFError::REACHED_EOF;
        }
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_flags), sizeof(ehdr.e_flags))) {
            return ELFError::REACHED_EOF;
        }
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_ehsize), sizeof(ehdr.e_ehsize))) {
            return ELFError::REACHED_EOF;
        }
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_phentsize), sizeof(ehdr.e_phentsize))) {
            return ELFError::REACHED_EOF;
        }
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_phnum), sizeof(ehdr.e_phnum))) {
            return ELFError::REACHED_EOF;
        }
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_shentsize), sizeof(ehdr.e_shentsize))) {
            return ELFError::REACHED_EOF;
        }
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_shnum), sizeof(ehdr.e_shnum))) {
            return ELFError::REACHED_EOF;
        }
		if(!elf.read(reinterpret_cast<char*>(&ehdr.e_shstrndx), sizeof(ehdr.e_shstrndx))) {
            return ELFError::REACHED_EOF;
        }

		Utility::byteswap_inplace(ehdr.e_type);
		Utility::byteswap_inplace(ehdr.e_machine);
		Utility::byteswap_inplace(ehdr.e_version);
		Utility::byteswap_inplace(ehdr.e_entry);
		Utility::byteswap_inplace(ehdr.e_phoff);
		Utility::byteswap_inplace(ehdr.e_shoff);
		Utility::byteswap_inplace(ehdr.e_flags);
		Utility::byteswap_inplace(ehdr.e_ehsize);
		Utility::byteswap_inplace(ehdr.e_phentsize);
		Utility::byteswap_inplace(ehdr.e_phnum);
		Utility::byteswap_inplace(ehdr.e_shentsize);
		Utility::byteswap_inplace(ehdr.e_shnum);
		Utility::byteswap_inplace(ehdr.e_shstrndx);

		shdr_table.reserve(ehdr.e_shnum); //Allocate the memory from the start to minimize copies
		for (int i = 0; i < ehdr.e_shnum; i++) {
			Elf32_shdr shdr;
			elf.seekg(ehdr.e_shoff + ehdr.e_shentsize * i, std::ios::beg);
			if(!elf.read(reinterpret_cast<char*>(&shdr.sh_name), sizeof(shdr.sh_name))) {
				return ELFError::REACHED_EOF;
			}
			if(!elf.read(reinterpret_cast<char*>(&shdr.sh_type), sizeof(shdr.sh_type))) {
				return ELFError::REACHED_EOF;
			}
			if(!elf.read(reinterpret_cast<char*>(&shdr.sh_flags), sizeof(shdr.sh_flags))) {
				return ELFError::REACHED_EOF;
			}
			if(!elf.read(reinterpret_cast<char*>(&shdr.sh_addr), sizeof(shdr.sh_addr))) {
				return ELFError::REACHED_EOF;
			}
			if(!elf.read(reinterpret_cast<char*>(&shdr.sh_offset), sizeof(shdr.sh_offset))) {
				return ELFError::REACHED_EOF;
			}
			if(!elf.read(reinterpret_cast<char*>(&shdr.sh_size), sizeof(shdr.sh_size))) {
				return ELFError::REACHED_EOF;
			}
			if(!elf.read(reinterpret_cast<char*>(&shdr.sh_link), sizeof(shdr.sh_link))) {
				return ELFError::REACHED_EOF;
			}
			if(!elf.read(reinterpret_cast<char*>(&shdr.sh_info), sizeof(shdr.sh_info))) {
				return ELFError::REACHED_EOF;
			}
			if(!elf.read(reinterpret_cast<char*>(&shdr.sh_addralign), sizeof(shdr.sh_addralign))) {
				return ELFError::REACHED_EOF;
			}
			if(!elf.read(reinterpret_cast<char*>(&shdr.sh_entsize), sizeof(shdr.sh_entsize))) {
				return ELFError::REACHED_EOF;
			}

			Utility::byteswap_inplace(shdr.sh_name);
			Utility::byteswap_inplace(shdr.sh_type);
			Utility::byteswap_inplace(shdr.sh_flags);
			Utility::byteswap_inplace(shdr.sh_addr);
			Utility::byteswap_inplace(shdr.sh_offset);
			Utility::byteswap_inplace(shdr.sh_size);
			Utility::byteswap_inplace(shdr.sh_link);
			Utility::byteswap_inplace(shdr.sh_info);
			Utility::byteswap_inplace(shdr.sh_addralign);
			Utility::byteswap_inplace(shdr.sh_entsize);

			if (shdr.sh_type != 8) {
				elf.seekg(shdr.sh_offset, std::ios::beg);
				shdr.data.resize(shdr.sh_size);
				if(!elf.read(&shdr.data[0], shdr.sh_size)) {
					return ELFError::REACHED_EOF;
				}
				if(shdr.sh_type != 0 && shdr.sh_size == 0) { //If type isn't nobits or NULL and the size is 0, should not happen for WWHD
					return ELFError::USED_SECTION_IS_EMTPY;
				}
			}
			else {//Don't read data if type is 8 (SHT_NOBITS) since the data would be useless
				if(shdr.sh_offset != 0) {
					return ELFError::NOBITS_SECTION_NOT_EMPTY; //Offset in file should always be 0
				}
			}
			shdr_table.push_back({ i, shdr });
		}

		isEmpty = false;

		return ELFError::NONE;
	}

	ELFError ELF::loadFromFile(const std::string& filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if(!file.is_open()) {
			return ELFError::COULD_NOT_OPEN;
		}
		return loadFromBinary(file);
	}

	ELFError ELF::extend_section(int index, uint32_t newSize, std::string newData) { //newData is data to append, not replace
		if(isEmpty == true) {
			return ELFError::HEADER_DATA_NOT_LOADED;
		}
		std::sort(shdr_table.begin(), shdr_table.end(), [](const std::pair<int, Elf32_shdr>& a, const std::pair<int, Elf32_shdr>& b) { //Make sure the items are sorted by index so we get the correct section
			return a.first < b.first;
			});
		if(shdr_table[index].second.data.size() == 0) { //Wouldn't append data to a section that didn't already have some
			return ELFError::SECTION_DATA_NOT_LOADED;
		}
		shdr_table[index].second.sh_size = newSize;
		shdr_table[index].second.data.append(newData);
		return ELFError::NONE;
	}

	ELFError ELF::extend_section(int index, std::string newData) { //newData is data to append, not replace
		if (isEmpty == true) {
			return ELFError::HEADER_DATA_NOT_LOADED;
		}
		std::sort(shdr_table.begin(), shdr_table.end(), [](const std::pair<int, Elf32_shdr>& a, const std::pair<int, Elf32_shdr>& b) { //Make sure the items are sorted by index so we get the correct section
			return a.first < b.first;
			});
		if (shdr_table[index].second.data.size() == 0) { //Wouldn't append data to a section that didn't already have some
			return ELFError::SECTION_DATA_NOT_LOADED;
		}
		shdr_table[index].second.sh_size += newData.size();
		shdr_table[index].second.data.append(newData);
		return ELFError::NONE;
	}

	ELFError ELF::writeToStream(std::ostream& out) {
		if (isEmpty == true) {
			return ELFError::HEADER_DATA_NOT_LOADED;
		}
		ehdr.e_shnum = shdr_table.size();
		Utility::byteswap_inplace(ehdr.e_type);
		Utility::byteswap_inplace(ehdr.e_machine);
		Utility::byteswap_inplace(ehdr.e_version);
		Utility::byteswap_inplace(ehdr.e_entry);
		Utility::byteswap_inplace(ehdr.e_phoff);
		Utility::byteswap_inplace(ehdr.e_shoff);
		Utility::byteswap_inplace(ehdr.e_flags);
		Utility::byteswap_inplace(ehdr.e_ehsize);
		Utility::byteswap_inplace(ehdr.e_phentsize);
		Utility::byteswap_inplace(ehdr.e_phnum);
		Utility::byteswap_inplace(ehdr.e_shentsize);
		Utility::byteswap_inplace(ehdr.e_shnum);
		Utility::byteswap_inplace(ehdr.e_shstrndx);

		out.write(reinterpret_cast<char*>(&ehdr.e_ident), sizeof(ehdr.e_ident));
		out.write(reinterpret_cast<char*>(&ehdr.e_type), sizeof(ehdr.e_type));
		out.write(reinterpret_cast<char*>(&ehdr.e_machine), sizeof(ehdr.e_machine));
		out.write(reinterpret_cast<char*>(&ehdr.e_version), sizeof(ehdr.e_version));
		out.write(reinterpret_cast<char*>(&ehdr.e_entry), sizeof(ehdr.e_entry));
		out.write(reinterpret_cast<char*>(&ehdr.e_phoff), sizeof(ehdr.e_phoff));
		out.write(reinterpret_cast<char*>(&ehdr.e_shoff), sizeof(ehdr.e_shoff));
		out.write(reinterpret_cast<char*>(&ehdr.e_flags), sizeof(ehdr.e_flags));
		out.write(reinterpret_cast<char*>(&ehdr.e_ehsize), sizeof(ehdr.e_ehsize));
		out.write(reinterpret_cast<char*>(&ehdr.e_phentsize), sizeof(ehdr.e_phentsize));
		out.write(reinterpret_cast<char*>(&ehdr.e_phnum), sizeof(ehdr.e_phnum));
		out.write(reinterpret_cast<char*>(&ehdr.e_shentsize), sizeof(ehdr.e_shentsize));
		out.write(reinterpret_cast<char*>(&ehdr.e_shnum), sizeof(ehdr.e_shnum));
		out.write(reinterpret_cast<char*>(&ehdr.e_shstrndx), sizeof(ehdr.e_shstrndx));

		Utility::byteswap_inplace(ehdr.e_type); //Swap ehdr data back so the data is correct for use later, byteswap inplace so things can be more easily converted to cross-platform
		Utility::byteswap_inplace(ehdr.e_machine);
		Utility::byteswap_inplace(ehdr.e_version);
		Utility::byteswap_inplace(ehdr.e_entry);
		Utility::byteswap_inplace(ehdr.e_phoff);
		Utility::byteswap_inplace(ehdr.e_shoff);
		Utility::byteswap_inplace(ehdr.e_flags);
		Utility::byteswap_inplace(ehdr.e_ehsize);
		Utility::byteswap_inplace(ehdr.e_phentsize);
		Utility::byteswap_inplace(ehdr.e_phnum);
		Utility::byteswap_inplace(ehdr.e_shentsize);
		Utility::byteswap_inplace(ehdr.e_shnum);
		Utility::byteswap_inplace(ehdr.e_shstrndx);

		std::sort(shdr_table.begin(), shdr_table.end(), [](const std::pair<int, Elf32_shdr>& a, const std::pair<int, Elf32_shdr>& b) {
			return a.second.sh_offset < b.second.sh_offset;
			});
		int nextOffset = ehdr.e_shoff + ehdr.e_shentsize * ehdr.e_shnum;
		for (int i = 0; i < ehdr.e_shnum; i++) {
			if (shdr_table[i].second.sh_type != 8 && shdr_table[i].second.sh_type != 0) { //Ignore null or nobits sections since their offsets are 0 and have no data in the file
				out.seekp(nextOffset, std::ios::beg);
				shdr_table[i].second.sh_offset = nextOffset;
				shdr_table[i].second.sh_size = shdr_table[i].second.data.size();
				if (shdr_table[i].second.sh_size != 0) { //Check if the size is non-zero (we check if section is SHT_NOBITS earlier)
					out.write(&shdr_table[i].second.data[0], shdr_table[i].second.data.size());
				}
				int padding_size = 0;
				if (i != ehdr.e_shnum - 1) { //No padding on the last entry, skip it
					if (out.tellp() % shdr_table[i + 1].second.sh_addralign != 0) {
						padding_size = shdr_table[i + 1].second.sh_addralign - (out.tellp() % shdr_table[i + 1].second.sh_addralign);
						std::string padding;
						padding.resize(padding_size);
						out.write(&padding[0], padding_size);
					}
				}
				nextOffset = nextOffset + shdr_table[i].second.sh_size + padding_size;
			}
		}
		std::sort(shdr_table.begin(), shdr_table.end(), [](const std::pair<int, Elf32_shdr>& a, const std::pair<int, Elf32_shdr>& b) { //Sort again so they are written by index, to update offsets we needed to write the data first
			return a.first < b.first;
			});

		out.seekp(ehdr.e_shoff, std::ios::beg);
		for (int i = 0; i < ehdr.e_shnum; i++) {
			Utility::byteswap_inplace(shdr_table[i].second.sh_name);
			Utility::byteswap_inplace(shdr_table[i].second.sh_type);
			Utility::byteswap_inplace(shdr_table[i].second.sh_flags);
			Utility::byteswap_inplace(shdr_table[i].second.sh_addr);
			Utility::byteswap_inplace(shdr_table[i].second.sh_offset);
			Utility::byteswap_inplace(shdr_table[i].second.sh_size);
			Utility::byteswap_inplace(shdr_table[i].second.sh_link);
			Utility::byteswap_inplace(shdr_table[i].second.sh_info);
			Utility::byteswap_inplace(shdr_table[i].second.sh_addralign);
			Utility::byteswap_inplace(shdr_table[i].second.sh_entsize);

			out.write(reinterpret_cast<char*>(&shdr_table[i].second.sh_name), sizeof(shdr_table[i].second.sh_name));
			out.write(reinterpret_cast<char*>(&shdr_table[i].second.sh_type), sizeof(shdr_table[i].second.sh_type));
			out.write(reinterpret_cast<char*>(&shdr_table[i].second.sh_flags), sizeof(shdr_table[i].second.sh_flags));
			out.write(reinterpret_cast<char*>(&shdr_table[i].second.sh_addr), sizeof(shdr_table[i].second.sh_addr));
			out.write(reinterpret_cast<char*>(&shdr_table[i].second.sh_offset), sizeof(shdr_table[i].second.sh_offset));
			out.write(reinterpret_cast<char*>(&shdr_table[i].second.sh_size), sizeof(shdr_table[i].second.sh_size));
			out.write(reinterpret_cast<char*>(&shdr_table[i].second.sh_link), sizeof(shdr_table[i].second.sh_link));
			out.write(reinterpret_cast<char*>(&shdr_table[i].second.sh_info), sizeof(shdr_table[i].second.sh_info));
			out.write(reinterpret_cast<char*>(&shdr_table[i].second.sh_addralign), sizeof(shdr_table[i].second.sh_addralign));
			out.write(reinterpret_cast<char*>(&shdr_table[i].second.sh_entsize), sizeof(shdr_table[i].second.sh_entsize));
		}

		return ELFError::NONE;
	}

	ELFError ELF::writeToFile(const std::string& outFilePath) {
		std::ofstream outFile(outFilePath, std::ios::binary | std::ios::in | std::ios::out);
		if(!outFile.is_open()) {
			return ELFError::COULD_NOT_OPEN;
		}
		return writeToStream(outFile);
	}
}
