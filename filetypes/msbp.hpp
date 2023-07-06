//Format is part of LibMessageStudio
//MSBP files store project data including text commands, colors, attributes, etc
//This isn't used in rando yet, but is a base for documentation and tools

#pragma once

#include <filetypes/shared/lms.hpp>
#include <utility/common.hpp>
#include <filetypes/baseFiletype.hpp>



class MSBPHeader : public FileHeader {
public:
    virtual ~MSBPHeader() override = default;
    virtual LMSError read(std::istream& in) override;
    //virtual void write(std::ostream& out) override;
};

class CLB1 : public SectionHeader, public HashTable {
public:
    virtual ~CLB1() override = default;
    virtual LMSError read(std::istream& in) override;
    virtual void write(std::ostream& out) override;
};

class CLR1 : public SectionHeader {
public:
    std::vector<RGBA8> colors;

    virtual ~CLR1() override = default;
    virtual LMSError read(std::istream& in) override;
    virtual void write(std::ostream& out) override;
private:
    uint32_t entryCount;
};

class ALB1 : public SectionHeader, public HashTable {
public:
    virtual ~ALB1() override = default;
    virtual LMSError read(std::istream& in) override;
    virtual void write(std::ostream& out) override;
};

struct Attribute {
    uint8_t type;
    uint8_t padding_0x00;
    uint16_t listIdx; //ALI2 block index, only used for type 9
    uint32_t atrOffset; //Location in the text attribute struct
};

class ATI2 : public SectionHeader {
public:
    std::vector<Attribute> attributes;

    virtual ~ATI2() override = default;
    virtual LMSError read(std::istream& in) override;
    virtual void write(std::ostream& out) override;
private:
    uint32_t entryCount;
};

struct AttributeList {
    uint32_t numItems;
    std::vector<uint32_t> offsets;
    std::vector<std::string> itemNames;
};

class ALI2 : public SectionHeader {
public:
    std::vector<AttributeList> lists;

    virtual ~ALI2() override = default;
    virtual LMSError read(std::istream& in) override;
    virtual void write(std::ostream& out) override;
private:
    uint32_t numLists;
    std::vector<uint32_t> offsets;
};

struct TagGroup {
    uint16_t numTags;
    std::vector<uint16_t> tagIndexes; //in TAG2 block
    std::string groupName;
};

class TGG2 : public SectionHeader {
public:
    std::vector<TagGroup> groups;

    virtual ~TGG2() override = default;
    virtual LMSError read(std::istream& in) override;
    virtual void write(std::ostream& out) override;
private:
    uint16_t numGroups;
    uint16_t padding_0x00;
    std::vector<uint32_t> offsets;
};

struct Tag {
    uint16_t numParams;
    std::vector<uint16_t> parameterIndexes; //in TGP2 block
    std::string tagName;
};

class TAG2 : public SectionHeader {
public:
    std::vector<Tag> tags;

    virtual ~TAG2() override = default;
    virtual LMSError read(std::istream& in) override;
    virtual void write(std::ostream& out) override;
private:
    uint16_t numTags;
    uint16_t padding_0x00;
    std::vector<uint32_t> offsets;
};

struct TagParameter {
    uint8_t paramType;
    std::string paramName;

    //These are only used for type 9
    //Could be std::optional but it's a bit unwieldy
    uint8_t padding_0x00;
    uint16_t numListItems;
    std::vector<uint16_t> itemIndexes; //in TGL2 block
};

class TGP2 : public SectionHeader {
public:
    std::vector<TagParameter> params;

    virtual ~TGP2() override = default;
    virtual LMSError read(std::istream& in) override;
    virtual void write(std::ostream& out) override;
private:
    uint16_t numParams;
    uint16_t padding_0x00;
    std::vector<uint32_t> offsets;
};

class TGL2 : public SectionHeader {
public:
    std::vector<std::string> names;

    virtual ~TGL2() override = default;
    virtual LMSError read(std::istream& in) override;
    virtual void write(std::ostream& out) override;
private:
    uint16_t numItems;
    uint16_t padding_0x00;
    std::vector<uint32_t> offsets;
};

class SLB1 : public SectionHeader, public HashTable {
public:
    virtual ~SLB1() override = default;
    virtual LMSError read(std::istream& in) override;
    virtual void write(std::ostream& out) override;
};

struct Style {
    uint32_t regionWidth; //textbox width in pixels, 720x1080 screen
    uint32_t lineNum;
    uint32_t fontIdx;
    uint32_t baseColorIdx;
};

class SYL3 : public SectionHeader {
public:
    std::vector<Style> styles;

    virtual ~SYL3() override = default;
    virtual LMSError read(std::istream& in) override;
    virtual void write(std::ostream& out) override;
private:
    uint32_t numStyles;
};

class CTI1 : public SectionHeader {
public:
    std::vector<std::string> filenames;

    virtual ~CTI1() override = default;
    virtual LMSError read(std::istream& in) override;
    virtual void write(std::ostream& out) override;
private:
    uint32_t entryCount;
    std::vector<uint32_t> offsets;
};

namespace FileTypes {

    class MSBPFile : public FileType {
    public:
        CLB1 colorLabels;
        CLR1 colors;
        ALB1 attributeLabels;
        ATI2 attributeInfo;
        ALI2 attributeLists;
        TGG2 tagGroups;
        TAG2 tags;
        TGP2 tagParams;
        TGL2 tagLists;
        SLB1 styleLabels;
        SYL3 styles;
        CTI1 sources;

        MSBPFile();
        static MSBPFile createNew();
        LMSError loadFromBinary(std::istream& msbp);
        LMSError loadFromFile(const std::string& filePath);
        LMSError writeToStream(std::ostream& out);
        LMSError writeToFile(const std::string& outFilePath);

    private:
        FileHeader header;

        void initNew() override;
    };
}
