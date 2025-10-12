#include "bffnt.hpp"

#include <utility/endian.hpp>
#include <utility/common.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;



BFFNTError BFFNTSection::read(std::istream& in) {
    if(!in.read(magic, sizeof(magic))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }

    if(!in.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }

    Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

    return BFFNTError::NONE;
}

BFFNTError BFFNTSection::write(std::ostream& out) {
    Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

    if(!out.write(magic, sizeof(magic))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }

    if(!out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }

    return BFFNTError::NONE;
}

BFFNTError FontInfo::read(std::istream& in) {
    LOG_AND_RETURN_IF_ERR(BFFNTSection::read(in));

    if(std::strncmp(magic, "FINF", 4) != 0) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);
    if(sectionSize != 0x20) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);

    if(!in.read(reinterpret_cast<char*>(&type), sizeof(type))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&height), sizeof(height))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&width), sizeof(width))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&ascent), sizeof(ascent))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&lineFeed), sizeof(lineFeed))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&defaultGlyphIdx), sizeof(defaultGlyphIdx))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&defaultLeftDist), sizeof(defaultLeftDist))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&defaultCellWidth), sizeof(defaultCellWidth))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&defaultCharWidth), sizeof(defaultCharWidth))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&encoding), sizeof(encoding))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&texSectionOffset), sizeof(texSectionOffset))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&widthSectionOffset), sizeof(widthSectionOffset))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&codeMapSectionOffset), sizeof(codeMapSectionOffset))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }

    Utility::Endian::toPlatform_inplace(eType::Big, lineFeed);
    Utility::Endian::toPlatform_inplace(eType::Big, defaultGlyphIdx);
    Utility::Endian::toPlatform_inplace(eType::Big, texSectionOffset);
    Utility::Endian::toPlatform_inplace(eType::Big, widthSectionOffset);
    Utility::Endian::toPlatform_inplace(eType::Big, codeMapSectionOffset);

    return BFFNTError::NONE;
}

BFFNTError FontInfo::write(std::ostream& out) {
    LOG_AND_RETURN_IF_ERR(BFFNTSection::write(out));

    Utility::Endian::toPlatform_inplace(eType::Big, lineFeed);
    Utility::Endian::toPlatform_inplace(eType::Big, defaultGlyphIdx);
    Utility::Endian::toPlatform_inplace(eType::Big, texSectionOffset);
    Utility::Endian::toPlatform_inplace(eType::Big, widthSectionOffset);
    Utility::Endian::toPlatform_inplace(eType::Big, codeMapSectionOffset);

    if(!out.write(reinterpret_cast<const char*>(&type), sizeof(type))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&height), sizeof(height))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&width), sizeof(width))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&ascent), sizeof(ascent))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&lineFeed), sizeof(lineFeed))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&defaultGlyphIdx), sizeof(defaultGlyphIdx))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&defaultLeftDist), sizeof(defaultLeftDist))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&defaultCellWidth), sizeof(defaultCellWidth))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&defaultCharWidth), sizeof(defaultCharWidth))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&encoding), sizeof(encoding))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&texSectionOffset), sizeof(texSectionOffset))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&widthSectionOffset), sizeof(widthSectionOffset))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&codeMapSectionOffset), sizeof(codeMapSectionOffset))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }

    return BFFNTError::NONE;
}

BFFNTError BFFNTTexture::read(std::istream& in) {
    LOG_AND_RETURN_IF_ERR(BFFNTSection::read(in));

    if(std::strncmp(magic, "TGLP", 4) != 0) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);

    if(!in.read(reinterpret_cast<char*>(&cellWidth), sizeof(cellWidth))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&cellHeight), sizeof(cellHeight))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&numSheets), sizeof(numSheets))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&unknown), sizeof(unknown))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&sheetSize), sizeof(sheetSize))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&baselinePos), sizeof(baselinePos))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&texFormat), sizeof(texFormat))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&cellsPerRow), sizeof(cellsPerRow))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&cellsPerColumn), sizeof(cellsPerColumn))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&texWidth), sizeof(texWidth))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&texHeight), sizeof(texHeight))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&texDataOffset), sizeof(texDataOffset))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }

    Utility::Endian::toPlatform_inplace(eType::Big, sheetSize);
    Utility::Endian::toPlatform_inplace(eType::Big, baselinePos);
    Utility::Endian::toPlatform_inplace(eType::Big, texFormat);
    Utility::Endian::toPlatform_inplace(eType::Big, cellsPerRow);
    Utility::Endian::toPlatform_inplace(eType::Big, cellsPerColumn);
    Utility::Endian::toPlatform_inplace(eType::Big, texWidth);
    Utility::Endian::toPlatform_inplace(eType::Big, texHeight);
    Utility::Endian::toPlatform_inplace(eType::Big, texDataOffset);

    sheets.resize(numSheets, std::string(sheetSize, '\0'));

    in.seekg(texDataOffset, std::ios::beg);
    for(std::string& sheet : sheets) {
        if(!in.read(sheet.data(), sheet.size())) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
    }

    return BFFNTError::NONE;
}

BFFNTError BFFNTTexture::write(std::ostream& out) {
    Utility::Endian::toPlatform_inplace(eType::Big, sheetSize);
    Utility::Endian::toPlatform_inplace(eType::Big, baselinePos);
    Utility::Endian::toPlatform_inplace(eType::Big, texFormat);
    Utility::Endian::toPlatform_inplace(eType::Big, cellsPerRow);
    Utility::Endian::toPlatform_inplace(eType::Big, cellsPerColumn);
    Utility::Endian::toPlatform_inplace(eType::Big, texWidth);
    Utility::Endian::toPlatform_inplace(eType::Big, texHeight);
    Utility::Endian::toPlatform_inplace(eType::Big, texDataOffset);

    LOG_AND_RETURN_IF_ERR(BFFNTSection::write(out));

    if(!out.write(reinterpret_cast<const char*>(&cellWidth), sizeof(cellWidth))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&cellHeight), sizeof(cellHeight))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&numSheets), sizeof(numSheets))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&unknown), sizeof(unknown))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&sheetSize), sizeof(sheetSize))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&baselinePos), sizeof(baselinePos))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&texFormat), sizeof(texFormat))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&cellsPerRow), sizeof(cellsPerRow))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&cellsPerColumn), sizeof(cellsPerColumn))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&texWidth), sizeof(texWidth))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&texHeight), sizeof(texHeight))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&texDataOffset), sizeof(texDataOffset))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }

    for(const std::string& sheet : sheets) {
        padToLen(out, 0x2000); //TODO: find out if this is the same always or if its particular to the surface (see: addrlib probably)
        if(!out.write(sheet.data(), sheet.size())) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
    }

    return BFFNTError::NONE;
}

BFFNTError BFFNTWidth::read(std::istream& in) {
    LOG_AND_RETURN_IF_ERR(BFFNTSection::read(in));

    if(std::strncmp(magic, "CWDH", 4) != 0) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);

    if(!in.read(reinterpret_cast<char*>(&firstGlyphIndex), sizeof(firstGlyphIndex))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&lastGlyphIndex), sizeof(lastGlyphIndex))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&nextPtr), sizeof(nextPtr))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }

    Utility::Endian::toPlatform_inplace(eType::Big, firstGlyphIndex);
    Utility::Endian::toPlatform_inplace(eType::Big, lastGlyphIndex);
    Utility::Endian::toPlatform_inplace(eType::Big, nextPtr);

    entries.resize(lastGlyphIndex - firstGlyphIndex + 1);

    for(Entry& entry : entries) {
        if(!in.read(reinterpret_cast<char*>(&entry.leftDist), sizeof(entry.leftDist))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&entry.cellWidth), sizeof(entry.cellWidth))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&entry.charWidth), sizeof(entry.charWidth))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
    }

    return BFFNTError::NONE;
}

BFFNTError BFFNTWidth::write(std::ostream& out) {
    LOG_AND_RETURN_IF_ERR(BFFNTSection::write(out));

    Utility::Endian::toPlatform_inplace(eType::Big, firstGlyphIndex);
    Utility::Endian::toPlatform_inplace(eType::Big, lastGlyphIndex);
    Utility::Endian::toPlatform_inplace(eType::Big, nextPtr);

    if(!out.write(reinterpret_cast<const char*>(&firstGlyphIndex), sizeof(firstGlyphIndex))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&lastGlyphIndex), sizeof(lastGlyphIndex))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&nextPtr), sizeof(nextPtr))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }

    for(const Entry& entry : entries) {
        if(!out.write(reinterpret_cast<const char*>(&entry.leftDist), sizeof(entry.leftDist))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if(!out.write(reinterpret_cast<const char*>(&entry.cellWidth), sizeof(entry.cellWidth))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if(!out.write(reinterpret_cast<const char*>(&entry.charWidth), sizeof(entry.charWidth))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
    }

    return BFFNTError::NONE;
}

BFFNTError BFFNTCodeMap::read(std::istream& in) {
    LOG_AND_RETURN_IF_ERR(BFFNTSection::read(in));

    if(std::strncmp(magic, "CMAP", 4) != 0) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);

    if(!in.read(reinterpret_cast<char*>(&firstCode), sizeof(firstCode))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&lastCode), sizeof(lastCode))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&mapType), sizeof(mapType))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&nextPtr), sizeof(nextPtr))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }

    Utility::Endian::toPlatform_inplace(eType::Big, firstCode);
    Utility::Endian::toPlatform_inplace(eType::Big, lastCode);
    Utility::Endian::toPlatform_inplace(eType::Big, mapType);
    Utility::Endian::toPlatform_inplace(eType::Big, padding_0x00);
    Utility::Endian::toPlatform_inplace(eType::Big, nextPtr);

    switch(mapType) {
        case MapType::DIRECT:
            {
                uint16_t baseIndex;
                if(!in.read(reinterpret_cast<char*>(&baseIndex), sizeof(baseIndex))) {
                    LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
                }
                Utility::Endian::toPlatform_inplace(eType::Big, baseIndex);

                for(size_t code = firstCode; code <= lastCode; code++) {
                    mapping[code] = baseIndex + (code - firstCode);
                }
            }

            break;
        case MapType::TABLE:
            for(size_t code = firstCode; code <= lastCode; code++) {
                uint16_t index;
                if(!in.read(reinterpret_cast<char*>(&index), sizeof(index))) {
                    LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
                }
                Utility::Endian::toPlatform_inplace(eType::Big, index);

                mapping[code] = index;
            }

            break;
        case MapType::SCAN:
            {
                uint16_t count;
                if(!in.read(reinterpret_cast<char*>(&count), sizeof(count))) {
                    LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
                }
                Utility::Endian::toPlatform_inplace(eType::Big, count);

                for(size_t i = 0; i < count; i++) {
                    uint16_t code, index;
                    if(!in.read(reinterpret_cast<char*>(&code), sizeof(code))) {
                        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
                    }
                    if(!in.read(reinterpret_cast<char*>(&index), sizeof(index))) {
                        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
                    }
                    Utility::Endian::toPlatform_inplace(eType::Big, code);
                    Utility::Endian::toPlatform_inplace(eType::Big, index);

                    mapping[code] = index;
                }
            }

            break;
        default:
            LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);
    }

    return BFFNTError::NONE;
}

BFFNTError BFFNTCodeMap::write(std::ostream& out) {
    LOG_AND_RETURN_IF_ERR(BFFNTSection::write(out));

    Utility::Endian::toPlatform_inplace(eType::Big, firstCode);
    Utility::Endian::toPlatform_inplace(eType::Big, lastCode);
    const auto mapType_BE = Utility::Endian::toPlatform(eType::Big, mapType);
    Utility::Endian::toPlatform_inplace(eType::Big, padding_0x00);
    Utility::Endian::toPlatform_inplace(eType::Big, nextPtr);

    if(!out.write(reinterpret_cast<const char*>(&firstCode), sizeof(firstCode))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&lastCode), sizeof(lastCode))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&mapType_BE), sizeof(mapType_BE))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }
    if(!out.write(reinterpret_cast<const char*>(&nextPtr), sizeof(nextPtr))) {
        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
    }

    switch(mapType) {
        case MapType::DIRECT:
            {
                const uint16_t baseIndex = Utility::Endian::toPlatform(eType::Big, mapping.begin()->second);
                if(!out.write(reinterpret_cast<const char*>(&baseIndex), sizeof(baseIndex))) {
                    LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
                }
            }

            break;
        case MapType::TABLE:
            for(const auto& [code, index] : mapping) {
                const uint16_t index_BE = Utility::Endian::toPlatform(eType::Big, index);
                if(!out.write(reinterpret_cast<const char*>(&index_BE), sizeof(index_BE))) {
                    LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
                }
            }

            break;
        case MapType::SCAN:
            {
                const uint16_t count = Utility::Endian::toPlatform(eType::Big, static_cast<uint16_t>(mapping.size()));
                if(!out.write(reinterpret_cast<const char*>(&count), sizeof(count))) {
                    LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
                }

                for(const auto& [code, index] : mapping) {
                    const uint16_t code_BE = Utility::Endian::toPlatform(eType::Big, code);
                    const uint16_t index_BE = Utility::Endian::toPlatform(eType::Big, index);
                    if(!out.write(reinterpret_cast<const char*>(&code_BE), sizeof(code_BE))) {
                        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
                    }
                    if(!out.write(reinterpret_cast<const char*>(&index_BE), sizeof(index_BE))) {
                        LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
                    }
                }
            }

            break;
        default:
            LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);
    }

    return BFFNTError::NONE;
}

BFFNTError BFFNTKerning::read(std::istream& in) {
    LOG_AND_RETURN_IF_ERR(BFFNTSection::read(in));

    if(std::strncmp(magic, "KRNG", 4) != 0) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);

    return BFFNTError::UNKNOWN;
}

BFFNTError BFFNTKerning::write(std::ostream& out) {
    return BFFNTError::UNKNOWN;
}



namespace FileTypes {

    const char* BFFNTErrorGetName(const BFFNTError err) {
        switch (err) {
            case BFFNTError::NONE:
                return "NONE";
            case BFFNTError::COULD_NOT_OPEN:
                return "COULD_NOT_OPEN";
            case BFFNTError::REACHED_EOF:
                return "REACHED_EOF";
            case BFFNTError::NOT_BFFNT:
                return "NOT_BFFNT";
            case BFFNTError::UNSUPPORTED_VERSION:
                return "UNSUPPORTED_VERSION";
            case BFFNTError::UNEXPECTED_VALUE:
                return "UNEXPECTED_VALUE";
            default:
                return "UNKNOWN";
        }
    }

    void BFFNTFile::initNew() {
        
    }

    BFFNTError BFFNTFile::loadFromBinary(std::istream& in) {
        if (!in.read(reinterpret_cast<char*>(&header.magicFFNT), sizeof(header.magicFFNT))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if(std::strncmp(header.magicFFNT, "FFNT", 4) != 0) LOG_ERR_AND_RETURN(BFFNTError::NOT_BFFNT);

        if (!in.read(reinterpret_cast<char*>(&header.byteOrderMarker_0xFEFF), sizeof(header.byteOrderMarker_0xFEFF))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&header.headerSize_0x14), sizeof(header.headerSize_0x14))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&header.version), sizeof(header.version))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&header.fileSize), sizeof(header.fileSize))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&header.numSections), sizeof(header.numSections))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, header.byteOrderMarker_0xFEFF);
        Utility::Endian::toPlatform_inplace(eType::Big, header.headerSize_0x14);
        Utility::Endian::toPlatform_inplace(eType::Big, header.version);
        Utility::Endian::toPlatform_inplace(eType::Big, header.fileSize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.numSections);

        if(header.byteOrderMarker_0xFEFF != 0xFEFF) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);
        if(header.headerSize_0x14 != 0x14) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);
        if(header.version != 0x03000000) LOG_ERR_AND_RETURN(BFFNTError::UNSUPPORTED_VERSION);
        if(header.padding_0x00 != 0x0000) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);

        //TODO: generalize this to be less particular about section order
        LOG_AND_RETURN_IF_ERR(info.read(in));

        if(info.texSectionOffset == 0) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);
        if(info.widthSectionOffset == 0) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);
        if(info.codeMapSectionOffset == 0) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);

        in.seekg(info.texSectionOffset - 8, std::ios::beg);
        LOG_AND_RETURN_IF_ERR(texture.read(in));

        for(uint32_t nextPtr = info.widthSectionOffset; nextPtr != 0; nextPtr = widths.back().nextPtr) {
            in.seekg(nextPtr - 8, std::ios::beg);
            LOG_AND_RETURN_IF_ERR(widths.emplace_back().read(in));
        }

        for(uint32_t nextPtr = info.codeMapSectionOffset; nextPtr != 0; nextPtr = mappings.back().nextPtr) {
            in.seekg(nextPtr - 8, std::ios::beg);
            LOG_AND_RETURN_IF_ERR(mappings.emplace_back().read(in));
        }

        //TODO: implement kerning

        return BFFNTError::NONE;
    }

    BFFNTError BFFNTFile::loadFromFile(const fspath& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERR_AND_RETURN(BFFNTError::COULD_NOT_OPEN);
        }
        return loadFromBinary(file);
    }

    BFFNTError BFFNTFile::writeToStream(std::ostream& out) {
        Utility::Endian::toPlatform_inplace(eType::Big, header.byteOrderMarker_0xFEFF);
        Utility::Endian::toPlatform_inplace(eType::Big, header.headerSize_0x14);
        Utility::Endian::toPlatform_inplace(eType::Big, header.version);
        Utility::Endian::toPlatform_inplace(eType::Big, header.fileSize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.numSections);

        if (!out.write(reinterpret_cast<const char*>(&header.magicFFNT), sizeof(header.magicFFNT))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if (!out.write(reinterpret_cast<const char*>(&header.byteOrderMarker_0xFEFF), sizeof(header.byteOrderMarker_0xFEFF))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if (!out.write(reinterpret_cast<const char*>(&header.headerSize_0x14), sizeof(header.headerSize_0x14))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if (!out.write(reinterpret_cast<const char*>(&header.version), sizeof(header.version))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if (!out.write(reinterpret_cast<const char*>(&header.fileSize), sizeof(header.fileSize))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if (!out.write(reinterpret_cast<const char*>(&header.numSections), sizeof(header.numSections))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }
        if (!out.write(reinterpret_cast<const char*>(&header.padding_0x00), sizeof(header.padding_0x00))) {
            LOG_ERR_AND_RETURN(BFFNTError::REACHED_EOF);
        }

        LOG_AND_RETURN_IF_ERR(info.write(out));

        if(info.texSectionOffset == 0) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);
        if(info.widthSectionOffset == 0) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);
        if(info.codeMapSectionOffset == 0) LOG_ERR_AND_RETURN(BFFNTError::UNEXPECTED_VALUE);

        padToLen(out, 4);
        LOG_AND_RETURN_IF_ERR(texture.write(out));

        for(BFFNTWidth& width : widths) {
            padToLen(out, 4);
            LOG_AND_RETURN_IF_ERR(width.write(out));
        }

        for(BFFNTCodeMap& mapping : mappings) {
            padToLen(out, 4);
            LOG_AND_RETURN_IF_ERR(mapping.write(out));
        }

        //TODO: implement kerning

        padToLen(out, 4);

        return BFFNTError::NONE;
    }

    BFFNTError BFFNTFile::writeToFile(const fspath& filePath) {
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERR_AND_RETURN(BFFNTError::COULD_NOT_OPEN);
        }
        return writeToStream(file);
    }

    uint8_t BFFNTFile::getWidth(const char16_t& c) const {
        const size_t& index = getGlyphIndex(c);
        
        const auto& it = std::ranges::find_if(widths, [index](const auto& width){ return width.firstGlyphIndex <= index && index <= width.lastGlyphIndex; });
        if(it == widths.end()) {
            return -1;
        }

        const size_t& widthIndex = index - it->firstGlyphIndex;
        if(widthIndex >= it->entries.size()) {
            return -1;
        }

        return it->entries[widthIndex].charWidth;
    }

    size_t BFFNTFile::getGlyphIndex(const char16_t& c) const {
        const auto& it = std::ranges::find_if(mappings, [c](const auto& map){ return map.firstCode <= c && c <= map.lastCode; });
        if(it == mappings.end() || !it->mapping.contains(c)) {
            return -1;
        }

        return it->mapping.at(c);
    }
}
