#include "lms.hpp"

#include <algorithm>
#include <utility/endian.hpp>
#include <utility/file.hpp>
#include <command/Log.hpp>

#include <utility/platform.hpp>

using eType = Utility::Endian::Type;

LMSError FileHeader::read(std::istream& in) {
    if (!in.read(magic, 8)) LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    if (!in.read(reinterpret_cast<char*>(&byteOrderMarker), sizeof(byteOrderMarker)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    if (!in.read(reinterpret_cast<char*>(&unknown_0x00), sizeof(unknown_0x00)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    if (!in.read(reinterpret_cast<char*>(&encoding), sizeof(encoding)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    if (!in.read(reinterpret_cast<char*>(&version_0x03), sizeof(version_0x03)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    if (!in.read(reinterpret_cast<char*>(&sectionCount), sizeof(sectionCount)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    if (!in.read(reinterpret_cast<char*>(&unknown2_0x00), sizeof(unknown2_0x00)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    if (!in.read(reinterpret_cast<char*>(&fileSize), sizeof(fileSize)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    if (!in.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    Utility::Endian::toPlatform_inplace(eType::Big, byteOrderMarker);
    Utility::Endian::toPlatform_inplace(eType::Big, unknown_0x00);
    Utility::Endian::toPlatform_inplace(eType::Big, sectionCount);
    Utility::Endian::toPlatform_inplace(eType::Big, unknown2_0x00);
    Utility::Endian::toPlatform_inplace(eType::Big, fileSize);
    if (unknown_0x00 != 0x0000) LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);
    if (version_0x03 != 0x03) LOG_ERR_AND_RETURN(LMSError::UNKNOWN_VERSION);
    if (unknown2_0x00 != 0x0000) LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);

    return LMSError::NONE;
}

void FileHeader::write(std::ostream& out) {
    Utility::Endian::toPlatform_inplace(eType::Big, byteOrderMarker);
    Utility::Endian::toPlatform_inplace(eType::Big, unknown_0x00);
    Utility::Endian::toPlatform_inplace(eType::Big, sectionCount);
    Utility::Endian::toPlatform_inplace(eType::Big, unknown2_0x00);
    Utility::Endian::toPlatform_inplace(eType::Big, fileSize);

    out.write(magic, 8);
    out.write(reinterpret_cast<const char*>(&byteOrderMarker), sizeof(byteOrderMarker));
    out.write(reinterpret_cast<const char*>(&unknown_0x00), sizeof(unknown_0x00));
    out.write(reinterpret_cast<const char*>(&encoding), sizeof(encoding));
    out.write(reinterpret_cast<const char*>(&version_0x03), sizeof(version_0x03));
    out.write(reinterpret_cast<const char*>(&sectionCount), sizeof(sectionCount));
    out.write(reinterpret_cast<const char*>(&unknown2_0x00), sizeof(unknown2_0x00));
    out.write(reinterpret_cast<const char*>(&fileSize), sizeof(fileSize));
    out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));
}

LMSError SectionHeader::read(std::istream &in) {
    if (!in.read(magic, 4)) LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    if (!in.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    if (!in.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
    
    return LMSError::NONE;
}

void SectionHeader::write(std::ostream &out) {
    Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

    out.write(magic, 4);
    out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize));
    out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));
}

LMSError HashTable::read(std::istream& in) {
    const std::streamoff tableStart = in.tellg();

    if (!in.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }

    Utility::Endian::toPlatform_inplace(eType::Big, entryCount);

    tableSlots.reserve(entryCount);
    for (uint32_t i = 0; i < entryCount; i++) {
        in.seekg(tableStart + 0x4 + i * 0x8, std::ios::beg);

        HashTableSlot& slot = tableSlots.emplace_back();
        if (!in.read(reinterpret_cast<char*>(&slot.labelCount), sizeof(slot.labelCount)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&slot.labelOffset), sizeof(slot.labelOffset)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, slot.labelCount);
        Utility::Endian::toPlatform_inplace(eType::Big, slot.labelOffset);
        
        in.seekg(slot.labelOffset + tableStart, std::ios::beg); //Seek to the start of the entries before the loop so it doesnt reset to the same string each time
        
        slot.labels.reserve(slot.labelCount);
        for (uint32_t x = 0; x < slot.labelCount; x++) {
            Label& label = slot.labels.emplace_back();
            label.tableIdx = i;
            if (!in.read(reinterpret_cast<char*>(&label.length), sizeof(label.length)))
            {
                LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
            }
            label.string.resize(label.length); //Length is 1 byte so no byteswap
            if (!in.read(&label.string[0], label.length))
            {
                LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
            }
            if (!in.read(reinterpret_cast<char*>(&label.itemIndex), sizeof(label.itemIndex)))
            {
                LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
            }
            Utility::Endian::toPlatform_inplace(eType::Big, label.itemIndex);
        }
    }

    return LMSError::NONE;
}
    
void HashTable::write(std::ostream& out) {
    const uint32_t tableStart = out.tellp();

    Utility::Endian::toPlatform_inplace(eType::Big, entryCount);
    out.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));

    Utility::Endian::toPlatform_inplace(eType::Big, entryCount);
    uint32_t i = 0;
    uint32_t curOffset = 0x8 * entryCount + 0x4;
    for (HashTableSlot& entry : tableSlots) {
        Utility::seek(out, tableStart + 0x4 + 0x8 * i, std::ios::beg);

        entry.labelCount = entry.labels.size();
        entry.labelOffset = curOffset;
        Utility::Endian::toPlatform_inplace(eType::Big, entry.labelCount); //byteswap inplace here and swap back later so all the swaps can be grouped in 1 spot instead of in each write (allows for easier Wii U conversion later)
        Utility::Endian::toPlatform_inplace(eType::Big, entry.labelOffset);

        out.write(reinterpret_cast<const char*>(&entry.labelCount), sizeof(entry.labelCount));
        out.write(reinterpret_cast<const char*>(&entry.labelOffset), sizeof(entry.labelOffset));

        std::ranges::sort(entry.labels, [](const Label& a, const Label& b) {
            return a.itemIndex < b.itemIndex;
        });
        Utility::Endian::toPlatform_inplace(eType::Big, entry.labelOffset);

        Utility::seek(out, tableStart + entry.labelOffset, std::ios::beg);
        for (Label& label : entry.labels) {
            Utility::Endian::toPlatform_inplace(eType::Big, label.itemIndex);

            label.length = label.string.size();
            out.write(reinterpret_cast<const char*>(&label.length), sizeof(label.length));
            out.write(&label.string[0], label.length);
            out.write(reinterpret_cast<const char*>(&label.itemIndex), sizeof(label.itemIndex));
            
            curOffset += 5 + label.length;
        }

        i++;
    }
}

namespace FileTypes {
    const char* LMSErrorGetName(LMSError err) {
        switch(err) {
            case LMSError::NONE:
                return "NONE";
            case LMSError::REACHED_EOF:
                return "REACHED_EOF";
            case LMSError::COULD_NOT_OPEN:
                return "COULD_NOT_OPEN";
            case LMSError::NOT_MSBT:
                return "NOT_MSBT";
            case LMSError::NOT_MSBP:
                return "NOT_MSBP";
            case LMSError::UNKNOWN_VERSION:
                return "UNKNOWN_VERSION";
            case LMSError::UNEXPECTED_VALUE:
                return "UNEXPECTED_VALUE";
            case LMSError::UNKNOWN_SECTION:
                return "UNKNOWN_SECTION";
            default:
                return "UNKNOWN";
        }
    }
}
