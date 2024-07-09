#include "sarc.hpp"

#include <cstring>
#include <algorithm>
#include <numeric>
#include <filesystem>

#include <utility/endian.hpp>
#include <utility/common.hpp>
#include <utility/file.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;

const std::unordered_map<std::string, uint32_t> alignments = {
    {"bflan", 0x00000004},
    {"bflyt", 0x00000004},
    {"szs", 0x00002000},
    {"sarc", 0x00002000},
    {"bfres", 0x00002000}, //seems to be 0x2000 usually
    {"sharcfb", 0x00002000} //seems to be 0x2000 usually, sometimes 0x100 in .sarc files?
};

namespace {
    uint32_t calculateHash(const std::string& name, uint32_t multiplier) {
        uint32_t hash = 0;
        for (const int8_t byte : name) {
            if (byte == 0x00) break; //string is null-terminated
            hash = hash * multiplier + byte;
        }
    
        return hash;
    }
    
    uint32_t getAlignment(const std::string& fileExt, const FileTypes::SARCFile::File& file) {
        if (alignments.contains(fileExt)) {
            return alignments.at(fileExt);
        }
        if (fileExt == "bflim" && file.data.substr(file.data.size() - 0x28, 4) == "FLIM") {
            uint16_t alignment = *reinterpret_cast<const uint16_t*>(&file.data[(file.data.size() - 8)]);
            Utility::Endian::toPlatform_inplace(eType::Big, alignment);
            return alignment;
        }
        return 0;
    }
}

namespace FileTypes{

    const char* SARCErrorGetName(SARCError err) {
        switch (err) {
        case SARCError::NONE:
            return "NONE";
        case SARCError::COULD_NOT_OPEN:
            return "COULD_NOT_OPEN";
        case SARCError::NOT_SARC:
            return "NOT_SARC";
        case SARCError::UNKNOWN_VERSION:
            return "UNKNOWN_VERSION";
        case SARCError::REACHED_EOF:
            return "REACHED_EOF";
        case SARCError::STRING_TOO_LONG:
            return "STRING_TOO_LONG";
        case SARCError::BAD_NODE_ATTR:
            return "BAD_NODE_ATTR";
        case SARCError::STRING_NOT_FOUND:
            return "STRING_NOT_FOUND";
        case SARCError::FILENAME_HASH_MISMATCH:
            return "FILENAME_HASH_MISMATCH";
        case SARCError::UNEXPECTED_VALUE:
            return "UNEXPECTED_VALUE";
        default:
            return "UNKNOWN";
        }
    }

    void SARCFile::initNew() {
        memcpy(&header.magicSARC, "SARC", 4);
        header.headerSize_0x14 = 0x14;
        header.byteOrderMarker = 0xFEFF;
        header.fileSize = 0;
        header.dataOffset = 0;
        header.version_0x0100 = 0x0100;
        header.padding_0x00[0] = 0x00;
        header.padding_0x00[1] = 0x00;

        memcpy(&fileTable.magicSFAT, "SFAT", 4);
        fileTable.headerSize_0xC = 0xC;
        fileTable.numFiles = 0;
        fileTable.hashKey_0x65 = 0x00000065;
        fileTable.nodes = {};

        memcpy(&nameTable.magicSFNT, "SFNT", 4);
        nameTable.headerSize_0x8 = 0x8;
        nameTable.padding_0x00[0] = 0x00;
        nameTable.padding_0x00[1] = 0x00;
        nameTable.filenames = {};

        file_index_by_name = {};
        files = {};
    }

    SARCFile SARCFile::createNew() {
        SARCFile newSARC{};
        newSARC.initNew();
        return newSARC;
    }

    void SARCFile::guessDefaultAlignment() { //most taken from https://github.com/zeldamods/sarc/blob/master/sarc/sarc.py#L98
        if(files.size() <= 2) {
            guessed_alignment = 4;
            return;
        }

        uint32_t gcd = header.dataOffset;
        for(const SFATNode& node : fileTable.nodes) {
            gcd = std::gcd(gcd, node.dataStart + header.dataOffset);
        }

        if(gcd == 0 || (gcd & (gcd - 1)) != 0) {
            guessed_alignment = 4;
            return;
        }

        guessed_alignment = gcd;
    }

    SARCError SARCFile::loadFromBinary(std::istream& sarc) {
        if (!sarc.read(header.magicSARC, 4)) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
        if (!sarc.read(reinterpret_cast<char*>(&header.headerSize_0x14), sizeof(header.headerSize_0x14))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
        if (!sarc.read(reinterpret_cast<char*>(&header.byteOrderMarker), sizeof(header.byteOrderMarker))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
        if (!sarc.read(reinterpret_cast<char*>(&header.fileSize), sizeof(header.fileSize))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
        if (!sarc.read(reinterpret_cast<char*>(&header.dataOffset), sizeof(header.dataOffset))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
        if (!sarc.read(reinterpret_cast<char*>(&header.version_0x0100), sizeof(header.version_0x0100))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
        if (!sarc.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);

        Utility::Endian::toPlatform_inplace(eType::Big, header.headerSize_0x14);
        Utility::Endian::toPlatform_inplace(eType::Big, header.byteOrderMarker);
        Utility::Endian::toPlatform_inplace(eType::Big, header.fileSize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.dataOffset);
        Utility::Endian::toPlatform_inplace(eType::Big, header.version_0x0100);

        if (std::strncmp("SARC", header.magicSARC, 4) != 0) LOG_ERR_AND_RETURN(SARCError::NOT_SARC);
        if (header.headerSize_0x14 != 0x0014) LOG_ERR_AND_RETURN(SARCError::UNEXPECTED_VALUE);
        if (header.byteOrderMarker != 0xFEFF) LOG_ERR_AND_RETURN(SARCError::UNEXPECTED_VALUE);
        if (header.version_0x0100 != 0x0100) LOG_ERR_AND_RETURN(SARCError::UNKNOWN_VERSION);
        if (header.padding_0x00[0] != 0x00 || header.padding_0x00[1] != 0x00) LOG_ERR_AND_RETURN(SARCError::UNEXPECTED_VALUE);

        if (!sarc.read(fileTable.magicSFAT, 4)) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
        if (!sarc.read(reinterpret_cast<char*>(&fileTable.headerSize_0xC), sizeof(fileTable.headerSize_0xC))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
        if (!sarc.read(reinterpret_cast<char*>(&fileTable.numFiles), sizeof(fileTable.numFiles))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
        if (!sarc.read(reinterpret_cast<char*>(&fileTable.hashKey_0x65), sizeof(fileTable.hashKey_0x65))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);

        Utility::Endian::toPlatform_inplace(eType::Big, fileTable.headerSize_0xC);
        Utility::Endian::toPlatform_inplace(eType::Big, fileTable.numFiles);
        Utility::Endian::toPlatform_inplace(eType::Big, fileTable.hashKey_0x65);

        if (std::strncmp("SFAT", fileTable.magicSFAT, 4) != 0) LOG_ERR_AND_RETURN(SARCError::UNEXPECTED_VALUE);
        if (fileTable.headerSize_0xC != 0xC) LOG_ERR_AND_RETURN(SARCError::UNEXPECTED_VALUE);
        if (fileTable.hashKey_0x65 != 0x65) LOG_ERR_AND_RETURN(SARCError::UNEXPECTED_VALUE);

        for (uint16_t i = 0; i < fileTable.numFiles; i++) {
            SFATNode& node = fileTable.nodes.emplace_back();
            if (!sarc.read(reinterpret_cast<char*>(&node.nameHash), sizeof(node.nameHash))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
            if (!sarc.read(reinterpret_cast<char*>(&node.attributes), sizeof(node.attributes))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
            if (!sarc.read(reinterpret_cast<char*>(&node.dataStart), sizeof(node.dataStart))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
            if (!sarc.read(reinterpret_cast<char*>(&node.dataEnd), sizeof(node.dataEnd))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);

            Utility::Endian::toPlatform_inplace(eType::Big, node.nameHash);
            Utility::Endian::toPlatform_inplace(eType::Big, node.attributes);
            Utility::Endian::toPlatform_inplace(eType::Big, node.dataStart);
            Utility::Endian::toPlatform_inplace(eType::Big, node.dataEnd);
        }

        const std::streamoff SFNTOffset = sarc.tellg();
        if (!sarc.read(nameTable.magicSFNT, 4)) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
        if (!sarc.read(reinterpret_cast<char*>(&nameTable.headerSize_0x8), sizeof(nameTable.headerSize_0x8))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
        if (!sarc.read(reinterpret_cast<char*>(&nameTable.padding_0x00), sizeof(nameTable.padding_0x00))) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);

        Utility::Endian::toPlatform_inplace(eType::Big, nameTable.headerSize_0x8);

        if (std::strncmp("SFNT", nameTable.magicSFNT, 4) != 0) LOG_ERR_AND_RETURN(SARCError::UNEXPECTED_VALUE);
        if (nameTable.headerSize_0x8 != 0x8) LOG_ERR_AND_RETURN(SARCError::UNEXPECTED_VALUE);
        if (nameTable.padding_0x00[0] != 0x00 || nameTable.padding_0x00[1] != 0x00) LOG_ERR_AND_RETURN(SARCError::UNEXPECTED_VALUE);

        std::string fileData(header.fileSize - header.dataOffset, '\0');
        sarc.seekg(header.dataOffset, std::ios::beg);
        if (!sarc.read(&fileData[0], header.fileSize - header.dataOffset)) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);

        for (const SFATNode& node : fileTable.nodes) {
            if ((node.attributes & 0xFF000000) >> 24 != 0x01) LOG_ERR_AND_RETURN(SARCError::BAD_NODE_ATTR);
            const uint32_t nameOffset = (node.attributes & 0x00FFFFFF) * 4;
            const std::string& name = nameTable.filenames.emplace_back(readNullTerminatedStr(sarc, SFNTOffset + 0x8 + nameOffset));
            if (name.empty()) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);

            const uint32_t hash = calculateHash(name, fileTable.hashKey_0x65);
            if (hash != node.nameHash) LOG_ERR_AND_RETURN(SARCError::FILENAME_HASH_MISMATCH);

            File& fileEntry = files.emplace_back();
            file_index_by_name[name] = files.size() - 1;
            fileEntry.name = name;
            fileEntry.data = fileData.substr(node.dataStart, node.dataEnd - node.dataStart);
        }

        guessDefaultAlignment();
        return SARCError::NONE;
    }

    SARCError SARCFile::loadFromFile(const fspath& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERR_AND_RETURN(SARCError::COULD_NOT_OPEN);
        }
        return loadFromBinary(file);
    }

    SARCFile::File* SARCFile::getFile(const std::string& filename) {
        if (!file_index_by_name.contains(filename)) {
            return nullptr;
        }
        return &files[file_index_by_name.at(filename)];
    }

    SARCError SARCFile::writeToStream(std::ostream& out) {
        //update offsets of files
        uint32_t curDataOffset = 0;
        for(size_t i = 0; i < files.size(); i++) {
            SFATNode& node = fileTable.nodes[i];
            const File& entry = files[i];
            
            std::string filetype = entry.name.substr(entry.name.find('.') + 1);
            filetype.pop_back();
            const uint32_t alignment = std::max(guessed_alignment, getAlignment(filetype, entry));
            if (alignment != 0) {
                unsigned int padLen = alignment - (curDataOffset % alignment);
                if (padLen == alignment) padLen = 0;
                node.dataStart = curDataOffset + padLen;
            }
            else {
                node.dataStart = curDataOffset;
            }

            node.dataEnd = node.dataStart + entry.data.size();
            curDataOffset = node.dataEnd;
        }

        {
            uint16_t headerSize_BE = Utility::Endian::toPlatform(eType::Big, header.headerSize_0x14);
            uint16_t byteOrderMarker_BE = Utility::Endian::toPlatform(eType::Big, header.byteOrderMarker);
            uint32_t dataOffset_BE = Utility::Endian::toPlatform(eType::Big, header.dataOffset);
            uint16_t version_BE = Utility::Endian::toPlatform(eType::Big, header.version_0x0100);

            out.write(header.magicSARC, 4);
            out.write(reinterpret_cast<const char*>(&headerSize_BE), sizeof(headerSize_BE));
            out.write(reinterpret_cast<const char*>(&byteOrderMarker_BE), sizeof(byteOrderMarker_BE));
            out.write("\0\0\0\0", 4); //pad for filesize
            out.write(reinterpret_cast<const char*>(&dataOffset_BE), sizeof(dataOffset_BE));
            out.write(reinterpret_cast<const char*>(&version_BE), sizeof(version_BE));
            out.write(reinterpret_cast<const char*>(&header.padding_0x00), sizeof(header.padding_0x00));
        }

        {
            uint16_t headerSize_BE = Utility::Endian::toPlatform(eType::Big, fileTable.headerSize_0xC);
            uint16_t numFiles_BE = Utility::Endian::toPlatform(eType::Big, fileTable.numFiles);
            uint32_t hashKey_BE = Utility::Endian::toPlatform(eType::Big, fileTable.hashKey_0x65);

            out.write(fileTable.magicSFAT, 4);
            out.write(reinterpret_cast<const char*>(&headerSize_BE), sizeof(headerSize_BE));
            out.write(reinterpret_cast<const char*>(&numFiles_BE), sizeof(numFiles_BE));
            out.write(reinterpret_cast<const char*>(&hashKey_BE), sizeof(hashKey_BE));

            for (const SFATNode& node : fileTable.nodes) {
                uint32_t nameHash_BE = Utility::Endian::toPlatform(eType::Big, node.nameHash);
                uint32_t attributes_BE = Utility::Endian::toPlatform(eType::Big, node.attributes);
                uint32_t dataStart_BE = Utility::Endian::toPlatform(eType::Big, node.dataStart);
                uint32_t dataEnd_BE = Utility::Endian::toPlatform(eType::Big, node.dataEnd);

                out.write(reinterpret_cast<const char*>(&nameHash_BE), sizeof(nameHash_BE));
                out.write(reinterpret_cast<const char*>(&attributes_BE), sizeof(attributes_BE));
                out.write(reinterpret_cast<const char*>(&dataStart_BE), sizeof(dataStart_BE));
                out.write(reinterpret_cast<const char*>(&dataEnd_BE), sizeof(dataEnd_BE));
            }
        }

        {
            uint16_t headerSize_BE = Utility::Endian::toPlatform(eType::Big, nameTable.headerSize_0x8);

            out.write(nameTable.magicSFNT, 4);
            out.write(reinterpret_cast<const char*>(&headerSize_BE), sizeof(headerSize_BE));
            out.write(reinterpret_cast<const char*>(&nameTable.padding_0x00), sizeof(nameTable.padding_0x00));

            for (const std::string& filename : nameTable.filenames) {
                out.write(&filename[0], filename.length());
                padToLen(out, 4);
            }
        }

        for (unsigned int i = 0; i < files.size(); i++) {
            const std::string fill((header.dataOffset + fileTable.nodes[i].dataStart) - out.tellp(), '\0');
            out.write(&fill[0], fill.size());
            out.write(&files[i].data[0], files[i].data.size());
        }

        header.fileSize = out.tellp();
        out.seekp(8, std::ios::beg);
        uint32_t fileSize_BE = Utility::Endian::toPlatform(eType::Big, header.fileSize);
        out.write(reinterpret_cast<const char*>(&fileSize_BE), sizeof(fileSize_BE));

        return SARCError::NONE;
    }

    SARCError SARCFile::writeToFile(const fspath& outFilePath) {
        std::ofstream outFile(outFilePath, std::ios::binary);
        if (!outFile.is_open()) {
            LOG_ERR_AND_RETURN(SARCError::COULD_NOT_OPEN);
        }
        return writeToStream(outFile);
    }

    SARCError SARCFile::extractToDir(const fspath& dirPath) const {
        for (const File& file : files)
        {
            const fspath path = dirPath / file.name;
            Utility::create_directories(path.parent_path()); //handle any folder structure stuff contained in the SARC
            std::ofstream outFile(path, std::ios::binary);
            if (!outFile.is_open())
            {
                LOG_ERR_AND_RETURN(SARCError::COULD_NOT_OPEN);
            }
            outFile.write(&file.data[0], file.data.size());
        }
        return SARCError::NONE;
    }

    SARCError SARCFile::replaceFile(const std::string& filename, const std::stringstream& newData) {
        if(!file_index_by_name.contains(filename)) LOG_ERR_AND_RETURN(SARCError::STRING_NOT_FOUND);
        const size_t& fileIndex = file_index_by_name.at(filename);
        File& entry = files[fileIndex];

        entry.data = newData.str();

        return SARCError::NONE;
    }

    SARCError SARCFile::replaceFile(const std::string& filename, const fspath& newFilePath) {
        const size_t& fileIndex = file_index_by_name.at(filename);
        uint32_t curDataOffset = 0;
        {
            if(!file_index_by_name.contains(filename)) LOG_ERR_AND_RETURN(SARCError::STRING_NOT_FOUND);
            SFATNode& node = fileTable.nodes[fileIndex];
            File& entry = files[fileIndex];

            const uint32_t fileSize = std::filesystem::file_size(newFilePath);
            curDataOffset = node.dataStart;

            entry.data.resize(fileSize);
            std::ifstream inFile(newFilePath, std::ios::binary);
            if (!inFile.read(&entry.data[0], fileSize)) {
                LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
            }

            node.dataEnd = node.dataStart + entry.data.size();
            curDataOffset = node.dataEnd;
        }

        //update offsets of later files
        for(size_t i = fileIndex + 1; i < files.size(); i++) {
            SFATNode& node = fileTable.nodes[i];
            const File& entry = files[i];
            
            std::string filetype = entry.name.substr(entry.name.find('.') + 1);
            filetype.pop_back();
            const uint32_t alignment = std::max(guessed_alignment, getAlignment(filetype, entry));
            if (alignment != 0) {
                unsigned int padLen = alignment - (curDataOffset % alignment);
                if (padLen == alignment) padLen = 0;
                node.dataStart = curDataOffset + padLen;
            }
            else {
                node.dataStart = curDataOffset;
            }

            node.dataEnd = node.dataStart + entry.data.size();
            curDataOffset = node.dataEnd;
        }

        return SARCError::NONE;
    }

    SARCError SARCFile::rebuildFromDir(const fspath& dirPath) {
        //rebuild using the original filename list (so extraneous unpacked stuff isnt added accidentally)

        uint32_t curDataOffset = 0;
        for (unsigned int i = 0; i < nameTable.filenames.size(); i++) {
            const std::string& filename = nameTable.filenames[i];
            fspath absPath = dirPath / filename;
            uint32_t fileSize = std::filesystem::file_size(absPath);
            SFATNode& node = fileTable.nodes[i];

            File& entry = files[i];
            entry.name = filename;
            entry.data.resize(fileSize);

            std::ifstream inFile(absPath, std::ios::binary);
            if (!inFile.read(&entry.data[0], fileSize)) {
                LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
            }

            //silly alignment stuff
            std::string filetype = filename.substr(filename.find('.') + 1);
            filetype.pop_back();
            uint32_t alignment = std::max(guessed_alignment, getAlignment(filetype, entry));
            if (alignment != 0) {
                unsigned int padLen = alignment - (curDataOffset % alignment);
                if (padLen == alignment) padLen = 0;
                node.dataStart = curDataOffset + padLen;
            }
            else {
                node.dataStart = curDataOffset;
            }

            node.dataEnd = node.dataStart + entry.data.size();
            curDataOffset = node.dataEnd;

            file_index_by_name[filename] = i;
        }

        return SARCError::NONE;
    }

    SARCError SARCFile::buildFromDir(const fspath& dirPath) { //needs some implementation updates to work completely from a new sarc
        fileTable.numFiles = 0;
        fileTable.nodes.clear();
        nameTable.filenames.clear();
        files.clear();

        uint32_t curDataOffset = 0x14 + 0xC + 0x8; //header sizes
        uint32_t curNameOffset = 0;
        for (const auto& path : std::filesystem::recursive_directory_iterator(dirPath)) {
            if (path.is_regular_file()) {
                const fspath& absPath = path.path();
                std::string filename = std::filesystem::relative(absPath, dirPath).generic_string();
                filename += '\0'; //add null terminator

                uint32_t fileSize = std::filesystem::file_size(absPath);
                SFATNode& node = fileTable.nodes.emplace_back();
                node.nameHash = calculateHash(filename, fileTable.hashKey_0x65);

                File& entry = files.emplace_back();
                entry.name = filename;
                entry.data.resize(fileSize);

                curDataOffset += 0x10 + filename.size(); //add node + filename length
                unsigned int numPaddingBytes = 4 - (filename.size() % 4);
                if (numPaddingBytes == 4) numPaddingBytes = 0;
                curDataOffset += numPaddingBytes; //add name padding

                std::ifstream inFile(absPath, std::ios::binary);
                if (!inFile.read(&entry.data[0], fileSize)) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
            }
        }

        unsigned int numPaddingBytes = 0x100 - (curDataOffset % 0x100);
        if (numPaddingBytes == 0x100) numPaddingBytes = 0;
        curDataOffset += numPaddingBytes; //pad to nearest 0x100, maybe not needed but haven't seen something to disprove it

        header.dataOffset = curDataOffset;

        std::ranges::sort(files, [&](const File& a, const File& b) {return calculateHash(a.name, fileTable.hashKey_0x65) < calculateHash(b.name, fileTable.hashKey_0x65); });
        std::ranges::sort(fileTable.nodes, [&](const SFATNode& a, const SFATNode& b) {return a.nameHash < b.nameHash; });
        fileTable.numFiles = files.size();

        curDataOffset = 0;

        for (unsigned int i = 0; i < files.size(); i++) {
            const File& entry = files[i];
            SFATNode& node = fileTable.nodes[i];

            //silly alignment stuff
            const std::string& filename = entry.name;
            std::string filetype = filename.substr(filename.find('.') + 1);
            filetype.pop_back();
            uint32_t alignment = std::max(guessed_alignment, getAlignment(filetype, entry));
            if (alignment != 0) {
                unsigned int padLen = alignment - (curDataOffset % alignment);
                if (padLen == alignment) padLen = 0;
                node.dataStart = curDataOffset + padLen;
            }
            else {
                node.dataStart = curDataOffset;
            }

            node.dataEnd = node.dataStart + entry.data.size();
            curDataOffset = node.dataEnd;

            node.attributes = 0x01000000 | ((curNameOffset / 4) & 0x00FFFFFF);
            curNameOffset += filename.size();
            numPaddingBytes = 4 - (curNameOffset % 4);
            if (numPaddingBytes == 4) numPaddingBytes = 0;
            curNameOffset += numPaddingBytes;
            nameTable.filenames.push_back(filename);
            file_index_by_name[filename] = i;
        }

        header.fileSize = header.dataOffset + fileTable.nodes.back().dataEnd;

        LOG_ERR_AND_RETURN(SARCError::UNKNOWN); //Unfinished, return error
    }
}
