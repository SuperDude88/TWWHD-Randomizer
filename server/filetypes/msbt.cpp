#include "msbt.hpp"

#include <cstring>
#include <algorithm>

#include "../utility/byteswap.hpp"
#include "../utility/common.hpp"



uint32_t LabelChecksum(uint32_t groupCount, std::string label) {
    unsigned int group = 0;

    for (unsigned int i = 0; i < label.length(); i++) {
        group = group * 0x492;
        group = group + label[i];
        group = group & 0xFFFFFFFF;
    }

    return group % groupCount;
}

MSBTError readHeader(std::istream& msbt, MSBTHeader& header) {
    if (!msbt.read(header.magicMsgStdBn, 8)) return MSBTError::REACHED_EOF;
    if (std::strncmp(header.magicMsgStdBn, "MsgStdBn", 8) != 0)
    {
        return MSBTError::NOT_MSBT;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.byteOrderMarker), sizeof(header.byteOrderMarker)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.unknown_0x00), sizeof(header.unknown_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.encoding_0x01), sizeof(header.encoding_0x01)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.version_0x03), sizeof(header.version_0x03)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.sectionCount), sizeof(header.sectionCount)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.unknown2_0x00), sizeof(header.unknown2_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.fileSize), sizeof(header.fileSize)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }

    Utility::byteswap_inplace(header.byteOrderMarker);
    Utility::byteswap_inplace(header.unknown_0x00);
    Utility::byteswap_inplace(header.sectionCount);
    Utility::byteswap_inplace(header.unknown2_0x00);
    Utility::byteswap_inplace(header.fileSize);

    if (header.unknown_0x00 != 0x0000) return MSBTError::UNEXPECTED_VALUE;
    if (header.encoding_0x01 != 0x01) return MSBTError::UNEXPECTED_VALUE;
    if (header.version_0x03 != 0x03) return MSBTError::UNKNOWN_VERSION;
    if (header.sectionCount != 0x0004) return MSBTError::UNEXPECTED_VALUE;
    if (header.unknown2_0x00 != 0x0000) return MSBTError::UNEXPECTED_VALUE;

    return MSBTError::NONE;
}

MSBTError readLBL1(std::istream& msbt, LBL1Header& header) {
    msbt.seekg(-4, std::ios::cur);
    if (!msbt.read(header.magicLBL1, 4)) return MSBTError::REACHED_EOF;
    if (std::strncmp(header.magicLBL1, "LBL1", 4) != 0)
    {
        return MSBTError::NOT_LBL1;
    }
    header.offset = (uint32_t)msbt.tellg() - 4;
    if (!msbt.read(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.entryCount), sizeof(header.entryCount)))
    {
        return MSBTError::REACHED_EOF;
    }

    Utility::byteswap_inplace(header.tableSize);
    Utility::byteswap_inplace(header.entryCount);

    header.entries.reserve(header.entryCount);
    for (uint32_t i = 0; i < header.entryCount; i++) {
        msbt.seekg(header.offset + 0x14 + i * 0x8, std::ios::beg);
        LBLEntry entry;
        if (!msbt.read(reinterpret_cast<char*>(&entry.stringCount), sizeof(entry.stringCount)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&entry.stringOffset), sizeof(entry.stringOffset)))
        {
            return MSBTError::REACHED_EOF;
        }
        Utility::byteswap_inplace(entry.stringCount);
        Utility::byteswap_inplace(entry.stringOffset);
        msbt.seekg(entry.stringOffset + header.offset + 0x10); //Seek to the start of the entries before the loop so it doesnt reset to the same string each time
        entry.labels.reserve(entry.stringCount);
        for (uint32_t x = 0; x < entry.stringCount; x++) {
            Label label;
            label.checksum = i;
            if (!msbt.read(reinterpret_cast<char*>(&label.length), sizeof(label.length)))
            {
                return MSBTError::REACHED_EOF;
            }
            label.string.resize(label.length); //Length is 1 bit so no byteswap
            if (!msbt.read(&label.string[0], label.length))
            {
                return MSBTError::REACHED_EOF;
            }
            if (!msbt.read(reinterpret_cast<char*>(&label.messageIndex), sizeof(label.messageIndex)))
            {
                return MSBTError::REACHED_EOF;
            }
            Utility::byteswap_inplace(label.messageIndex);
            entry.labels.push_back(label);
        }
        header.entries.push_back(entry);
    }


    /*
    if (msbt.tellg() % 16 != 0) {
        int padding_size = 16 - (msbt.tellg() % 16);
        char charater;
        for (int i = 16 - (msbt.tellg() % 16); i > 0; i--) {
            if (!msbt.read(&character, 1)) return MSBTError::REACHED_EOF;
            if (character != '\xab') return MSBTError::UNEXPECTED_VALUE;
        }
    }
    */
    // ^ may be a faster way to do the same thing, depends on speed of istream read

    if (msbt.tellg() % 16 != 0) {
        unsigned int padding_size = 16 - (msbt.tellg() % 16);
        std::string padding;
        padding.resize(padding_size);
        if (!msbt.read(&padding[0], padding_size)) return MSBTError::REACHED_EOF;
        for (const char& character : padding) {
            if (character != '\xab') return MSBTError::UNEXPECTED_VALUE;
        }
    }

    return MSBTError::NONE;

}

MSBTError readATR1(std::istream& msbt, ATR1Header& header) {
    msbt.seekg(-4, std::ios::cur);
    if (!msbt.read(header.magicATR1, 4)) return MSBTError::REACHED_EOF;
    if (std::strncmp(header.magicATR1, "ATR1", 4) != 0)
    {
        return MSBTError::NOT_ATR1;
    }
    header.offset = (uint32_t)msbt.tellg() - 4;
    if (!msbt.read(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.entryCount), sizeof(header.entryCount)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.entrySize), sizeof(header.entrySize)))
    {
        return MSBTError::REACHED_EOF;
    }

    Utility::byteswap_inplace(header.tableSize);
    Utility::byteswap_inplace(header.entryCount);
    Utility::byteswap_inplace(header.entrySize);

    header.entries.reserve(header.entryCount);
    for (uint32_t i = 0; i < header.entryCount; i++) {
        msbt.seekg(header.offset + 0x18 + i * header.entrySize, std::ios::beg);
        Attributes attributes;
        if (!msbt.read(reinterpret_cast<char*>(&attributes.character), sizeof(attributes.character))) //need to read fields individually, otherwise most do not get populated properly
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&attributes.boxStyle), sizeof(attributes.boxStyle)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&attributes.drawType), sizeof(attributes.drawType)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&attributes.screenPos), sizeof(attributes.screenPos)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&attributes.price), sizeof(attributes.price)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&attributes.nextNo), sizeof(attributes.nextNo)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&attributes.item), sizeof(attributes.item)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&attributes.lineAlignment), sizeof(attributes.lineAlignment)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&attributes.soundEffect), sizeof(attributes.soundEffect)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&attributes.camera), sizeof(attributes.camera)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&attributes.demoID), sizeof(attributes.demoID)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&attributes.animation), sizeof(attributes.animation)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&attributes.commentE_1), sizeof(attributes.commentE_1)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&attributes.commentE_2), sizeof(attributes.commentE_2)))
        {
            return MSBTError::REACHED_EOF;
        }

        Utility::byteswap_inplace(attributes.price);
        Utility::byteswap_inplace(attributes.nextNo);
        Utility::byteswap_inplace(attributes.demoID);
        Utility::byteswap_inplace(attributes.commentE_1);
        Utility::byteswap_inplace(attributes.commentE_2);

        header.entries.push_back(attributes);
    }

    msbt.seekg(header.offset + 0x10 + header.tableSize, std::ios::beg);
    if (msbt.tellg() % 16 != 0) {
        unsigned int padding_size = 16 - (msbt.tellg() % 16);
        std::string padding;
        padding.resize(padding_size);
        if (!msbt.read(&padding[0], padding_size)) return MSBTError::REACHED_EOF;
        for (const char& character : padding) {
            if (character != '\xab') return MSBTError::UNEXPECTED_VALUE;
        }
    }

    return MSBTError::NONE;

}

MSBTError readTSY1(std::istream& msbt, TSY1Header& header) {
    msbt.seekg(-4, std::ios::cur);
    if (!msbt.read(header.magicTSY1, 4)) return MSBTError::REACHED_EOF;
    if (std::strncmp(header.magicTSY1, "TSY1", 4) != 0)
    {
        return MSBTError::NOT_ATR1;
    }
    header.offset = (uint32_t)msbt.tellg() - 4;
    if (!msbt.read(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }

    Utility::byteswap_inplace(header.tableSize);

    header.entries.reserve((header.tableSize + 0x10U + header.offset) / 4);
    for (uint32_t i = header.offset + 0x10U; i < (header.tableSize + 0x10U + header.offset); i += 4) {
        msbt.seekg(i, std::ios::beg);
        TSY1Entry entry;
        if (!msbt.read(reinterpret_cast<char*>(&entry.styleIndex), sizeof(entry.styleIndex)))
        {
            return MSBTError::REACHED_EOF;
        }

        Utility::byteswap_inplace(entry.styleIndex);

        header.entries.push_back(entry);
    }

    if (msbt.tellg() % 16 != 0) {
        unsigned int padding_size = 16 - (msbt.tellg() % 16);
        std::string padding;
        padding.resize(padding_size);
        if (!msbt.read(&padding[0], padding_size)) return MSBTError::REACHED_EOF;
        for (const char& character : padding) {
            if (character != '\xab') return MSBTError::UNEXPECTED_VALUE;
        }
    }

    return MSBTError::NONE;

}

MSBTError readTXT2(std::istream& msbt, TXT2Header& header) {
    msbt.seekg(-4, std::ios::cur);
    if (!msbt.read(header.magicTXT2, 4)) return MSBTError::REACHED_EOF;
    if (std::strncmp(header.magicTXT2, "TXT2", 4) != 0)
    {
        return MSBTError::NOT_ATR1;
    }
    header.offset = (uint32_t)msbt.tellg() - 4;
    if (!msbt.read(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00)))
    {
        return MSBTError::REACHED_EOF;
    }
    if (!msbt.read(reinterpret_cast<char*>(&header.entryCount), sizeof(header.entryCount)))
    {
        return MSBTError::REACHED_EOF;
    }

    Utility::byteswap_inplace(header.tableSize);
    Utility::byteswap_inplace(header.entryCount);

    header.entries.reserve(header.entryCount);
    for (uint32_t i = 0; i < header.entryCount; i++) {
        msbt.seekg(header.offset + 0x14 + i * 0x4); //0x14 comes from the header info size
        TXT2Entry entry;
        if (!msbt.read(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset)))
        {
            return MSBTError::REACHED_EOF;
        }
        if (!msbt.read(reinterpret_cast<char*>(&entry.nextOffset), sizeof(entry.nextOffset)))
        {
            return MSBTError::REACHED_EOF;
        }

        Utility::byteswap_inplace(entry.offset);
        Utility::byteswap_inplace(entry.nextOffset);

        msbt.seekg(header.offset + 0x10 + entry.offset); //Offsets are relative to the "end" of the header, 4 bytes before the offset data starts (add 0x10 instead of 0x14)
        uint32_t length;
        if (i + 1 == header.entryCount) { //Check if the index is the last in the file
            length = header.tableSize - entry.offset;
            entry.nextOffset = header.tableSize;
        }
        else {
            length = entry.nextOffset - entry.offset;
        }
        entry.message.resize(length / 2); //length is bytes, 2 bytes per char
        if (!msbt.read((char*)&entry.message[0], length))
        {
            return MSBTError::REACHED_EOF;
        }

        header.entries.push_back(entry);
    }

    if (msbt.tellg() % 16 != 0) {
        unsigned int padding_size = 16 - (msbt.tellg() % 16);
        std::string padding;
        padding.resize(padding_size);
        if (!msbt.read(&padding[0], padding_size)) return MSBTError::REACHED_EOF;
        for (const char& character : padding) {
            if (character != '\xab') return MSBTError::UNEXPECTED_VALUE;
        }
    }

    return MSBTError::NONE;

}

void writeLBL1(std::ostream& out, LBL1Header& header) {
    header.offset = out.tellp();
    Utility::byteswap_inplace(header.tableSize);
    Utility::byteswap_inplace(header.entryCount);

    out.write(header.magicLBL1, 4);
    out.write(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize));
    out.write(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00));
    out.write(reinterpret_cast<char*>(&header.entryCount), sizeof(header.entryCount));

    int i = 0;
    for (LBLEntry& entry : header.entries) {
        out.seekp(header.offset + 0x14 + 0x8 * i, std::ios::beg);
        Utility::byteswap_inplace(entry.stringCount); //byteswap inplace here and swap back later so all the swaps can be grouped in 1 spot instead of in each write (allows for easier Wii U conversion later)
        Utility::byteswap_inplace(entry.stringOffset);

        out.write(reinterpret_cast<char*>(&entry.stringCount), sizeof(entry.stringCount));
        out.write(reinterpret_cast<char*>(&entry.stringOffset), sizeof(entry.stringOffset));
        std::sort(entry.labels.begin(), entry.labels.end(), [](const Label& a, const Label& b) {
            int IDa = std::stoi(a.string), IDb = std::stoi(b.string);
            return IDa < IDb;
        });
        Utility::byteswap_inplace(entry.stringOffset);
        out.seekp(header.offset + 0x10 + entry.stringOffset, std::ios::beg);
        for (Label& label : entry.labels) {
            Utility::byteswap_inplace(label.messageIndex);

            label.length = label.string.size();
            out.write(reinterpret_cast<char*>(&label.length), sizeof(label.length));
            out.write(&label.string[0], label.length);
            out.write(reinterpret_cast<char*>(&label.messageIndex), 4);
        }
        i = i + 1;
    }

    padToLen(out, 16, '\xab');

    return;

}

void writeATR1(std::ostream& out, ATR1Header& header) {
    header.offset = out.tellp();
    Utility::byteswap_inplace(header.tableSize);
    Utility::byteswap_inplace(header.entryCount);
    Utility::byteswap_inplace(header.entrySize);

    out.write(header.magicATR1, 4);
    out.write(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize));
    out.write(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00));
    out.write(reinterpret_cast<char*>(&header.entryCount), sizeof(header.entryCount));
    out.write(reinterpret_cast<char*>(&header.entrySize), sizeof(header.entrySize));

    for (Attributes& attributes : header.entries) {
        Utility::byteswap_inplace(attributes.price);
        Utility::byteswap_inplace(attributes.nextNo);
        Utility::byteswap_inplace(attributes.demoID);
        Utility::byteswap_inplace(attributes.commentE_1);
        Utility::byteswap_inplace(attributes.commentE_2);

        out.write(reinterpret_cast<char*>(&attributes.character), sizeof(attributes.character));
        out.write(reinterpret_cast<char*>(&attributes.boxStyle), sizeof(attributes.boxStyle));
        out.write(reinterpret_cast<char*>(&attributes.drawType), sizeof(attributes.drawType));
        out.write(reinterpret_cast<char*>(&attributes.screenPos), sizeof(attributes.screenPos));
        out.write(reinterpret_cast<char*>(&attributes.price), sizeof(attributes.price));
        out.write(reinterpret_cast<char*>(&attributes.nextNo), sizeof(attributes.nextNo));
        out.write(reinterpret_cast<char*>(&attributes.item), sizeof(attributes.item));
        out.write(reinterpret_cast<char*>(&attributes.lineAlignment), sizeof(attributes.lineAlignment));
        out.write(reinterpret_cast<char*>(&attributes.soundEffect), sizeof(attributes.soundEffect));
        out.write(reinterpret_cast<char*>(&attributes.camera), sizeof(attributes.camera));
        out.write(reinterpret_cast<char*>(&attributes.demoID), sizeof(attributes.demoID));
        out.write(reinterpret_cast<char*>(&attributes.animation), sizeof(attributes.animation));
        out.write(reinterpret_cast<char*>(&attributes.commentE_1), sizeof(attributes.commentE_1));
        out.write(reinterpret_cast<char*>(&attributes.commentE_2), sizeof(attributes.commentE_2));
    }

    padToLen(out, 16, '\xab');

    return;

}

void writeTSY1(std::ostream& out, TSY1Header& header) {
    header.offset = out.tellp();
    Utility::byteswap_inplace(header.tableSize);

    out.write(header.magicTSY1, 4);
    out.write(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize));
    out.write(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00));

    for (TSY1Entry& entry : header.entries) {
        Utility::byteswap_inplace(entry.styleIndex);

        out.write(reinterpret_cast<char*>(&entry.styleIndex), sizeof(entry.styleIndex));
    }

    padToLen(out, 16, '\xab');

    return;

}

void writeTXT2(std::ostream& out, TXT2Header& header) {
    header.offset = out.tellp();
    Utility::byteswap_inplace(header.tableSize);
    Utility::byteswap_inplace(header.entryCount);

    out.write(header.magicTXT2, 4);
    out.write(reinterpret_cast<char*>(&header.tableSize), sizeof(header.tableSize));
    out.write(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00));
    out.write(reinterpret_cast<char*>(&header.entryCount), sizeof(header.entryCount));

    for (TXT2Entry& entry : header.entries) {
        Utility::byteswap_inplace(entry.offset);

        out.write(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset));
    }

    for (TXT2Entry& entry : header.entries) { //Instead of looping through stuff with indexes and seeking back and forth, we loop through all the header and offset table data and then write the strings
        out.write((char*)&entry.message[0], entry.message.size() * 2); //double the size because it is the number of 2-byte chars to write, but functions treats them as 1-byte
    }

    padToLen(out, 16, '\xab');

    return;

}

namespace FileTypes {

    const char* MSBTErrorGetName(MSBTError err) {
        switch (err) {
            case MSBTError::NONE:
                return "NONE";
            case MSBTError::COULD_NOT_OPEN:
                return "COULD_NOT_OPEN";
            case MSBTError::NOT_MSBT:
                return "NOT_MSBT";
            case MSBTError::UNKNOWN_VERSION:
                return "UNKNOWN_VERSION";
            case MSBTError::UNEXPECTED_VALUE:
                return "UNEXPECTED_VALUE";
            case MSBTError::UNKNOWN_SECTION:
                return "UNKNOWN_SECTION";
            case MSBTError::NOT_LBL1:
                return "NOT_LBL1";
            case MSBTError::NOT_ATR1:
                return "NOT_ATR1";
            case MSBTError::NOT_TSY1:
                return "NOT_TSY1";
            case MSBTError::NOT_TXT2:
                return "NOT_TXT2";
            case MSBTError::REACHED_EOF:
                return "REACHED_EOF";
            default:
                return "UNKNOWN";
        }
    }

    MSBTFile::MSBTFile() {
        
    }

    void MSBTFile::initNew() {
        memcpy(&header.magicMsgStdBn, "MsgStdBn", 8);
        header.byteOrderMarker = 0xFEFF;
        header.unknown_0x00 = 0;
        header.encoding_0x01 = 0x01;
        header.version_0x03 = 0x03;
        header.sectionCount = 4;
        header.unknown2_0x00 = 0;
        header.fileSize = 0;
        memset(&header.padding_0x00, '\0', 10);
        LBL1.offset = 0;
        memcpy(LBL1.magicLBL1, "LBL1", 4);
        LBL1.tableSize = 0;
        memset(&LBL1.padding_0x00, '\0', 8);
        LBL1.entryCount = 0;
        LBL1.entries = {};
        ATR1.offset = 0;
        memcpy(&ATR1.magicATR1, "ATR1", 4);
        ATR1.tableSize = 0;
        memset(&ATR1.padding_0x00, '\0', 8);
        ATR1.entryCount = 0;
        ATR1.entrySize = 0x17; //WWHD always uses this for its entry size
        ATR1.entries = {};
        TSY1.offset = 0;
        memcpy(&TSY1.magicTSY1, "TSY1", 4);
        TSY1.tableSize = 0;
        memset(&TSY1.padding_0x00, '\0', 8);
        TSY1.entries = {};
        TXT2.offset = 0;
        memcpy(&TXT2.magicTXT2, "TXT2", 4);
        TXT2.tableSize = 0;
        memset(&TXT2.padding_0x00, '\0', 8);
        TXT2.entryCount = 0;
        TXT2.entries = {};
        messages_by_label = {};
    }

    MSBTFile MSBTFile::createNew(const std::string& filename) {
        MSBTFile newMSBT{};
        newMSBT.initNew();
        return newMSBT;
    }

    MSBTError MSBTFile::readSection(std::istream& msbt) {
        char magic[4];
        MSBTError error = MSBTError::NONE;
        if (!msbt.read(magic, 4)) return MSBTError::REACHED_EOF;
        if (std::strncmp(magic, "LBL1", 4) == 0) {
            error = readLBL1(msbt, LBL1);
            if (error != MSBTError::NONE) return error;
        }
        else if (std::strncmp(magic, "ATR1", 4) == 0) {
            error = readATR1(msbt, ATR1);
            if (error != MSBTError::NONE) return error;
        }
        else if (std::strncmp(magic, "TSY1", 4) == 0) {
            error = readTSY1(msbt, TSY1);
            if (error != MSBTError::NONE) return error;
        }
        else if (std::strncmp(magic, "TXT2", 4) == 0) {
            error = readTXT2(msbt, TXT2);
            if (error != MSBTError::NONE) return error;
        }

        return MSBTError::NONE;

    }

    MSBTError MSBTFile::loadFromBinary(std::istream& msbt) {
        MSBTError error = MSBTError::NONE;

        error = readHeader(msbt, header);
        if (error != MSBTError::NONE) return error;

        for (uint16_t i = 0; i < header.sectionCount; i++) {
            error = readSection(msbt);
            if (error != MSBTError::NONE) return error;
        }

        for (const LBLEntry& entry : LBL1.entries) {
            for (const Label& label : entry.labels) { //Populate map of messages
                Message msg;
                msg.label = label;
                msg.attributes = ATR1.entries[label.messageIndex];
                msg.style = TSY1.entries[label.messageIndex];
                msg.text = TXT2.entries[label.messageIndex];
                messages_by_label[label.string] = msg;
            }
        }

        return MSBTError::NONE;
    }

    MSBTError MSBTFile::loadFromFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return MSBTError::COULD_NOT_OPEN;
        }
        return loadFromBinary(file);
    }

    Message& MSBTFile::addMessage(const std::string& label, const Attributes& attributes, const TSY1Entry& style, const std::u16string& message) {
        Message newMessage;

        Label newLabel;
        newLabel.checksum = LabelChecksum(LBL1.entryCount, label); //Entry count is always 0x65 for the main text blocks
        newLabel.length = label.size();
        newLabel.string = label;
        newLabel.messageIndex = TXT2.entryCount; //Number of entries = index of message 1 past the end of existing list

        TXT2Entry newEntry;
        newEntry.message = message;

        newMessage.label = newLabel;
        newMessage.attributes = attributes;
        newMessage.style = style;
        newMessage.text = newEntry;
        messages_by_label[newLabel.string] = newMessage;

        return messages_by_label[newLabel.string];
    }

    MSBTError MSBTFile::writeToStream(std::ostream& out) {

        Utility::byteswap_inplace(header.byteOrderMarker);
        Utility::byteswap_inplace(header.unknown_0x00);
        Utility::byteswap_inplace(header.sectionCount);
        Utility::byteswap_inplace(header.unknown2_0x00);

        out.write(header.magicMsgStdBn, 8);
        out.write((char*)&header.byteOrderMarker, sizeof(header.byteOrderMarker));
        out.write((char*)&header.unknown_0x00, sizeof(header.unknown_0x00));
        out.write((char*)&header.encoding_0x01, sizeof(header.encoding_0x01));
        out.write((char*)&header.version_0x03, sizeof(header.version_0x03));
        out.write((char*)&header.sectionCount, sizeof(header.sectionCount));
        out.write((char*)&header.unknown2_0x00, sizeof(header.unknown2_0x00));
        out.seekp(4, std::ios::cur); //skip filesize for now
        out.write((char*)&header.padding_0x00, sizeof(header.padding_0x00));

        //Go through and update all the sections based on the messages by ID
        LBLEntry temp; //Fill all the entries with blank entries
        std::fill(LBL1.entries.begin(), LBL1.entries.end(), temp);
        for (LBLEntry& entry : LBL1.entries) { //Clear each entry's list of labels
            entry.labels.clear();
        }
        ATR1.entries.resize(messages_by_label.size()); //Make sure these are full size, the file relies on indexes a bunch so we need to replace the right indexes in the list (labels store indexes, they're different)
        TSY1.entries.resize(messages_by_label.size());
        TXT2.entries.resize(messages_by_label.size());

        for (const auto& [label, message] : messages_by_label) {
            LBL1.entries[message.label.checksum].stringCount += 1;
            LBL1.entries[message.label.checksum].labels.push_back(message.label);
            ATR1.entries[message.label.messageIndex] = message.attributes;
            TSY1.entries[message.label.messageIndex] = message.style;
            TXT2.entries[message.label.messageIndex] = message.text;
        }

        LBL1.tableSize = LBL1.entryCount * 0x8;
        uint32_t nextGroupOffset = LBL1.entryCount * 0x8 + 0x4; //First entry starts after the table so they start here, 0x4 extra for the entry count
        for (LBLEntry& entry : LBL1.entries) {
            entry.stringCount = entry.labels.size();
            entry.stringOffset = nextGroupOffset;
            for (Label& label : entry.labels) {
                nextGroupOffset = nextGroupOffset + label.string.size() + 0x5; //loop through the labels in the group and add their length for the next group offset
                LBL1.tableSize = LBL1.tableSize + label.string.size() + 0x5; //Add entry lengths to the table length
            }
        }

        ATR1.entryCount = ATR1.entries.size();
        ATR1.tableSize = ATR1.entryCount * ATR1.entrySize + 0x8; //Table size includes the 8 bytes for entry count + size

        TSY1.tableSize = TSY1.entries.size() * 0x4;

        TXT2.entryCount = TXT2.entries.size();
        uint32_t nextOffset = TXT2.entryCount * 0x4 + 0x4; //first offset = offset for each entry + the number of entries value (counts as part of the offset table)
        for (TXT2Entry& entry : TXT2.entries) {
            entry.offset = nextOffset;
            entry.nextOffset = entry.offset + (entry.message.size() * 2); //size x2 because it returns the number of utf-16 chars, but file offsets need byte counts
            nextOffset = entry.nextOffset;
        }
        TXT2.tableSize = TXT2.entries.back().nextOffset;

        writeLBL1(out, LBL1);
        writeATR1(out, ATR1);
        writeTSY1(out, TSY1);
        writeTXT2(out, TXT2);

        out.seekp(0, std::ios::end);
        header.fileSize = out.tellp();
        out.seekp(0x12, std::ios::beg);

        uint32_t fileSize_BE = Utility::byteswap(header.fileSize);
        out.write((char*)&fileSize_BE, sizeof(fileSize_BE)); //Update full file size

        return MSBTError::NONE;
    }

    MSBTError MSBTFile::writeToFile(const std::string& outFilePath) {
        std::ofstream outFile(outFilePath, std::ios::binary);
        if (!outFile.is_open()) {
            return MSBTError::COULD_NOT_OPEN;
        }
        return writeToStream(outFile);
    }

}
