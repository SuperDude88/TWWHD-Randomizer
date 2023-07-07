#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>

//LMS is LibMessageStudio, Nintendo's small library for message data
//Includes MSBT, MSBP, and MSBF files (only MSBT and MSBP are present in WWHD)

enum struct [[nodiscard]] LMSError
{
    NONE = 0,
    REACHED_EOF,
    COULD_NOT_OPEN,
    NOT_MSBT,
    NOT_MSBP,
    UNKNOWN_VERSION,
    UNEXPECTED_VALUE,
    UNKNOWN_SECTION,
    UNKNOWN,
    COUNT
};

class FileHeader {
public:
    char magic[8];
    uint16_t byteOrderMarker;
    uint16_t unknown_0x00;
    uint8_t encoding;
    uint8_t version_0x03;
    uint16_t sectionCount;
    uint16_t unknown2_0x00;
    uint32_t fileSize;
    uint8_t padding_0x00[10];

    virtual ~FileHeader() = default;
    virtual LMSError read(std::istream& in);
    virtual void write(std::ostream& out);
};

class SectionHeader {
public:
    char magic[4];
    uint32_t sectionSize;
    
    virtual ~SectionHeader() = default;
    virtual LMSError read(std::istream& in);
    virtual void write(std::ostream& out);
private:
    uint8_t padding_0x00[8];
};

struct Label {
    uint32_t tableIdx = 0;

    uint8_t length = 0;
    std::string string = "";
    uint32_t itemIndex = 0;
};

struct HashTableSlot {
    uint32_t labelCount = 0;
    uint32_t labelOffset = 0;
    std::vector<Label> labels = {};
};

class HashTable {
public:
    uint32_t entryCount = 0;
    std::vector<HashTableSlot> tableSlots = {}; // slots variable name conflicts with Qt slots keyword

    virtual ~HashTable() = default;
    virtual LMSError read(std::istream& in);
    virtual void write(std::ostream& out);
};

namespace LMS {
    inline uint32_t calcLabelHash(const uint32_t& groupCount, const std::string& label) {
        uint32_t group = 0;

        for (uint32_t i = 0; i < label.length(); i++) {
            group = group * 0x492;
            group = group + label[i];
            group = group & 0xFFFFFFFF;
        }

        return group % groupCount;
    }
}

namespace FileTypes {
    const char* LMSErrorGetName(LMSError err);
}
