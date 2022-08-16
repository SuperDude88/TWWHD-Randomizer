#include "msbt.hpp"

#include <cstring>
#include <algorithm>

#include "../utility/endian.hpp"
#include "../utility/common.hpp"
#include "../command/Log.hpp"

#include "../utility/platform.hpp"

using eType = Utility::Endian::Type;

LMSError MSBTHeader::read(std::istream& in) {
    LOG_AND_RETURN_IF_ERR(FileHeader::read(in));
    if (std::strncmp(magic, "MsgStdBn", 8) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::NOT_MSBT);
    }
    if (encoding != 0x01) LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);
    if (sectionCount != 0x0004) {
        LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);
    }

    return LMSError::NONE;
}

LMSError LBL1::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "LBL1", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }
    LOG_AND_RETURN_IF_ERR(HashTable::read(in));

    if (in.tellg() % 16 != 0) {
        const unsigned int padding_size = 16 - (in.tellg() % 16);
        std::string padding(padding_size, '\0');
        if (!in.read(&padding[0], padding_size)) LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        for (const char& character : padding) {
            if (character != '\xab') LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);
        }
    }

    return LMSError::NONE;
}

void LBL1::write(std::ostream &out) {
    SectionHeader::write(out);
    HashTable::write(out);
    padToLen(out, 16, '\xab');

    return;
}

LMSError ATR1::read(std::istream &in) {
    const std::streamoff sectionStart = in.tellg();

    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "ATR1", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }
    if (!in.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    if (!in.read(reinterpret_cast<char*>(&entrySize), sizeof(entrySize)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }

    Utility::Endian::toPlatform_inplace(eType::Big, entryCount);
    Utility::Endian::toPlatform_inplace(eType::Big, entrySize);

    if(entrySize != 0x00000017) LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);

    entries.reserve(entryCount);
    for (uint32_t i = 0; i < entryCount; i++) {
        Attributes& attributes = entries.emplace_back();
        if (!in.read(reinterpret_cast<char*>(&attributes.character), sizeof(attributes.character))) //need to read fields individually, otherwise most do not get populated properly
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attributes.boxStyle), sizeof(attributes.boxStyle)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attributes.drawType), sizeof(attributes.drawType)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attributes.screenPos), sizeof(attributes.screenPos)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attributes.price), sizeof(attributes.price)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attributes.nextNo), sizeof(attributes.nextNo)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attributes.item), sizeof(attributes.item)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attributes.lineAlignment), sizeof(attributes.lineAlignment)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attributes.soundEffect), sizeof(attributes.soundEffect)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attributes.camera), sizeof(attributes.camera)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attributes.demoID), sizeof(attributes.demoID)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attributes.animation), sizeof(attributes.animation)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attributes.commentE_1), sizeof(attributes.commentE_1)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attributes.commentE_2), sizeof(attributes.commentE_2)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, attributes.price);
        Utility::Endian::toPlatform_inplace(eType::Big, attributes.nextNo);
        Utility::Endian::toPlatform_inplace(eType::Big, attributes.demoID);
        Utility::Endian::toPlatform_inplace(eType::Big, attributes.commentE_1);
        Utility::Endian::toPlatform_inplace(eType::Big, attributes.commentE_2);
    }

    in.seekg(sectionStart + 0x10 + sectionSize, std::ios::beg);
    if (in.tellg() % 16 != 0) {
        unsigned int padding_size = 16 - (in.tellg() % 16);
        std::string padding(padding_size, '\0');
        if (!in.read(&padding[0], padding_size)) LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        for (const char& character : padding) {
            if (character != '\xab') LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);
        }
    }

    return LMSError::NONE;
}

void ATR1::write(std::ostream &out) {
    SectionHeader::write(out);

    entryCount = entries.size();
    Utility::Endian::toPlatform_inplace(eType::Big, entryCount);
    Utility::Endian::toPlatform_inplace(eType::Big, entrySize);

    out.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));
    out.write(reinterpret_cast<const char*>(&entrySize), sizeof(entrySize));

    for (Attributes& attributes : entries) {
        Utility::Endian::toPlatform_inplace(eType::Big, attributes.price);
        Utility::Endian::toPlatform_inplace(eType::Big, attributes.nextNo);
        Utility::Endian::toPlatform_inplace(eType::Big, attributes.demoID);
        Utility::Endian::toPlatform_inplace(eType::Big, attributes.commentE_1);
        Utility::Endian::toPlatform_inplace(eType::Big, attributes.commentE_2);

        out.write(reinterpret_cast<const char*>(&attributes.character), sizeof(attributes.character));
        out.write(reinterpret_cast<const char*>(&attributes.boxStyle), sizeof(attributes.boxStyle));
        out.write(reinterpret_cast<const char*>(&attributes.drawType), sizeof(attributes.drawType));
        out.write(reinterpret_cast<const char*>(&attributes.screenPos), sizeof(attributes.screenPos));
        out.write(reinterpret_cast<const char*>(&attributes.price), sizeof(attributes.price));
        out.write(reinterpret_cast<const char*>(&attributes.nextNo), sizeof(attributes.nextNo));
        out.write(reinterpret_cast<const char*>(&attributes.item), sizeof(attributes.item));
        out.write(reinterpret_cast<const char*>(&attributes.lineAlignment), sizeof(attributes.lineAlignment));
        out.write(reinterpret_cast<const char*>(&attributes.soundEffect), sizeof(attributes.soundEffect));
        out.write(reinterpret_cast<const char*>(&attributes.camera), sizeof(attributes.camera));
        out.write(reinterpret_cast<const char*>(&attributes.demoID), sizeof(attributes.demoID));
        out.write(reinterpret_cast<const char*>(&attributes.animation), sizeof(attributes.animation));
        out.write(reinterpret_cast<const char*>(&attributes.commentE_1), sizeof(attributes.commentE_1));
        out.write(reinterpret_cast<const char*>(&attributes.commentE_2), sizeof(attributes.commentE_2));
    }

    padToLen(out, 16, '\xab');

    return;
}

LMSError TSY1::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "TSY1", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }

    entries.reserve(sectionSize / 4);
    for (uint32_t i = 0; i < (sectionSize / 4); i++) {
        TSY1Entry& entry = entries.emplace_back();
        if (!in.read(reinterpret_cast<char*>(&entry.styleIndex), sizeof(entry.styleIndex)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, entry.styleIndex);
    }

    if (in.tellg() % 16 != 0) {
        unsigned int padding_size = 16 - (in.tellg() % 16);
        std::string padding(padding_size, '\0');
        if (!in.read(&padding[0], padding_size)) LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        for (const char& character : padding) {
            if (character != '\xab') LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);
        }
    }

    return LMSError::NONE;
}

void TSY1::write(std::ostream &out) {
    SectionHeader::write(out);
    for (TSY1Entry& entry : entries) {
        Utility::Endian::toPlatform_inplace(eType::Big, entry.styleIndex);

        out.write(reinterpret_cast<const char*>(&entry.styleIndex), sizeof(entry.styleIndex));
    }

    padToLen(out, 16, '\xab');

    return;
}

LMSError TXT2::read(std::istream &in) {
    const std::streamoff sectionStart = in.tellg();
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "TXT2", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }
    if (!in.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }

    Utility::Endian::toPlatform_inplace(eType::Big, entryCount);

    entries.reserve(entryCount);
    for (uint32_t i = 0; i < entryCount; i++) {
        in.seekg(sectionStart + 0x14 + i * 0x4);
        TXT2Entry& entry = entries.emplace_back();
        if (!in.read(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&entry.nextOffset), sizeof(entry.nextOffset)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, entry.offset);
        Utility::Endian::toPlatform_inplace(eType::Big, entry.nextOffset);

        //can't use null-terminated string read, some commands include null characters that would break things
        in.seekg(sectionStart + 0x10 + entry.offset);
        uint32_t length;
        if (i + 1 == entryCount) { //Check if the index is the last in the file
            length = sectionSize - entry.offset;
            entry.nextOffset = sectionSize;
        }
        else {
            length = entry.nextOffset - entry.offset;
        }
        entry.message.resize(length / 2); //length is bytes, 2 bytes per char
        if (!in.read(reinterpret_cast<char*>(&entry.message[0]), length))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, entry.message);
    }

    if (in.tellg() % 16 != 0) {
        unsigned int padding_size = 16 - (in.tellg() % 16);
        std::string padding(padding_size, '\0');
        if (!in.read(&padding[0], padding_size)) LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        for (const char& character : padding) {
            if (character != '\xab') LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);
        }
    }

    return LMSError::NONE;
}

void TXT2::write(std::ostream &out) {
    SectionHeader::write(out);

    entryCount = entries.size();
    Utility::Endian::toPlatform_inplace(eType::Big, entryCount);
    out.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));

    for (TXT2Entry& entry : entries) { //Loop through all the header and offset table data, then write the strings
        Utility::Endian::toPlatform_inplace(eType::Big, entry.offset);
        out.write(reinterpret_cast<const char*>(&entry.offset), sizeof(entry.offset));
    }

    for (TXT2Entry& entry : entries) { //Write strings
        Utility::Endian::toPlatform_inplace(eType::Big, entry.message);
        out.write(reinterpret_cast<const char*>(&entry.message[0]), entry.message.size() * 2); //size() returns number of 2-byte chars, function needs bytes total
    }

    padToLen(out, 16, '\xab');

    return;
}

namespace FileTypes {
    MSBTFile::MSBTFile() {
        
    }

    void MSBTFile::initNew() {
        memcpy(&header.magic, "MsgStdBn", 8);
        header.byteOrderMarker = 0xFEFF;
        header.unknown_0x00 = 0;
        header.encoding = 0x01;
        header.version_0x03 = 0x03;
        header.sectionCount = 4;
        header.unknown2_0x00 = 0;
        header.fileSize = 0;
        memset(&header.padding_0x00, '\0', 10);
        memcpy(labels.magic, "LBL1", 4);
        labels.sectionSize = 0;
        labels.tableSlots = {};
        memcpy(&attributes.magic, "ATR1", 4);
        attributes.sectionSize = 0;
        attributes.entries = {};
        memcpy(&styles.magic, "TSY1", 4);
        styles.sectionSize = 0;
        styles.entries = {};
        memcpy(&text.magic, "TXT2", 4);
        text.sectionSize = 0;
        text.entries = {};
        messages_by_label = {};
    }

    MSBTFile MSBTFile::createNew(const std::string& filename) {
        MSBTFile newMSBT{};
        newMSBT.initNew();
        return newMSBT;
    }

    LMSError MSBTFile::loadFromBinary(std::istream& msbt) {
        LOG_AND_RETURN_IF_ERR(header.read(msbt));

        LOG_AND_RETURN_IF_ERR(labels.read(msbt));
        LOG_AND_RETURN_IF_ERR(attributes.read(msbt));
        LOG_AND_RETURN_IF_ERR(styles.read(msbt));
        LOG_AND_RETURN_IF_ERR(text.read(msbt));

        for (const HashTableSlot& slot : labels.tableSlots) {
            for (const Label& label : slot.labels) { //Populate map of messages
                Message& msg = messages_by_label[label.string];
                msg.label = label;
                msg.attributes = attributes.entries[label.itemIndex];
                msg.style = styles.entries[label.itemIndex];
                msg.text = text.entries[label.itemIndex];
            }
        }

        return LMSError::NONE;
    }

    LMSError MSBTFile::loadFromFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERR_AND_RETURN(LMSError::COULD_NOT_OPEN);
        }
        return loadFromBinary(file);
    }

    Message& MSBTFile::addMessage(const std::string& label, const Attributes& attributes, const TSY1Entry& style, const std::u16string& message) {
        Message& newMessage = messages_by_label[label];

        newMessage.label.tableIdx = LMS::calcLabelHash(labels.entryCount, label); //Entry count is always 0x65 for .msbt
        newMessage.label.length = label.size();
        newMessage.label.string = label;
        newMessage.label.itemIndex = messages_by_label.size() - 1;

        newMessage.attributes = attributes;
        newMessage.style = style;
        newMessage.text.message = message;

        return newMessage;
    }

    LMSError MSBTFile::writeToStream(std::ostream& out) {
        //Go through and update all the sections based on the messages by ID
        labels.tableSlots.clear();
        labels.tableSlots.resize(101); //hash table always has 101 tableSlots in MSBT files
        attributes.entries.resize(messages_by_label.size()); //Make sure these are full size, the file relies on indexes a bunch so we need to replace the right indexes in the list (labels store indexes, they're different)
        styles.entries.resize(messages_by_label.size());
        text.entries.resize(messages_by_label.size());

        for (const auto& [label, message] : messages_by_label) {
            labels.tableSlots[message.label.tableIdx].labels.push_back(message.label);
            attributes.entries[message.label.itemIndex] = message.attributes;
            styles.entries[message.label.itemIndex] = message.style;
            text.entries[message.label.itemIndex] = message.text;
        }

        labels.sectionSize = labels.entryCount * 0x8 + 0x4;
        uint32_t nextGroupOffset = labels.sectionSize; //First entry starts after the table
        for (HashTableSlot& entry : labels.tableSlots) {
            entry.labelCount = entry.labels.size();
            entry.labelOffset = nextGroupOffset;
            for (Label& label : entry.labels) {
                nextGroupOffset += label.string.size() + 0x5; //loop through the labels in group, add their length to offset
                labels.sectionSize += label.string.size() + 0x5; //Add entry lengths to section length
            }
        }

        attributes.sectionSize = attributes.entries.size() * attributes.getEntrySize() + 0x8; //Size includes 8 bytes for entry count + size

        styles.sectionSize = styles.entries.size() * 0x4;

        uint32_t nextOffset = text.entries.size() * 0x4 + 0x4; //first offset = offset for each entry + the number of entries
        for (TXT2Entry& entry : text.entries) {
            entry.offset = nextOffset;
            entry.nextOffset = entry.offset + (entry.message.size() * 2); //size x2 because it returns the number of utf-16 chars, but file offsets need byte counts
            nextOffset = entry.nextOffset;
        }
        text.sectionSize = text.entries.back().nextOffset;

        header.write(out);
        labels.write(out);
        attributes.write(out);
        styles.write(out);
        text.write(out);

        out.seekp(0, std::ios::end);
        header.fileSize = out.tellp();
        out.seekp(0x12, std::ios::beg);

        uint32_t fileSize_BE = Utility::Endian::toPlatform(eType::Big, header.fileSize);
        out.write(reinterpret_cast<const char*>(&fileSize_BE), sizeof(fileSize_BE)); //Update full file size

        return LMSError::NONE;
    }

    LMSError MSBTFile::writeToFile(const std::string& outFilePath) {
        std::ofstream outFile(outFilePath, std::ios::binary | std::ios::out);
        if (!outFile.is_open()) {
            LOG_ERR_AND_RETURN(LMSError::COULD_NOT_OPEN);
        }
        return writeToStream(outFile);
    }

}
