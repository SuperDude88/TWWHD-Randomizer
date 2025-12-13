// Format is part of LibMessageStudio
// MSBT files store text along with labels and other info

#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <filetypes/shared/lms.hpp>
#include <filetypes/baseFiletype.hpp>



class MSBTHeader final : public FileHeader {
public:
    ~MSBTHeader() override = default;

    LMSError read(std::istream& in) override;
    //virtual void write(std::ostream& out) override;
};

class LBL1 final : public SectionHeader, public HashTable {
public:
    ~LBL1() override = default;

    LMSError read(std::istream& in) override;
    void write(std::ostream& out) override;
};

struct Attributes {
    uint8_t character = 0; // The NPC (or similar entity) that it's attached to, internally "CharacterName"
    uint8_t boxStyle = 0; // Internally "BalloonType"
    uint8_t drawType = 0; // Internally "DisplayStyle"
    uint8_t screenPos = 3; // Internally "BalloonPlacement"
    uint16_t price = 0; // Internally "Price"
    uint16_t nextNo = 0; // Internally "NextNo", unknown purpose
    uint8_t item = 0xFF; // Matches with SD, is ignored, internally "Item"
    uint8_t lineAlignment = 0; // Internally "LineAlignment"
    uint8_t soundEffect = 0; // Internally "SE"
    uint8_t camera = 0; // Internally "Camera"
    uint16_t demoID = 0; // Internally "DemoID", extra ID used for cutscenes 
    uint8_t animation = 0; // Internally "Animation"
    uint32_t commentE_1 = 0; // Internally "Comment(E-1)", doesnt seem to align as an offset?
    uint32_t commentE_2 = 0; // Internally "Comment(E-2)", doesnt seem to align as an offset?
    
    uint8_t checkOriginal = 0; // Internally "CheckOriginal", unknown purpose, seemingly cut off by attribute entry length
    uint8_t checkRev = 0; // Internally "MCT Check Rev", unknown purpose, seemingly cut off by attribute entry length
    uint8_t mctTester = 0; // Internally "MCT Tester", unknown purpose, seemingly cut off by attribute entry length
};

class ATR1 final : public SectionHeader {
public:
    std::vector<Attributes> entries;

    ~ATR1() override = default;

    LMSError read(std::istream& in) override;
    void write(std::ostream& out) override;
    uint32_t getEntrySize() const { return entrySize; }
private:
    uint32_t entryCount;
    uint32_t entrySize;
};

struct TSY1Entry {
    uint32_t styleIndex; // Index in the MSBP style list
};

class TSY1 final : public SectionHeader {
public:
    std::vector<TSY1Entry> entries;
    
    ~TSY1() override = default;

    LMSError read(std::istream& in) override;
    void write(std::ostream& out) override;
};

struct TXT2Entry {
    uint32_t offset;
    uint32_t nextOffset;
    std::u16string message;
};

class TXT2 final : public SectionHeader {
public:
    std::vector<TXT2Entry> entries;
    
    ~TXT2() override = default;

    LMSError read(std::istream& in) override;
    void write(std::ostream& out) override;
private:
    uint32_t entryCount;
};

struct Message {
    Label label;
    Attributes attributes;
    TSY1Entry style;
    TXT2Entry text;
};

namespace FileTypes {
    class MSBTFile final : public FileType {
    public:
        std::unordered_map<std::string, Message> messages_by_label;

        MSBTFile() = default;
        static MSBTFile createNew();
        LMSError loadFromBinary(std::istream& msbt);
        LMSError loadFromFile(const fspath& filePath);
        Message& addMessage(const std::string& label, const Attributes& attributes, const TSY1Entry& style, const std::u16string& message);
        LMSError writeToStream(std::ostream& out);
        LMSError writeToFile(const fspath& outFilePath);

    private:
        FileHeader header;
        LBL1 labels;
        ATR1 attributes;
        TSY1 styles;
        TXT2 text;

        void initNew() override;
    };
}
