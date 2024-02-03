#include "wiiurpx.hpp"

#include <cstring>

#include <algorithm>

#include <libs/zlib-ng.hpp>
#include <filetypes/shared/elf_structs.hpp>
#include <utility/endian.hpp>
#include <utility/common.hpp>
#include <utility/file.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;



namespace FileTypes {
    const char* RPXErrorGetName(RPXError err) {
        switch (err) {
            case RPXError::NONE:
                return "NONE";
            case RPXError::COULD_NOT_OPEN:
                return "COULD_NOT_OPEN";
            case RPXError::NOT_RPX:
                return "NOT_RPX";
            case RPXError::UNEXPECTED_VALUE:
                return "UNEXPECTED_VALUE";
            case RPXError::ZLIB_ERROR:
                return "ZLIB_ERROR";
            case RPXError::REACHED_EOF:
                return "REACHED_EOF";
            default:
                return "UNKNOWN";
        }
    }

    RPXError rpx_decompress(std::istream& in, std::ostream& out)
    {
        Elf32_Ehdr header;

        if(!in.read(reinterpret_cast<char*>(&header.e_ident), sizeof(header.e_ident))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(memcmp(header.e_ident, "\x7F\x45\x4C\x46", 4) != 0) {
            LOG_ERR_AND_RETURN(RPXError::NOT_RPX);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_type), sizeof(header.e_type))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_machine), sizeof(header.e_machine))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_version), sizeof(header.e_version))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_entry), sizeof(header.e_entry))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_phoff), sizeof(header.e_phoff))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_shoff), sizeof(header.e_shoff))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_flags), sizeof(header.e_flags))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_ehsize), sizeof(header.e_ehsize))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_phentsize), sizeof(header.e_phentsize))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_phnum), sizeof(header.e_phnum))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_shentsize), sizeof(header.e_shentsize))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_shnum), sizeof(header.e_shnum))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_shstrndx), sizeof(header.e_shstrndx))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, header.e_type);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_machine);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_version);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_entry);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_phoff);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shoff);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_flags);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_ehsize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_phentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_phnum);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shnum);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shstrndx);

        if(header.e_shentsize != 0x28) {
            LOG_ERR_AND_RETURN(RPXError::UNEXPECTED_VALUE);
        }
        in.seekg(header.e_shoff, std::ios::beg);

        std::vector<shdr_index_t> sectionHeaders(header.e_shnum);
        for(size_t i = 0; i < sectionHeaders.size(); i++) {
            sectionHeaders[i].first = i;
            Elf32_Shdr& shdr = sectionHeaders[i].second;

            if(!in.read(reinterpret_cast<char*>(&shdr.sh_name), sizeof(shdr.sh_name))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_type), sizeof(shdr.sh_type))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_flags), sizeof(shdr.sh_flags))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_addr), sizeof(shdr.sh_addr))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_offset), sizeof(shdr.sh_offset))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_size), sizeof(shdr.sh_size))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_link), sizeof(shdr.sh_link))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_info), sizeof(shdr.sh_info))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_addralign), sizeof(shdr.sh_addralign))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_entsize), sizeof(shdr.sh_entsize))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
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
        }
        
        // sort by offset in the file
        std::sort(sectionHeaders.begin(), sectionHeaders.end(), [](const shdr_index_t& a, const shdr_index_t& b) { return a.second.sh_offset < b.second.sh_offset; });
        Utility::seek(out, header.e_shoff + header.e_shentsize * header.e_shnum);

        for(auto& [index, section] : sectionHeaders) {
            if(section.sh_offset == 0) {
                continue;
            }

            in.seekg(section.sh_offset, std::ios::beg);

            section.sh_offset = out.tellp();

            if(section.sh_flags & static_cast<std::underlying_type_t<SectionFlags>>(SectionFlags::SHF_DEFLATED)) {
                // uncompressed size is stored at the start of each compressed section
                uint32_t uncompressedSize = 0;
                if(!in.read(reinterpret_cast<char*>(&uncompressedSize), sizeof(uncompressedSize))) {
                    LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
                }
                Utility::Endian::toPlatform_inplace(eType::Big, uncompressedSize);
                std::string outBuf(uncompressedSize, '\0');
                
                std::string sectionData(section.sh_size, '\0');
                if(!in.read(sectionData.data(), sectionData.size() - sizeof(uncompressedSize))) {
                    LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
                }
                
                size_t bufSize = uncompressedSize;
                if(zng_uncompress(reinterpret_cast<uint8_t*>(outBuf.data()), &bufSize, reinterpret_cast<const uint8_t*>(sectionData.data()), sectionData.size()) != Z_OK) {
                    LOG_ERR_AND_RETURN(RPXError::ZLIB_ERROR);
                }
                
                out.write(outBuf.data(), bufSize);
                section.sh_size = bufSize;
            }
            else {
                std::string sectionData(section.sh_size, '\0');
                if(!in.read(sectionData.data(), sectionData.size())) {
                    LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
                }

                out.write(sectionData.data(), sectionData.size());
            }
            
            padToLen(out, 0x40);
        }
        
        // Sort again so they are written by index, to update offsets we needed to write the data first
        std::sort(sectionHeaders.begin(), sectionHeaders.end(), [](const shdr_index_t& a, const shdr_index_t& b) { return a.first < b.first; });

        out.seekp(0, std::ios::beg);
        
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_type);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_machine);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_version);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_entry);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_phoff);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shoff);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_flags);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_ehsize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_phentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_phnum);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shnum);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shstrndx);

        out.write(reinterpret_cast<const char*>(&header.e_ident), sizeof(header.e_ident));
        out.write(reinterpret_cast<const char*>(&header.e_type), sizeof(header.e_type));
        out.write(reinterpret_cast<const char*>(&header.e_machine), sizeof(header.e_machine));
        out.write(reinterpret_cast<const char*>(&header.e_version), sizeof(header.e_version));
        out.write(reinterpret_cast<const char*>(&header.e_entry), sizeof(header.e_entry));
        out.write(reinterpret_cast<const char*>(&header.e_phoff), sizeof(header.e_phoff));
        out.write(reinterpret_cast<const char*>(&header.e_shoff), sizeof(header.e_shoff));
        out.write(reinterpret_cast<const char*>(&header.e_flags), sizeof(header.e_flags));
        out.write(reinterpret_cast<const char*>(&header.e_ehsize), sizeof(header.e_ehsize));
        out.write(reinterpret_cast<const char*>(&header.e_phentsize), sizeof(header.e_phentsize));
        out.write(reinterpret_cast<const char*>(&header.e_phnum), sizeof(header.e_phnum));
        out.write(reinterpret_cast<const char*>(&header.e_shentsize), sizeof(header.e_shentsize));
        out.write(reinterpret_cast<const char*>(&header.e_shnum), sizeof(header.e_shnum));
        out.write(reinterpret_cast<const char*>(&header.e_shstrndx), sizeof(header.e_shstrndx));

        padToLen(out, 0x40);

        for(auto& [index, shdr] : sectionHeaders) {
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

            out.write(reinterpret_cast<const char*>(&shdr.sh_name), sizeof(shdr.sh_name));
            out.write(reinterpret_cast<const char*>(&shdr.sh_type), sizeof(shdr.sh_type));
            out.write(reinterpret_cast<const char*>(&shdr.sh_flags), sizeof(shdr.sh_flags));
            out.write(reinterpret_cast<const char*>(&shdr.sh_addr), sizeof(shdr.sh_addr));
            out.write(reinterpret_cast<const char*>(&shdr.sh_offset), sizeof(shdr.sh_offset));
            out.write(reinterpret_cast<const char*>(&shdr.sh_size), sizeof(shdr.sh_size));
            out.write(reinterpret_cast<const char*>(&shdr.sh_link), sizeof(shdr.sh_link));
            out.write(reinterpret_cast<const char*>(&shdr.sh_info), sizeof(shdr.sh_info));
            out.write(reinterpret_cast<const char*>(&shdr.sh_addralign), sizeof(shdr.sh_addralign));
            out.write(reinterpret_cast<const char*>(&shdr.sh_entsize), sizeof(shdr.sh_entsize));
        }

        return RPXError::NONE;
    }

    RPXError rpx_compress(std::istream& in, std::ostream& out)
    {
        Elf32_Ehdr header;

        if(!in.read(reinterpret_cast<char*>(&header.e_ident), sizeof(header.e_ident))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(memcmp(header.e_ident, "\x7F\x45\x4C\x46", 4) != 0) {
            LOG_ERR_AND_RETURN(RPXError::NOT_RPX);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_type), sizeof(header.e_type))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_machine), sizeof(header.e_machine))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_version), sizeof(header.e_version))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_entry), sizeof(header.e_entry))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_phoff), sizeof(header.e_phoff))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_shoff), sizeof(header.e_shoff))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_flags), sizeof(header.e_flags))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_ehsize), sizeof(header.e_ehsize))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_phentsize), sizeof(header.e_phentsize))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_phnum), sizeof(header.e_phnum))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_shentsize), sizeof(header.e_shentsize))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_shnum), sizeof(header.e_shnum))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&header.e_shstrndx), sizeof(header.e_shstrndx))) {
            LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, header.e_type);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_machine);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_version);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_entry);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_phoff);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shoff);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_flags);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_ehsize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_phentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_phnum);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shnum);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shstrndx);

        if(header.e_shentsize != 0x28) {
            LOG_ERR_AND_RETURN(RPXError::UNEXPECTED_VALUE);
        }
        in.seekg(header.e_shoff, std::ios::beg);

        std::vector<shdr_index_t> sectionHeaders(header.e_shnum);
        for(size_t i = 0; i < sectionHeaders.size(); i++) {
            sectionHeaders[i].first = i;
            Elf32_Shdr& shdr = sectionHeaders[i].second;

            if(!in.read(reinterpret_cast<char*>(&shdr.sh_name), sizeof(shdr.sh_name))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_type), sizeof(shdr.sh_type))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_flags), sizeof(shdr.sh_flags))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_addr), sizeof(shdr.sh_addr))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_offset), sizeof(shdr.sh_offset))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_size), sizeof(shdr.sh_size))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_link), sizeof(shdr.sh_link))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_info), sizeof(shdr.sh_info))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_addralign), sizeof(shdr.sh_addralign))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            if(!in.read(reinterpret_cast<char*>(&shdr.sh_entsize), sizeof(shdr.sh_entsize))) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
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
        }
        
        // sort by offset in the file
        std::sort(sectionHeaders.begin(), sectionHeaders.end(), [](const shdr_index_t& a, const shdr_index_t& b) { return a.second.sh_offset < b.second.sh_offset; });
        Utility::seek(out, 0x40 + header.e_shentsize * header.e_shnum);

        std::vector<uint32_t> crcs(sectionHeaders.size(), 0);
        for(auto& [index, section] : sectionHeaders) {
            if(section.sh_offset == 0) {
                continue;
            }

            in.seekg(section.sh_offset, std::ios::beg);
            std::string sectionData(section.sh_size, '\0');
            if(!in.read(sectionData.data(), sectionData.size())) {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }

            if(section.sh_type != SectionType::SHT_RPL_CRCS) {
                crcs[index] = zng_crc32(0, reinterpret_cast<const uint8_t*>(sectionData.data()), sectionData.size());
            }
            section.sh_offset = out.tellp();

            if(section.sh_flags & static_cast<std::underlying_type_t<SectionFlags>>(SectionFlags::SHF_DEFLATED)) {
                uint32_t uncompressedSize = sectionData.size();

                std::string outBuf(zng_compressBound(uncompressedSize), '\0');

                size_t bufSize = outBuf.size(); // updated by zng_compress
                if(zng_compress(reinterpret_cast<uint8_t*>(outBuf.data()), &bufSize, reinterpret_cast<const uint8_t*>(sectionData.data()), sectionData.size()) != Z_OK) {
                    LOG_ERR_AND_RETURN(RPXError::ZLIB_ERROR);
                }
                
                // uncompressed size is stored at the start of each compressed section
                Utility::Endian::toPlatform_inplace(eType::Big, uncompressedSize);
                if(!out.write(reinterpret_cast<const char*>(&uncompressedSize), sizeof(uncompressedSize))) {
                    LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
                }

                out.write(outBuf.data(), bufSize);
                section.sh_size = bufSize + 4; // 32-bit int for uncompressed size is included in the section size
            }
            else {
                out.write(sectionData.data(), sectionData.size());
            }
            
            padToLen(out, 0x40);
        }

        // update crcs
        const auto it = std::find_if(sectionHeaders.begin(), sectionHeaders.end(), [](const shdr_index_t& hdr) { return hdr.second.sh_type == SectionType::SHT_RPL_CRCS; });
        if(it == sectionHeaders.end()) {
            return RPXError::NOT_RPX; // all RPX should have this section
        }
        if(it->second.sh_size != crcs.size() * sizeof(uint32_t)) {
            return RPXError::UNEXPECTED_VALUE;
        }
        out.seekp(it->second.sh_offset, std::ios::beg);
        for(uint32_t& crc : crcs) {
            Utility::Endian::toPlatform_inplace(eType::Big, crc);
            out.write(reinterpret_cast<const char*>(&crc), sizeof(crc));
        }
        
        // Sort again so they are written by index, to update offsets we needed to write the data first
        std::sort(sectionHeaders.begin(), sectionHeaders.end(), [](const shdr_index_t& a, const shdr_index_t& b) { return a.first < b.first; });

        out.seekp(0, std::ios::beg);
        
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_type);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_machine);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_version);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_entry);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_phoff);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shoff);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_flags);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_ehsize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_phentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_phnum);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shentsize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shnum);
        Utility::Endian::toPlatform_inplace(eType::Big, header.e_shstrndx);

        out.write(reinterpret_cast<const char*>(&header.e_ident), sizeof(header.e_ident));
        out.write(reinterpret_cast<const char*>(&header.e_type), sizeof(header.e_type));
        out.write(reinterpret_cast<const char*>(&header.e_machine), sizeof(header.e_machine));
        out.write(reinterpret_cast<const char*>(&header.e_version), sizeof(header.e_version));
        out.write(reinterpret_cast<const char*>(&header.e_entry), sizeof(header.e_entry));
        out.write(reinterpret_cast<const char*>(&header.e_phoff), sizeof(header.e_phoff));
        out.write(reinterpret_cast<const char*>(&header.e_shoff), sizeof(header.e_shoff));
        out.write(reinterpret_cast<const char*>(&header.e_flags), sizeof(header.e_flags));
        out.write(reinterpret_cast<const char*>(&header.e_ehsize), sizeof(header.e_ehsize));
        out.write(reinterpret_cast<const char*>(&header.e_phentsize), sizeof(header.e_phentsize));
        out.write(reinterpret_cast<const char*>(&header.e_phnum), sizeof(header.e_phnum));
        out.write(reinterpret_cast<const char*>(&header.e_shentsize), sizeof(header.e_shentsize));
        out.write(reinterpret_cast<const char*>(&header.e_shnum), sizeof(header.e_shnum));
        out.write(reinterpret_cast<const char*>(&header.e_shstrndx), sizeof(header.e_shstrndx));

        padToLen(out, 0x40);

        for(auto& [index, shdr] : sectionHeaders) {
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

            out.write(reinterpret_cast<const char*>(&shdr.sh_name), sizeof(shdr.sh_name));
            out.write(reinterpret_cast<const char*>(&shdr.sh_type), sizeof(shdr.sh_type));
            out.write(reinterpret_cast<const char*>(&shdr.sh_flags), sizeof(shdr.sh_flags));
            out.write(reinterpret_cast<const char*>(&shdr.sh_addr), sizeof(shdr.sh_addr));
            out.write(reinterpret_cast<const char*>(&shdr.sh_offset), sizeof(shdr.sh_offset));
            out.write(reinterpret_cast<const char*>(&shdr.sh_size), sizeof(shdr.sh_size));
            out.write(reinterpret_cast<const char*>(&shdr.sh_link), sizeof(shdr.sh_link));
            out.write(reinterpret_cast<const char*>(&shdr.sh_info), sizeof(shdr.sh_info));
            out.write(reinterpret_cast<const char*>(&shdr.sh_addralign), sizeof(shdr.sh_addralign));
            out.write(reinterpret_cast<const char*>(&shdr.sh_entsize), sizeof(shdr.sh_entsize));
        }

        return RPXError::NONE;
    }
}
