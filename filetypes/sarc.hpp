// Format is part of sead (Nintendo EAD's internal "standard" library)
// SARC files are archives that store various types of data

#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <string>

#include <filetypes/baseFiletype.hpp>



enum struct [[nodiscard]] SARCError {
    NONE = 0,
    COULD_NOT_OPEN,
    NOT_SARC,
    UNKNOWN_VERSION,
    REACHED_EOF,
    BAD_NODE_ATTR,
    STRING_NOT_FOUND,
    FILENAME_HASH_MISMATCH,
    UNEXPECTED_VALUE,
    UNKNOWN,
    COUNT
};

struct SARCHeader {
    char magicSARC[4] = {'S', 'A', 'R', 'C'};
    uint16_t headerSize_0x14 = 0x14;
    uint16_t byteOrderMarker = 0xFEFF;
    uint32_t fileSize = 0;
    uint32_t dataOffset = 0;
    uint16_t version_0x0100 = 0x0100;
    uint8_t padding_0x00[2] = {0, 0};
};

struct SFATNode {
    uint32_t nameHash = 0;
    uint32_t attributes = 0;
    uint32_t dataStart = 0; // relative to data section start
    uint32_t dataEnd = 0; // relative to data section start
};

struct SFAT {
    char magicSFAT[4] = {'S', 'F', 'A', 'T'};
    uint16_t headerSize_0xC = 0xC;
    uint16_t numFiles = 0;
    uint32_t hashKey_0x65 = 0x65;

    std::vector<SFATNode> nodes;
};

struct SFNT {
    char magicSFNT[4] = {'S', 'F', 'N', 'T'};
    uint16_t headerSize_0x8 = 0x8;
    uint8_t padding_0x00[2] = {0, 0};

    std::vector<std::string> filenames;
};

namespace FileTypes {
    const char* SARCErrorGetName(SARCError err);

    class SARCFile final : public FileType {
    private:
        template<typename Comparator = std::less<>>
        struct HashCompare {
            bool operator()(const std::string& lhs, const std::string& rhs) const {
                return Comparator{}(calculateHash(lhs), calculateHash(rhs));
            }
        };

    public:
        std::map<std::string, std::string, HashCompare<>> files; // sorted by name hash

        SARCFile() = default;
        static SARCFile createNew();
        SARCError loadFromBinary(std::istream& sarc);
        SARCError loadFromFile(const fspath& filePath);
        std::string* getFile(const std::string& filename);
        SARCError writeToStream(std::ostream& out);
        SARCError writeToFile(const fspath& outFilePath);
        SARCError extractToDir(const fspath& dirPath) const;
        SARCError replaceFile(const std::string& filename, const fspath& newFilePath);
        SARCError rebuildFromDir(const fspath& dirPath);
        SARCError buildFromDir(const fspath& dirPath);
    private:
        SARCHeader header;
        SFAT fileTable;
        SFNT nameTable;

        static uint32_t calculateHash(const std::string& name, const uint32_t multiplier = 0x65) {
            uint32_t hash = 0;
            for (const int8_t byte : name) {
                if (byte == 0x00) break; // string is null-terminated
                hash = hash * multiplier + byte;
            }
        
            return hash;
        }

        void initNew() override;
    };
}
