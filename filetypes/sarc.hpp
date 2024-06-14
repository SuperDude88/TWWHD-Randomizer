//Format is part of sead (Nintendo's standard library)
//SARC files are archives that store various types of data

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <filetypes/baseFiletype.hpp>



enum struct [[nodiscard]] SARCError {
    NONE = 0,
    COULD_NOT_OPEN,
    NOT_SARC,
    UNKNOWN_VERSION,
    REACHED_EOF,
    STRING_TOO_LONG,
    BAD_NODE_ATTR,
    STRING_NOT_FOUND,
    FILENAME_HASH_MISMATCH,
    UNEXPECTED_VALUE,
    UNKNOWN,
    COUNT
};

struct SARCHeader {
    char magicSARC[4];
    uint16_t headerSize_0x14;
    uint16_t byteOrderMarker;
    uint32_t fileSize;
    uint32_t dataOffset;
    uint16_t version_0x0100;
    uint8_t padding_0x00[2];
};

struct SFATNode {
    uint32_t nameHash;
    uint32_t attributes;
    uint32_t dataStart; //relative to data section start
    uint32_t dataEnd; //relative to data section start
};

struct SFAT {
    char magicSFAT[4];
    uint16_t headerSize_0xC;
    uint16_t numFiles;
    uint32_t hashKey_0x65;

    std::vector<SFATNode> nodes;
};

struct SFNT {
    char magicSFNT[4];
    uint16_t headerSize_0x8;
    uint8_t padding_0x00[2];

    std::vector<std::string> filenames;
};

namespace FileTypes {
    const char* SARCErrorGetName(SARCError err);

    class SARCFile final : public FileType {
    public:
        struct File {
            std::string name;
            std::string data;
        };

        SARCFile() = default;
        static SARCFile createNew();
        SARCError loadFromBinary(std::istream& sarc);
        SARCError loadFromFile(const std::string& filePath);
        File* getFile(const std::string& filename);
        SARCError writeToStream(std::ostream& out);
        SARCError writeToFile(const std::string& outFilePath);
        SARCError extractToDir(const std::string& dirPath) const;
        SARCError replaceFile(const std::string& filename, const std::stringstream& newData);
        SARCError replaceFile(const std::string& filename, const std::string& newFilePath);
        SARCError rebuildFromDir(const std::string& dirPath);
        SARCError buildFromDir(const std::string& dirPath); //partly untested, should work though
    private:
        SARCHeader header;
        SFAT fileTable;
        SFNT nameTable;
        std::unordered_map<std::string, size_t> file_index_by_name;
        std::vector<File> files; //store as vector to keep insertion order
        uint32_t guessed_alignment;

        void initNew() override;
        void guessDefaultAlignment();
    };
}
