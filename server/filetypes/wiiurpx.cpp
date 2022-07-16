// Copyright (C) 2016  CBH <maodatouint8_t8@163.com>
// Licensed under the terms of the GNU GPL, version 3
// http://www.gnu.org/licenses/gpl-3.0.txt

//Largely pulled from https://github.com/0CBH0/wiiurpxtool
//Several things are changed to be consistent with other filetypes

#include "wiiurpx.hpp"

#include <zlib.h>
#include <vector>
#include <array>
#include <fstream>
#include <algorithm>
#include <cstring>

#include "../utility/endian.hpp"
#include "../utility/file.hpp"
#include "../command/Log.hpp"

using eType = Utility::Endian::Type;



namespace {
    uint32_t crc32_rpx(uint32_t crc, uint8_t* buff, const uint32_t len)
    {
        std::array<uint32_t, 256> crc_table;
        for (uint32_t i = 0; i < 256; i++)
        {
            uint32_t c = i;
            for (uint32_t j = 0; j < 8; j++)
            {
                if (c & 1)
                    c = 0xedb88320L ^ (c >> 1);
                else
                    c = c >> 1;
            }
            crc_table[i] = c;
        }
        crc = ~crc;
        for (uint32_t i = 0; i < len; i++)
            crc = (crc >> 8) ^ crc_table[(crc ^ buff[i]) & 0xff];
        return ~crc;
    }
}



namespace FileTypes {
    const char* RPXErrorGetName(RPXError err) {
        switch (err) {
            case RPXError::NONE:
                return "NONE";
            case RPXError::COULD_NOT_OPEN:
                return "COULD_NOT_OPEN";
            case RPXError::NOT_RPX:
                return "NOT_RPX";
            case RPXError::UNKNOWN_E_TYPE:
                return "UNKNOWN_E_TYPE";
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
        Elf32_Ehdr ehdr;
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_ident[0]), 0x10)) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (std::strncmp(reinterpret_cast<char*>(&ehdr.e_ident[0]), "\x7F""ELF", 4)) LOG_ERR_AND_RETURN(RPXError::NOT_RPX);

        if (!in.read(reinterpret_cast<char*>(&ehdr.e_type), sizeof(ehdr.e_type))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_machine), sizeof(ehdr.e_machine))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_version), sizeof(ehdr.e_version))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_entry), sizeof(ehdr.e_entry))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_phoff), sizeof(ehdr.e_phoff))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_shoff), sizeof(ehdr.e_shoff))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_flags), sizeof(ehdr.e_flags))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_ehsize), sizeof(ehdr.e_ehsize))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_phentsize), sizeof(ehdr.e_phentsize))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_phnum), sizeof(ehdr.e_phnum))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_shentsize), sizeof(ehdr.e_shentsize))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_shnum), sizeof(ehdr.e_shnum))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_shstrndx), sizeof(ehdr.e_shstrndx))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);

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
        
        if (ehdr.e_type != 0xFE01) LOG_ERR_AND_RETURN(RPXError::UNKNOWN_E_TYPE);

        uint32_t shdr_data_elf_offset = ehdr.e_shoff + ehdr.e_shnum * ehdr.e_shentsize;
        out.write(reinterpret_cast<const char*>(&ehdr.e_ident[0]), 0x10);

        auto e_type = Utility::Endian::toPlatform(eType::Big, ehdr.e_type);
        auto e_machine = Utility::Endian::toPlatform(eType::Big, ehdr.e_machine);
        auto e_version = Utility::Endian::toPlatform(eType::Big, ehdr.e_version);
        auto e_entry = Utility::Endian::toPlatform(eType::Big, ehdr.e_entry);
        auto e_phoff = Utility::Endian::toPlatform(eType::Big, ehdr.e_phoff);
        auto e_shoff = Utility::Endian::toPlatform(eType::Big, ehdr.e_shoff);
        auto e_flags = Utility::Endian::toPlatform(eType::Big, ehdr.e_flags);
        auto e_ehsize = Utility::Endian::toPlatform(eType::Big, ehdr.e_ehsize);
        auto e_phentsize = Utility::Endian::toPlatform(eType::Big, ehdr.e_phentsize);
        auto e_phnum = Utility::Endian::toPlatform(eType::Big, ehdr.e_phnum);
        auto e_shentsize = Utility::Endian::toPlatform(eType::Big, ehdr.e_shentsize);
        auto e_shnum = Utility::Endian::toPlatform(eType::Big, ehdr.e_shnum);
        auto e_shstrndx = Utility::Endian::toPlatform(eType::Big, ehdr.e_shstrndx);

        out.write(reinterpret_cast<const char*>(&e_type), sizeof(e_type));
        out.write(reinterpret_cast<const char*>(&e_machine), sizeof(e_machine));
        out.write(reinterpret_cast<const char*>(&e_version), sizeof(e_version));
        out.write(reinterpret_cast<const char*>(&e_entry), sizeof(e_entry));
        out.write(reinterpret_cast<const char*>(&e_phoff), sizeof(e_phoff));
        out.write(reinterpret_cast<const char*>(&e_shoff), sizeof(e_shoff));
        out.write(reinterpret_cast<const char*>(&e_flags), sizeof(e_flags));
        out.write(reinterpret_cast<const char*>(&e_ehsize), sizeof(e_ehsize));
        out.write(reinterpret_cast<const char*>(&e_phentsize), sizeof(e_phentsize));
        out.write(reinterpret_cast<const char*>(&e_phnum), sizeof(e_phnum));
        out.write(reinterpret_cast<const char*>(&e_shentsize), sizeof(e_shentsize));
        out.write(reinterpret_cast<const char*>(&e_shnum), sizeof(e_shnum));
        out.write(reinterpret_cast<const char*>(&e_shstrndx), sizeof(e_shstrndx));
        uint32_t crc_data_offset = 0;
        std::vector<uint32_t> crcs(ehdr.e_shnum, 0);
        std::vector<shdr_index_t> shdr_table(ehdr.e_shnum);
        while (static_cast<uint32_t>(out.tellp()) < shdr_data_elf_offset) out.put(0);
        if (!in.seekg(ehdr.e_shoff, std::ios::beg)) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        for (uint16_t i = 0; i < ehdr.e_shnum; i++)
        {
            shdr_table[i].first = i;
            Elf32_Shdr& shdr_entry = shdr_table[i].second;
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_name), sizeof(shdr_entry.sh_name))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_type), sizeof(shdr_entry.sh_type))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_flags), sizeof(shdr_entry.sh_flags))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_addr), sizeof(shdr_entry.sh_addr))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_offset), sizeof(shdr_entry.sh_offset))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_size), sizeof(shdr_entry.sh_size))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_link), sizeof(shdr_entry.sh_link))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_info), sizeof(shdr_entry.sh_info))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_addralign), sizeof(shdr_entry.sh_addralign))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_entsize), sizeof(shdr_entry.sh_entsize))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);

            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_name);
            shdr_entry.sh_type = static_cast<SectionType>(Utility::Endian::toPlatform(eType::Big, static_cast<std::underlying_type_t<SectionType>>(shdr_entry.sh_type)));
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_flags);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_addr);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_offset);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_size);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_link);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_info);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_addralign);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_entsize);
        }
        std::sort(shdr_table.begin(), shdr_table.end(), [](const shdr_index_t& a, const shdr_index_t& b) {return a.second.sh_offset < b.second.sh_offset; } );
        for(auto& [index, entry] : shdr_table)
        {
            if(entry.sh_offset == 0) continue;

            if (!in.seekg(entry.sh_offset))
            {
                LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            }
            entry.sh_offset = out.tellp();
            if((entry.sh_flags & SHF_RPL_ZLIB) == SHF_RPL_ZLIB)
            {
                uint32_t data_size = entry.sh_size - 4;

                if (!in.read(reinterpret_cast<char*>(&entry.sh_size), sizeof(entry.sh_size))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
                Utility::Endian::toPlatform_inplace(eType::Big, entry.sh_size);

                uint32_t block_size = CHUNK;
                uint32_t have;
                z_stream strm;
                char buff_in[CHUNK];
                char buff_out[CHUNK];
                strm.zalloc = Z_NULL;
                strm.zfree = Z_NULL;
                strm.opaque = Z_NULL;
                strm.avail_in = 0;
                strm.next_in = Z_NULL;
                int err = Z_OK;
                if ((err = inflateInit(&strm)) != Z_OK)
                {
                    LOG_ERR_AND_RETURN(RPXError::ZLIB_ERROR);
                }
                while(data_size > 0)
                {
                    block_size = CHUNK;
                    if(data_size<block_size)
                        block_size = data_size;
                    data_size -= block_size;
                    in.read(buff_in, block_size);
                    strm.avail_in = in.gcount();
                    strm.next_in = reinterpret_cast<Bytef*>(buff_in);
                    do
                    {
                        strm.avail_out = CHUNK;
                        strm.next_out = reinterpret_cast<Bytef*>(buff_out);
                        err = inflate(&strm, Z_NO_FLUSH);
                        if (err != Z_OK && err != Z_BUF_ERROR && err != Z_STREAM_END)
                        {
                            LOG_ERR_AND_RETURN(RPXError::ZLIB_ERROR);
                        }
                        have = CHUNK - strm.avail_out;
                        out.write(buff_out, have);
                        crcs[index] = crc32_rpx(crcs[index], reinterpret_cast<uint8_t*>(buff_out), have);
                    }while(strm.avail_out == 0);
                }
                if ((err = inflateEnd(&strm)) != Z_OK)
                {
                    LOG_ERR_AND_RETURN(RPXError::ZLIB_ERROR);
                }
                entry.sh_flags &= ~SHF_RPL_ZLIB;
            }
            else
            {
                uint32_t data_size = entry.sh_size;
                uint32_t block_size = CHUNK;
                while(data_size>0)
                {
                    char data[CHUNK];
                    block_size = CHUNK;
                    if(data_size < block_size)
                        block_size = data_size;
                    data_size -= block_size;
                    in.read(data, block_size);
                    out.write(data, block_size);
                    crcs[index] = crc32_rpx(crcs[index], reinterpret_cast<uint8_t*>(data), block_size);
                }
            }
            while (out.tellp() % 0x40 != 0) out.put(0);
            if(entry.sh_type == SectionType::SHT_RPL_CRCS)
            {
                crcs[index] = 0;
                crc_data_offset = entry.sh_offset;
            }
        }
        Utility::seek(out, ehdr.e_shoff);
        std::sort(shdr_table.begin(), shdr_table.end(), [](const shdr_index_t& a, const shdr_index_t& b) {return a.first < b.first; } );
        for (const auto& [index, entry] : shdr_table)
        {
            auto sh_name = Utility::Endian::toPlatform(eType::Big, entry.sh_name);
            auto sh_type = Utility::Endian::toPlatform(eType::Big, static_cast<std::underlying_type_t<SectionType>>(entry.sh_type));
            auto sh_flags = Utility::Endian::toPlatform(eType::Big, entry.sh_flags);
            auto sh_addr = Utility::Endian::toPlatform(eType::Big, entry.sh_addr);
            auto sh_offset = Utility::Endian::toPlatform(eType::Big, entry.sh_offset);
            auto sh_size = Utility::Endian::toPlatform(eType::Big, entry.sh_size);
            auto sh_link = Utility::Endian::toPlatform(eType::Big, entry.sh_link);
            auto sh_info = Utility::Endian::toPlatform(eType::Big, entry.sh_info);
            auto sh_addralign = Utility::Endian::toPlatform(eType::Big, entry.sh_addralign);
            auto sh_entsize = Utility::Endian::toPlatform(eType::Big, entry.sh_entsize);

            out.write(reinterpret_cast<const char*>(&sh_name), sizeof(sh_name));
            out.write(reinterpret_cast<const char*>(&sh_type), sizeof(sh_type));
            out.write(reinterpret_cast<const char*>(&sh_flags), sizeof(sh_flags));
            out.write(reinterpret_cast<const char*>(&sh_addr), sizeof(sh_addr));
            out.write(reinterpret_cast<const char*>(&sh_offset), sizeof(sh_offset));
            out.write(reinterpret_cast<const char*>(&sh_size), sizeof(sh_size));
            out.write(reinterpret_cast<const char*>(&sh_link), sizeof(sh_link));
            out.write(reinterpret_cast<const char*>(&sh_info), sizeof(sh_info));
            out.write(reinterpret_cast<const char*>(&sh_addralign), sizeof(sh_addralign));
            out.write(reinterpret_cast<const char*>(&sh_entsize), sizeof(sh_entsize));
        }
        
        Utility::seek(out, crc_data_offset);
        for (uint32_t i = 0; i < ehdr.e_shnum; i++) {
            auto crc = Utility::Endian::toPlatform(eType::Big, crcs[i]);
            out.write(reinterpret_cast<const char*>(&crc), sizeof(crc));
        }
        
        return RPXError::NONE;
    }

    RPXError rpx_compress(std::istream& in, std::ostream& out)
    {
        Elf32_Ehdr ehdr;
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_ident[0]), 0x10)) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (std::strncmp(reinterpret_cast<char*>(&ehdr.e_ident[0]), "\x7F""ELF", 4)) LOG_ERR_AND_RETURN(RPXError::NOT_RPX);

        if (!in.read(reinterpret_cast<char*>(&ehdr.e_type), sizeof(ehdr.e_type))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_machine), sizeof(ehdr.e_machine))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_version), sizeof(ehdr.e_version))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_entry), sizeof(ehdr.e_entry))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_phoff), sizeof(ehdr.e_phoff))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_shoff), sizeof(ehdr.e_shoff))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_flags), sizeof(ehdr.e_flags))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_ehsize), sizeof(ehdr.e_ehsize))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_phentsize), sizeof(ehdr.e_phentsize))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_phnum), sizeof(ehdr.e_phnum))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_shentsize), sizeof(ehdr.e_shentsize))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_shnum), sizeof(ehdr.e_shnum))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
        if (!in.read(reinterpret_cast<char*>(&ehdr.e_shstrndx), sizeof(ehdr.e_shstrndx))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);

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

        if (ehdr.e_type != 0xFE01) LOG_ERR_AND_RETURN(RPXError::UNKNOWN_E_TYPE);

        uint32_t shdr_data_elf_offset = ehdr.e_shoff + ehdr.e_shnum * ehdr.e_shentsize;
        out.write(reinterpret_cast<const char*>(&ehdr.e_ident[0]), 0x10);

        auto e_type = Utility::Endian::toPlatform(eType::Big, ehdr.e_type);
        auto e_machine = Utility::Endian::toPlatform(eType::Big, ehdr.e_machine);
        auto e_version = Utility::Endian::toPlatform(eType::Big, ehdr.e_version);
        auto e_entry = Utility::Endian::toPlatform(eType::Big, ehdr.e_entry);
        auto e_phoff = Utility::Endian::toPlatform(eType::Big, ehdr.e_phoff);
        auto e_shoff = Utility::Endian::toPlatform(eType::Big, ehdr.e_shoff);
        auto e_flags = Utility::Endian::toPlatform(eType::Big, ehdr.e_flags);
        auto e_ehsize = Utility::Endian::toPlatform(eType::Big, ehdr.e_ehsize);
        auto e_phentsize = Utility::Endian::toPlatform(eType::Big, ehdr.e_phentsize);
        auto e_phnum = Utility::Endian::toPlatform(eType::Big, ehdr.e_phnum);
        auto e_shentsize = Utility::Endian::toPlatform(eType::Big, ehdr.e_shentsize);
        auto e_shnum = Utility::Endian::toPlatform(eType::Big, ehdr.e_shnum);
        auto e_shstrndx = Utility::Endian::toPlatform(eType::Big, ehdr.e_shstrndx);

        out.write(reinterpret_cast<const char*>(&e_type), sizeof(e_type));
        out.write(reinterpret_cast<const char*>(&e_machine), sizeof(e_machine));
        out.write(reinterpret_cast<const char*>(&e_version), sizeof(e_version));
        out.write(reinterpret_cast<const char*>(&e_entry), sizeof(e_entry));
        out.write(reinterpret_cast<const char*>(&e_phoff), sizeof(e_phoff));
        out.write(reinterpret_cast<const char*>(&e_shoff), sizeof(e_shoff));
        out.write(reinterpret_cast<const char*>(&e_flags), sizeof(e_flags));
        out.write(reinterpret_cast<const char*>(&e_ehsize), sizeof(e_ehsize));
        out.write(reinterpret_cast<const char*>(&e_phentsize), sizeof(e_phentsize));
        out.write(reinterpret_cast<const char*>(&e_phnum), sizeof(e_phnum));
        out.write(reinterpret_cast<const char*>(&e_shentsize), sizeof(e_shentsize));
        out.write(reinterpret_cast<const char*>(&e_shnum), sizeof(e_shnum));
        out.write(reinterpret_cast<const char*>(&e_shstrndx), sizeof(e_shstrndx));

        const uint32_t zero = 0x00000000;
        out.write(reinterpret_cast<const char*>(&zero), 4);
        out.write(reinterpret_cast<const char*>(&zero), 4);
        out.write(reinterpret_cast<const char*>(&zero), 4);

        uint32_t crc_data_offset = 0;
        std::vector<uint32_t> crcs(ehdr.e_shnum, 0);
        std::vector<shdr_index_t> shdr_table(ehdr.e_shnum);
        while(static_cast<uint32_t>(out.tellp()) < shdr_data_elf_offset) out.put(0);
        in.seekg(ehdr.e_shoff);
        for (uint32_t i = 0; i < ehdr.e_shnum; i++)
        {
            shdr_table[i].first = i;
            Elf32_Shdr& shdr_entry = shdr_table[i].second;
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_name), sizeof(shdr_entry.sh_name))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_type), sizeof(shdr_entry.sh_type))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_flags), sizeof(shdr_entry.sh_flags))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_addr), sizeof(shdr_entry.sh_addr))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_offset), sizeof(shdr_entry.sh_offset))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_size), sizeof(shdr_entry.sh_size))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_link), sizeof(shdr_entry.sh_link))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_info), sizeof(shdr_entry.sh_info))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_addralign), sizeof(shdr_entry.sh_addralign))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);
            if (!in.read(reinterpret_cast<char*>(&shdr_entry.sh_entsize), sizeof(shdr_entry.sh_entsize))) LOG_ERR_AND_RETURN(RPXError::REACHED_EOF);

            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_name);
            shdr_entry.sh_type = static_cast<SectionType>(Utility::Endian::toPlatform(eType::Big, static_cast<std::underlying_type_t<SectionType>>(shdr_entry.sh_type)));
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_flags);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_addr);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_offset);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_size);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_link);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_info);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_addralign);
            Utility::Endian::toPlatform_inplace(eType::Big, shdr_entry.sh_entsize);
        }

        std::sort(shdr_table.begin(), shdr_table.end(), [](const shdr_index_t& a, const shdr_index_t& b) {return a.second.sh_offset < b.second.sh_offset; } );
        for(auto& [index, entry] : shdr_table)
        {
            if(entry.sh_offset == 0) continue;

            in.seekg(entry.sh_offset);
            entry.sh_offset = out.tellp();
            if ((entry.sh_type == SectionType::SHT_RPL_FILEINFO)||
                (entry.sh_type == SectionType::SHT_RPL_CRCS)||
                ((entry.sh_flags & SHF_RPL_ZLIB) == SHF_RPL_ZLIB))
            {
                uint32_t data_size = entry.sh_size;
                uint32_t block_size = CHUNK;
                while(data_size>0)
                {
                    std::string data(CHUNK, '\0');
                    block_size = CHUNK;
                    if(data_size<block_size)
                        block_size = data_size;
                    data_size -= block_size;
                    in.read(&data[0], block_size);
                    out.write(&data[0], block_size);
                    crcs[index] = crc32_rpx(crcs[index], reinterpret_cast<uint8_t*>(&data[0]), block_size);
                }
            }
            else
            {
                uint32_t data_size = entry.sh_size;
                uint32_t block_size = CHUNK;
                uint32_t have;
                z_stream strm;
                std::string buff_in(CHUNK, '\0');
                std::string buff_out(CHUNK, '\0');
                strm.zalloc = Z_NULL;
                strm.zfree = Z_NULL;
                strm.opaque = Z_NULL;
                strm.avail_in = 0;
                strm.next_in = Z_NULL;
                if(data_size<CHUNK)
                {
                    block_size = data_size;
                    deflateInit(&strm, LEVEL);
                    in.read(&buff_in[0], block_size);
                    strm.avail_in = in.gcount();
                    crcs[index] = crc32_rpx(crcs[index], reinterpret_cast<uint8_t*>(&buff_in[0]), block_size);
                    strm.next_in = reinterpret_cast<Bytef*>(&buff_in[0]);
                    strm.avail_out = CHUNK;
                    strm.next_out = reinterpret_cast<Bytef*>(&buff_out[0]);
                    deflate(&strm, Z_FINISH);
                    have = CHUNK - strm.avail_out;
                    if(have + 4 < block_size)
                    {
                        auto data_size_BE = Utility::Endian::toPlatform(eType::Big, data_size);
                        out.write(reinterpret_cast<const char*>(&data_size_BE), sizeof(data_size_BE));
                        out.write(&buff_out[0], have);
                        entry.sh_size = have + 4;
                        entry.sh_flags |= SHF_RPL_ZLIB;
                    }
                    else
                        out.write(&buff_in[0], block_size);
                    deflateEnd(&strm);
                }
                else
                {
                    int32_t flush = Z_NO_FLUSH;
                    uint32_t compress_size = 4;

                    auto data_size_BE = Utility::Endian::toPlatform(eType::Big, data_size);
                    out.write(reinterpret_cast<const char*>(&data_size_BE), sizeof(data_size_BE));

                    deflateInit(&strm, LEVEL);
                    while(data_size>0)
                    {
                        block_size = CHUNK;
                        flush = Z_NO_FLUSH;
                        if(data_size <= block_size)
                        {
                            block_size = data_size;
                            flush = Z_FINISH;
                        }
                        data_size -= block_size;
                        in.read(&buff_in[0], block_size);
                        strm.avail_in = in.gcount();
                        crcs[index] = crc32_rpx(crcs[index], reinterpret_cast<uint8_t*>(&buff_in[0]), block_size);
                        strm.next_in = reinterpret_cast<Bytef*>(&buff_in[0]);
                        do{
                            strm.avail_out = CHUNK;
                            strm.next_out = reinterpret_cast<Bytef*>(&buff_out[0]);
                            deflate(&strm, flush);
                            have = CHUNK - strm.avail_out;
                            out.write(&buff_out[0], have);
                            compress_size += have;
                        } while(strm.avail_out == 0);
                    }
                    deflateEnd(&strm);
                    entry.sh_size = compress_size;
                    entry.sh_flags |= SHF_RPL_ZLIB;
                }
            }
            while(out.tellp() % 0x40 != 0) out.put(0);
            if(entry.sh_type == SectionType::SHT_RPL_CRCS)
            {
                crcs[index] = 0;
                crc_data_offset = entry.sh_offset;
            }
        }
        Utility::seek(out, ehdr.e_shoff);
        std::sort(shdr_table.begin(), shdr_table.end(), [](const shdr_index_t& a, const shdr_index_t& b) {return a.first < b.first; } );
        for (const auto& [index, entry] : shdr_table)
        {
            auto sh_name = Utility::Endian::toPlatform(eType::Big, entry.sh_name);
            auto sh_type = Utility::Endian::toPlatform(eType::Big, entry.sh_type);
            auto sh_flags = Utility::Endian::toPlatform(eType::Big, entry.sh_flags);
            auto sh_addr = Utility::Endian::toPlatform(eType::Big, entry.sh_addr);
            auto sh_offset = Utility::Endian::toPlatform(eType::Big, entry.sh_offset);
            auto sh_size = Utility::Endian::toPlatform(eType::Big, entry.sh_size);
            auto sh_link = Utility::Endian::toPlatform(eType::Big, entry.sh_link);
            auto sh_info = Utility::Endian::toPlatform(eType::Big, entry.sh_info);
            auto sh_addralign = Utility::Endian::toPlatform(eType::Big, entry.sh_addralign);
            auto sh_entsize = Utility::Endian::toPlatform(eType::Big, entry.sh_entsize);

            out.write(reinterpret_cast<const char*>(&sh_name), sizeof(sh_name));
            out.write(reinterpret_cast<const char*>(&sh_type), sizeof(sh_type));
            out.write(reinterpret_cast<const char*>(&sh_flags), sizeof(sh_flags));
            out.write(reinterpret_cast<const char*>(&sh_addr), sizeof(sh_addr));
            out.write(reinterpret_cast<const char*>(&sh_offset), sizeof(sh_offset));
            out.write(reinterpret_cast<const char*>(&sh_size), sizeof(sh_size));
            out.write(reinterpret_cast<const char*>(&sh_link), sizeof(sh_link));
            out.write(reinterpret_cast<const char*>(&sh_info), sizeof(sh_info));
            out.write(reinterpret_cast<const char*>(&sh_addralign), sizeof(sh_addralign));
            out.write(reinterpret_cast<const char*>(&sh_entsize), sizeof(sh_entsize));
        }

        Utility::seek(out, crc_data_offset);
        for (uint32_t i = 0; i < ehdr.e_shnum; i++) {
            auto crc = Utility::Endian::toPlatform(eType::Big, crcs[i]);
            out.write(reinterpret_cast<const char*>(&crc), sizeof(crc));
        }

        return RPXError::NONE;
    }
}
