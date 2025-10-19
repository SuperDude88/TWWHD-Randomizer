#include "sarc.hpp"

#include <cstring>
#include <filesystem>
#include <unordered_map>
#include <fstream>

#include <utility/endian.hpp>
#include <utility/common.hpp>
#include <utility/string.hpp>
#include <utility/file.hpp>
#include <utility/math.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;

namespace {
    uint32_t getAlignment(const std::string& fileExt, const std::string& file) {
        // TODO: check these numbers with more examples
        static const std::unordered_map<std::string, uint32_t> alignments = {
            {"bflan", 0x00000004},
            {"bflyt", 0x00000004},
            {"szs", 0x00002000},
            {"sarc", 0x00002000},
            {"bfres", 0x00002000}, // seems to always(?) be 0x2000
            {"sharcfb", 0x00002000}
        };

        if (alignments.contains(fileExt)) {
            return alignments.at(fileExt);
        }

        if (fileExt == "bflim" && file.substr(file.size() - 0x28, 4) == "FLIM") {
            const uint16_t alignment = *reinterpret_cast<const uint16_t*>(file.substr(file.size() - 8).data());
            return Utility::Endian::toPlatform(eType::Big, alignment);
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

        files = {};
    }

    SARCFile SARCFile::createNew() {
        SARCFile newSARC{};
        newSARC.initNew();
        return newSARC;
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

        fileTable.nodes.reserve(fileTable.numFiles);
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

        std::string fileData(header.fileSize - header.dataOffset, '\0'); // TODO: don't do this giant allocation just read each file later?
        sarc.seekg(header.dataOffset, std::ios::beg);
        if (!sarc.read(fileData.data(), header.fileSize - header.dataOffset)) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);

        for (const SFATNode& node : fileTable.nodes) {
            if ((node.attributes & 0xFF000000) >> 24 != 0x01) LOG_ERR_AND_RETURN(SARCError::BAD_NODE_ATTR); // TODO: handle hash collisions

            const uint32_t nameOffset = (node.attributes & 0x00FFFFFF) * 4;
            const std::string& name = nameTable.filenames.emplace_back(Utility::Str::readNullTerminatedStr<std::string>(sarc, SFNTOffset + 0x8 + nameOffset, true));
            if (name.empty()) LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);

            if (calculateHash(name, fileTable.hashKey_0x65) != node.nameHash) LOG_ERR_AND_RETURN(SARCError::FILENAME_HASH_MISMATCH);

            files.emplace(name, fileData.substr(node.dataStart, node.dataEnd - node.dataStart));
        }

        return SARCError::NONE;
    }

    SARCError SARCFile::loadFromFile(const fspath& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERR_AND_RETURN(SARCError::COULD_NOT_OPEN);
        }
        return loadFromBinary(file);
    }

    std::string* SARCFile::getFile(const std::string& filename) {
        if (!files.contains(filename)) {
            return nullptr;
        }

        return &files.at(filename);
    }

    SARCError SARCFile::writeToStream(std::ostream& out) {
        fileTable.nodes.clear();
        nameTable.filenames.clear();
        fileTable.nodes.reserve(files.size());
        nameTable.filenames.reserve(files.size());

        // generate the file and name tables
        uint32_t curDataOffset = 0;
        uint32_t curNameOffset = 0;
        for(const auto& [name, data] : files) {
            const std::string& filename = Utility::Str::assureNullTermination(name);

            SFATNode& node = fileTable.nodes.emplace_back();

            node.nameHash = calculateHash(filename, fileTable.hashKey_0x65);

            nameTable.filenames.emplace_back(filename);
            curNameOffset = roundUp<uint32_t>(curNameOffset, 4);
            node.attributes = 0x01000000 | ((curNameOffset / 4) & 0x00FFFFFF); // TODO: handle hash collisions (attributes & 0xFF000000)
            curNameOffset += filename.length();

            const std::string& filetype = name.substr(name.find('.') + 1);
            if (const uint32_t& alignment = getAlignment(filetype, data); alignment != 0) {
                curDataOffset = roundUp<uint32_t>(curDataOffset, alignment);
            }

            node.dataStart = curDataOffset;
            node.dataEnd = node.dataStart + data.size();

            curDataOffset = node.dataEnd;
        }

        header.dataOffset = roundUp<uint32_t>(0x14 + fileTable.nodes.size() * 0x10 + curNameOffset, 0x2000);
        header.fileSize = header.dataOffset + fileTable.nodes.back().dataEnd;

        {
            const uint16_t headerSize_BE = Utility::Endian::toPlatform(eType::Big, header.headerSize_0x14);
            const uint16_t byteOrderMarker_BE = Utility::Endian::toPlatform(eType::Big, header.byteOrderMarker);
            const uint32_t fileSize_BE = Utility::Endian::toPlatform(eType::Big, header.fileSize);
            const uint32_t dataOffset_BE = Utility::Endian::toPlatform(eType::Big, header.dataOffset);
            const uint16_t version_BE = Utility::Endian::toPlatform(eType::Big, header.version_0x0100);

            out.write(header.magicSARC, 4);
            out.write(reinterpret_cast<const char*>(&headerSize_BE), sizeof(headerSize_BE));
            out.write(reinterpret_cast<const char*>(&byteOrderMarker_BE), sizeof(byteOrderMarker_BE));
            out.write(reinterpret_cast<const char*>(&fileSize_BE), sizeof(fileSize_BE));
            out.write(reinterpret_cast<const char*>(&dataOffset_BE), sizeof(dataOffset_BE));
            out.write(reinterpret_cast<const char*>(&version_BE), sizeof(version_BE));
            out.write(reinterpret_cast<const char*>(&header.padding_0x00), sizeof(header.padding_0x00));
        }

        fileTable.numFiles = fileTable.nodes.size();

        {
            const uint16_t headerSize_BE = Utility::Endian::toPlatform(eType::Big, fileTable.headerSize_0xC);
            const uint16_t numFiles_BE = Utility::Endian::toPlatform(eType::Big, fileTable.numFiles);
            const uint32_t hashKey_BE = Utility::Endian::toPlatform(eType::Big, fileTable.hashKey_0x65);

            out.write(fileTable.magicSFAT, 4);
            out.write(reinterpret_cast<const char*>(&headerSize_BE), sizeof(headerSize_BE));
            out.write(reinterpret_cast<const char*>(&numFiles_BE), sizeof(numFiles_BE));
            out.write(reinterpret_cast<const char*>(&hashKey_BE), sizeof(hashKey_BE));

            for (const SFATNode& node : fileTable.nodes) {
                const uint32_t nameHash_BE = Utility::Endian::toPlatform(eType::Big, node.nameHash);
                const uint32_t attributes_BE = Utility::Endian::toPlatform(eType::Big, node.attributes);
                const uint32_t dataStart_BE = Utility::Endian::toPlatform(eType::Big, node.dataStart);
                const uint32_t dataEnd_BE = Utility::Endian::toPlatform(eType::Big, node.dataEnd);

                out.write(reinterpret_cast<const char*>(&nameHash_BE), sizeof(nameHash_BE));
                out.write(reinterpret_cast<const char*>(&attributes_BE), sizeof(attributes_BE));
                out.write(reinterpret_cast<const char*>(&dataStart_BE), sizeof(dataStart_BE));
                out.write(reinterpret_cast<const char*>(&dataEnd_BE), sizeof(dataEnd_BE));
            }
        }

        {
            const uint16_t headerSize_BE = Utility::Endian::toPlatform(eType::Big, nameTable.headerSize_0x8);

            out.write(nameTable.magicSFNT, 4);
            out.write(reinterpret_cast<const char*>(&headerSize_BE), sizeof(headerSize_BE));
            out.write(reinterpret_cast<const char*>(&nameTable.padding_0x00), sizeof(nameTable.padding_0x00));

            for (const std::string& filename : nameTable.filenames) {
                out.write(filename.data(), filename.length());
                padToLen(out, 4);
            }
        }

        for (size_t i = 0; const auto& [name, data] : files) {
            Utility::seek(out, header.dataOffset + fileTable.nodes[i].dataStart);
            out.write(data.data(), data.size());

            i++;
        }

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
        for (const auto& [name, data] : files)
        {
            const fspath path = dirPath / name;
            std::filesystem::create_directories(path.parent_path()); // handle any folder structure stuff contained in the SARC
            std::ofstream outFile(path, std::ios::binary);
            if (!outFile.is_open())
            {
                LOG_ERR_AND_RETURN(SARCError::COULD_NOT_OPEN);
            }

            outFile.write(data.data(), data.size());
        }

        return SARCError::NONE;
    }

    SARCError SARCFile::replaceFile(const std::string& filename, const fspath& newFilePath) {
        if(!files.contains(filename)) LOG_ERR_AND_RETURN(SARCError::STRING_NOT_FOUND);

        const uint32_t fileSize = std::filesystem::file_size(newFilePath);

        std::string& data = files.at(filename);
        data.resize(fileSize);
        std::ifstream inFile(newFilePath, std::ios::binary);
        if (!inFile.read(data.data(), data.size())) {
            LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
        }

        return SARCError::NONE;
    }

    SARCError SARCFile::rebuildFromDir(const fspath& dirPath) {
        // rebuild using the original filename list (so extraneous unpacked stuff isn't added)

        if(nameTable.filenames.empty()) {
            LOG_ERR_AND_RETURN(SARCError::UNEXPECTED_VALUE);
        }

        files.clear();

        for (const std::string& filename : nameTable.filenames) {
            const fspath absPath = dirPath / filename;

            const uint32_t fileSize = std::filesystem::file_size(absPath);

            std::string& data = files.emplace(filename, std::string(fileSize, '\0')).first->second;

            std::ifstream inFile(absPath, std::ios::binary);
            if (!inFile.read(data.data(), data.size())) {
                LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
            }
        }

        return SARCError::NONE;
    }

    SARCError SARCFile::buildFromDir(const fspath& dirPath) {
        files.clear();

        for (const auto& path : std::filesystem::recursive_directory_iterator(dirPath)) {
            if (path.is_regular_file()) {
                const fspath& absPath = path.path();

                const uint32_t fileSize = std::filesystem::file_size(absPath);

                std::string& data = files.emplace(std::filesystem::relative(absPath, dirPath).generic_string(), std::string(fileSize, '\0')).first->second;

                std::ifstream inFile(absPath, std::ios::binary);
                if (!inFile.read(data.data(), data.size())) {
                    LOG_ERR_AND_RETURN(SARCError::REACHED_EOF);
                }
            }
        }

        return SARCError::NONE;
    }
}
