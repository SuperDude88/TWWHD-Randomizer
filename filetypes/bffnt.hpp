//Format is handled under the nw::font namespace, possibly part of the NintendoWare::lyt library(?)
//BFFNT files store fonts, primarily for BFLYT layouts
//This isn't used in rando at the moment but could be used for more precise text wrapping

#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include <string>
#include <map>

#include <filetypes/baseFiletype.hpp>



enum struct [[nodiscard]] BFFNTError {
    NONE = 0,
    COULD_NOT_OPEN,
    REACHED_EOF,
    NOT_BFFNT,
    UNSUPPORTED_VERSION,
    UNEXPECTED_VALUE,
    UNKNOWN,
    COUNT
};

struct BFFNTHeader {
    char magicFFNT[4];
    uint16_t byteOrderMarker_0xFEFF;
    uint16_t headerSize_0x14;
    uint32_t version;
    uint32_t fileSize;
    uint16_t numSections;
    uint16_t padding_0x00;
};

class BFFNTSection {
public:
    char magic[4];
    uint32_t sectionSize;

    virtual ~BFFNTSection() = default;

    virtual BFFNTError read(std::istream& in);
    virtual BFFNTError write(std::ostream& out);
};

class FontInfo : public BFFNTSection {
public:
    uint8_t type; // might be flags? existing tools call it type but game code doesn't offer much info besides IsBorderEffectEnabled
    uint8_t height;
    uint8_t width;
    uint8_t ascent;
    uint16_t lineFeed;
    uint16_t defaultGlyphIdx;
    uint8_t defaultLeftDist;
    uint8_t defaultCellWidth;
    uint8_t defaultCharWidth;
    uint8_t encoding;
    uint32_t texSectionOffset;
    uint32_t widthSectionOffset;
    uint32_t codeMapSectionOffset;

    BFFNTError read(std::istream& in) override;
    BFFNTError write(std::ostream& out) override;
};

class BFFNTTexture : public BFFNTSection {
public:
    uint8_t cellWidth;
    uint8_t cellHeight;
    uint8_t numSheets;
    uint8_t unknown;
    uint32_t sheetSize;
    uint16_t baselinePos;
    uint16_t texFormat;
    uint16_t cellsPerRow;
    uint16_t cellsPerColumn;
    uint16_t texWidth;
    uint16_t texHeight;
    uint32_t texDataOffset;

    std::vector<std::string> sheets;

    BFFNTError read(std::istream& in) override;
    BFFNTError write(std::ostream& out) override;
};

class BFFNTWidth : public BFFNTSection {
public:
    struct Entry {
        uint8_t leftDist;
        uint8_t cellWidth;
        uint8_t charWidth;
    };

    uint16_t firstGlyphIndex;
    uint16_t lastGlyphIndex;
    uint32_t nextPtr;

    std::vector<Entry> entries;

    BFFNTError read(std::istream& in) override;
    BFFNTError write(std::ostream& out) override;
};

class BFFNTCodeMap : public BFFNTSection {
public:
    enum MapType : uint16_t {
        DIRECT = 0,
        TABLE = 1,
        SCAN = 2
    };

    uint16_t firstCode;
    uint16_t lastCode;
    MapType mapType;
    uint16_t padding_0x00;
    uint32_t nextPtr;

    std::map<uint16_t, uint16_t> mapping;

    BFFNTError read(std::istream& in) override;
    BFFNTError write(std::ostream& out) override;
};

class BFFNTKerning : public BFFNTSection {
public:
    // From what I've gathered so far
    // there is a list of "first characters" with offsets to a "second character" array
    // the second characters are each paired with a horizontal offset

    // the first character is matched with the previous letter, then the second character is
    // matched with the current letter to give the correct horizontal adjustment

    // have not yet discerned the format for this data since WWHD doesn't seem to use it
    // and I don't understand most of the existing resources

    BFFNTError read(std::istream& in) override;
    BFFNTError write(std::ostream& out) override;
};

namespace FileTypes {
    const char* BFFNTErrorGetName(BFFNTError err);

    class BFFNTFile final : public FileType {
    public:
        BFFNTHeader header;
        FontInfo info;
        BFFNTTexture texture;
        std::vector<BFFNTWidth> widths;
        std::vector<BFFNTCodeMap> mappings;
        std::optional<BFFNTKerning> kerning;

        BFFNTError loadFromBinary(std::istream& in);
        BFFNTError loadFromFile(const fspath& filePath);
        BFFNTError writeToStream(std::ostream& out);
        BFFNTError writeToFile(const fspath& filePath);

        uint8_t getWidth(const char16_t& c) const;
    private:
        size_t getGlyphIndex(const char16_t& c) const;

        void initNew() override;
    };
}
