#include "msbp.hpp"

#include <cstring>

#include <utility/endian.hpp>
#include <utility/common.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;

LMSError MSBPHeader::read(std::istream& in) {
    LOG_AND_RETURN_IF_ERR(FileHeader::read(in));
    if (std::strncmp(magic, "MsgPrjBn", 8) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::NOT_MSBP);
    }
    if (encoding != 0x00) LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);
    if (sectionCount != 0x000C) LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);

    return LMSError::NONE;
}

LMSError CLB1::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "CLB1", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }
    LOG_AND_RETURN_IF_ERR(HashTable::read(in));

    LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 16, "\xab"));

    return LMSError::NONE;
}

void CLB1::write(std::ostream &out) {
    SectionHeader::write(out);
    HashTable::write(out);
    padToLen(out, 16, '\xab');
}

LMSError CLR1::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "CLR1", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }

    if (!in.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    Utility::Endian::toPlatform_inplace(eType::Big, entryCount);

    colors.reserve(entryCount);
    for (uint32_t i = 0; i < entryCount; i++) {
        RGBA8& color = colors.emplace_back();
        readRGBA(in, in.tellg(), color);
    }

    LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 16, "\xab"));

    return LMSError::NONE;
}

void CLR1::write(std::ostream &out) {
    SectionHeader::write(out);

    entryCount = colors.size();
    Utility::Endian::toPlatform_inplace(eType::Big, entryCount);
    
    out.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));

    for (const RGBA8& color : colors) {
        writeRGBA(out, color);
    }
    padToLen(out, 16, '\xab');
}

LMSError ALB1::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "ALB1", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }
    LOG_AND_RETURN_IF_ERR(HashTable::read(in));

    LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 16, "\xab"));

    return LMSError::NONE;
}

void ALB1::write(std::ostream &out) {
    SectionHeader::write(out);
    HashTable::write(out);
    padToLen(out, 16, '\xab');
}

LMSError ATI2::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "ATI2", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }

    if (!in.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    Utility::Endian::toPlatform_inplace(eType::Big, entryCount);

    attributes.reserve(entryCount);
    for (uint32_t i = 0; i < entryCount; i++) {
        Attribute& attr = attributes.emplace_back();
        
        if (!in.read(reinterpret_cast<char*>(&attr.type), sizeof(attr.type)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attr.padding_0x00), sizeof(attr.padding_0x00)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attr.listIdx), sizeof(attr.listIdx)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&attr.atrOffset), sizeof(attr.atrOffset)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, attr.listIdx);
        Utility::Endian::toPlatform_inplace(eType::Big, attr.atrOffset);
        if(attr.padding_0x00 != 0x00) LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);
    }

    LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 16, "\xab"));

    return LMSError::NONE;
}

void ATI2::write(std::ostream &out) {
    SectionHeader::write(out);

    entryCount = attributes.size();
    Utility::Endian::toPlatform_inplace(eType::Big, entryCount);
    
    out.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));

    for (Attribute& attr : attributes) {
        Utility::Endian::toPlatform_inplace(eType::Big, attr.listIdx);
        Utility::Endian::toPlatform_inplace(eType::Big, attr.atrOffset);

        out.write(reinterpret_cast<const char*>(&attr.type), sizeof(attr.type));
        out.write(reinterpret_cast<const char*>(&attr.padding_0x00), sizeof(attr.padding_0x00));
        out.write(reinterpret_cast<const char*>(&attr.listIdx), sizeof(attr.listIdx));
        out.write(reinterpret_cast<const char*>(&attr.atrOffset), sizeof(attr.atrOffset));
    }
    padToLen(out, 16, '\xab');
}

LMSError ALI2::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "ALI2", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }

    if (!in.read(reinterpret_cast<char*>(&numLists), sizeof(numLists)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    Utility::Endian::toPlatform_inplace(eType::Big, numLists);

    offsets.reserve(numLists);
    lists.reserve(numLists);
    for (uint32_t i = 0; i < numLists; i++) {
        uint32_t& offset = offsets.emplace_back();
        if (!in.read(reinterpret_cast<char*>(&offset), sizeof(offset)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, offset);
    }

    for (uint32_t i = 0; i < numLists; i++) {
        AttributeList& list = lists.emplace_back();
        
        if (!in.read(reinterpret_cast<char*>(&list.numItems), sizeof(list.numItems)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, list.numItems);

        for (uint32_t x = 0; x < list.numItems; x++) {
            uint32_t& offset = list.offsets.emplace_back();
            if (!in.read(reinterpret_cast<char*>(&offset), sizeof(offset)))
            {
                LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
            }

            Utility::Endian::toPlatform_inplace(eType::Big, offset);
        }

        for (uint32_t x = 0; x < list.numItems; x++) {
            std::string& name = list.itemNames.emplace_back();
            name = readNullTerminatedStr(in, in.tellg());
            if(name.empty()) LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        
        LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 4, "\x00"));
    }

    LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 16, "\xab"));

    return LMSError::NONE;
}

void ALI2::write(std::ostream &out) {
    SectionHeader::write(out);

    numLists = lists.size();
    Utility::Endian::toPlatform_inplace(eType::Big, numLists);
    out.write(reinterpret_cast<const char*>(&numLists), sizeof(numLists));

    for (uint32_t& offset : offsets) {
        Utility::Endian::toPlatform_inplace(eType::Big, offset);
        out.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
    }

    for (AttributeList& list : lists) {
        list.numItems = list.itemNames.size();
        Utility::Endian::toPlatform_inplace(eType::Big, list.numItems);
        out.write(reinterpret_cast<const char*>(&list.numItems), sizeof(list.numItems));

        for (uint32_t& offset : list.offsets) {
            Utility::Endian::toPlatform_inplace(eType::Big, offset);
            out.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
        }
        
        for (const std::string& name : list.itemNames) {
            out.write(&name[0], name.size());
        }
        
        padToLen(out, 4, '\0');
    }

    padToLen(out, 16, '\xab');
}

LMSError TGG2::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "TGG2", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }

    if (!in.read(reinterpret_cast<char*>(&numGroups), sizeof(numGroups)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    if (!in.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    Utility::Endian::toPlatform_inplace(eType::Big, numGroups);
    Utility::Endian::toPlatform_inplace(eType::Big, padding_0x00);
    if(padding_0x00 != 0x0000) LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);

    offsets.reserve(numGroups);
    groups.reserve(numGroups);
    for (uint16_t i = 0; i < numGroups; i++) {
        uint32_t& offset = offsets.emplace_back();
        if (!in.read(reinterpret_cast<char*>(&offset), sizeof(offset)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, offset);
    }

    for (uint16_t i = 0; i < numGroups; i++) {
        TagGroup& group = groups.emplace_back();
        
        if (!in.read(reinterpret_cast<char*>(&group.numTags), sizeof(group.numTags)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, group.numTags);

        group.tagIndexes.reserve(group.numTags);
        for (uint16_t x = 0; x < group.numTags; x++) {
            uint16_t& index = group.tagIndexes.emplace_back();
            if (!in.read(reinterpret_cast<char*>(&index), sizeof(index)))
            {
                LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
            }

            Utility::Endian::toPlatform_inplace(eType::Big, index);
        }

        group.groupName = readNullTerminatedStr(in, in.tellg());
        if(group.groupName.empty()) LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        
        LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 4, "\x00"));
    }

    LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 16, "\xab"));

    return LMSError::NONE;
}

void TGG2::write(std::ostream &out) {
    SectionHeader::write(out);

    numGroups = groups.size();
    Utility::Endian::toPlatform_inplace(eType::Big, numGroups);
    Utility::Endian::toPlatform_inplace(eType::Big, padding_0x00);
    out.write(reinterpret_cast<const char*>(&numGroups), sizeof(numGroups));
    out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));

    for (uint32_t& offset : offsets) {
        Utility::Endian::toPlatform_inplace(eType::Big, offset);
        out.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
    }

    for (TagGroup& group : groups) {
        group.numTags = group.tagIndexes.size();
        Utility::Endian::toPlatform_inplace(eType::Big, group.numTags);
        out.write(reinterpret_cast<const char*>(&group.numTags), sizeof(group.numTags));

        for (uint16_t& index : group.tagIndexes) {
            Utility::Endian::toPlatform_inplace(eType::Big, index);
            out.write(reinterpret_cast<const char*>(&index), sizeof(index));
        }
        
        out.write(&group.groupName[0], group.groupName.size());
        
        padToLen(out, 4, '\0');
    }

    padToLen(out, 16, '\xab');
}

LMSError TAG2::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "TAG2", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }

    if (!in.read(reinterpret_cast<char*>(&numTags), sizeof(numTags)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    if (!in.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    Utility::Endian::toPlatform_inplace(eType::Big, numTags);
    Utility::Endian::toPlatform_inplace(eType::Big, padding_0x00);
    if(padding_0x00 != 0x0000) LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);

    offsets.reserve(numTags);
    tags.reserve(numTags);
    for (uint16_t i = 0; i < numTags; i++) {
        uint32_t& offset = offsets.emplace_back();
        if (!in.read(reinterpret_cast<char*>(&offset), sizeof(offset)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, offset);
    }

    for (uint16_t i = 0; i < numTags; i++) {
        Tag& tag = tags.emplace_back();
        
        if (!in.read(reinterpret_cast<char*>(&tag.numParams), sizeof(tag.numParams)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, tag.numParams);

        tag.parameterIndexes.reserve(tag.numParams);
        for (uint16_t x = 0; x < tag.numParams; x++) {
            uint16_t& index = tag.parameterIndexes.emplace_back();
            if (!in.read(reinterpret_cast<char*>(&index), sizeof(index)))
            {
                LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
            }

            Utility::Endian::toPlatform_inplace(eType::Big, index);
        }

        tag.tagName = readNullTerminatedStr(in, in.tellg());
        if(tag.tagName.empty()) LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        
        LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 4, "\x00"));
    }

    LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 16, "\xab"));

    return LMSError::NONE;
}

void TAG2::write(std::ostream &out) {
    SectionHeader::write(out);

    numTags = tags.size();
    Utility::Endian::toPlatform_inplace(eType::Big, numTags);
    Utility::Endian::toPlatform_inplace(eType::Big, padding_0x00);
    out.write(reinterpret_cast<const char*>(&numTags), sizeof(numTags));
    out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));

    for (uint32_t& offset : offsets) {
        Utility::Endian::toPlatform_inplace(eType::Big, offset);
        out.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
    }

    for (Tag& tag : tags) {
        tag.numParams = tag.parameterIndexes.size();
        Utility::Endian::toPlatform_inplace(eType::Big, tag.numParams);
        out.write(reinterpret_cast<const char*>(&tag.numParams), sizeof(tag.numParams));

        for (uint16_t& index : tag.parameterIndexes) {
            Utility::Endian::toPlatform_inplace(eType::Big, index);
            out.write(reinterpret_cast<const char*>(&index), sizeof(index));
        }
        
        out.write(&tag.tagName[0], tag.tagName.size());
        
        padToLen(out, 4, '\0');
    }

    padToLen(out, 16, '\xab');
}

LMSError TGP2::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "TGP2", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }

    if (!in.read(reinterpret_cast<char*>(&numParams), sizeof(numParams)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    if (!in.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    Utility::Endian::toPlatform_inplace(eType::Big, numParams);
    Utility::Endian::toPlatform_inplace(eType::Big, padding_0x00);
    if(padding_0x00 != 0x0000) LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);

    offsets.reserve(numParams);
    params.reserve(numParams);
    for (uint16_t i = 0; i < numParams; i++) {
        uint32_t& offset = offsets.emplace_back();
        if (!in.read(reinterpret_cast<char*>(&offset), sizeof(offset)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, offset);
    }

    for (uint16_t i = 0; i < numParams; i++) {
        TagParameter& param = params.emplace_back();
        
        if (!in.read(reinterpret_cast<char*>(&param.paramType), sizeof(param.paramType)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }

        if(param.paramType == 9) {
            if (!in.read(reinterpret_cast<char*>(&param.padding_0x00), sizeof(param.padding_0x00)))
            {
                LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
            }
            if(padding_0x00 != 0x00) LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);

            if (!in.read(reinterpret_cast<char*>(&param.numListItems), sizeof(param.numListItems)))
            {
                LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
            }
                Utility::Endian::toPlatform_inplace(eType::Big, param.numListItems);

            param.itemIndexes.reserve(param.numListItems);
            for (uint16_t x = 0; x < param.numListItems; x++) {
                uint16_t& index = param.itemIndexes.emplace_back();
                if (!in.read(reinterpret_cast<char*>(&index), sizeof(index)))
                {
                    LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
                }

                Utility::Endian::toPlatform_inplace(eType::Big, index);
            }
        }

        param.paramName = readNullTerminatedStr(in, in.tellg());
        if(param.paramName.empty()) LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        
        LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 4, "\x00"));
    }

    LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 16, "\xab"));

    return LMSError::NONE;
}

void TGP2::write(std::ostream &out) {
    SectionHeader::write(out);

    numParams = params.size();
    Utility::Endian::toPlatform_inplace(eType::Big, numParams);
    Utility::Endian::toPlatform_inplace(eType::Big, padding_0x00);
    out.write(reinterpret_cast<const char*>(&numParams), sizeof(numParams));
    out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));

    for (uint32_t& offset : offsets) {
        Utility::Endian::toPlatform_inplace(eType::Big, offset);
        out.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
    }

    for (TagParameter& param : params) {
        out.write(reinterpret_cast<const char*>(&param.paramType), sizeof(param.paramType));

        if(param.paramType == 9) {
            out.write(reinterpret_cast<const char*>(&param.padding_0x00), sizeof(param.padding_0x00));

            param.numListItems = param.itemIndexes.size();
            Utility::Endian::toPlatform_inplace(eType::Big, param.numListItems);
            out.write(reinterpret_cast<const char*>(&param.numListItems), sizeof(param.numListItems));

            for (uint16_t& index : param.itemIndexes) {
                Utility::Endian::toPlatform_inplace(eType::Big, index);
                out.write(reinterpret_cast<const char*>(&index), sizeof(index));
            }
        }
        
        out.write(&param.paramName[0], param.paramName.size());
        
        padToLen(out, 4, '\0');
    }

    padToLen(out, 16, '\xab');
}

LMSError TGL2::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "TGL2", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }

    if (!in.read(reinterpret_cast<char*>(&numItems), sizeof(numItems)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    if (!in.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    Utility::Endian::toPlatform_inplace(eType::Big, numItems);
    Utility::Endian::toPlatform_inplace(eType::Big, padding_0x00);
    if(padding_0x00 != 0x0000) LOG_ERR_AND_RETURN(LMSError::UNEXPECTED_VALUE);

    offsets.reserve(numItems);
    names.reserve(numItems);
    for (uint16_t i = 0; i < numItems; i++) {
        uint32_t& offset = offsets.emplace_back();
        if (!in.read(reinterpret_cast<char*>(&offset), sizeof(offset)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, offset);
    }

    for (uint16_t i = 0; i < numItems; i++) {
        std::string& name = names.emplace_back();
        
        name = readNullTerminatedStr(in, in.tellg());
        if(name.empty()) LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }

    LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 16, "\xab"));

    return LMSError::NONE;
}

void TGL2::write(std::ostream &out) {
    SectionHeader::write(out);

    numItems = names.size();
    Utility::Endian::toPlatform_inplace(eType::Big, numItems);
    Utility::Endian::toPlatform_inplace(eType::Big, padding_0x00);
    out.write(reinterpret_cast<const char*>(&numItems), sizeof(numItems));
    out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));

    for (uint32_t& offset : offsets) {
        Utility::Endian::toPlatform_inplace(eType::Big, offset);
        out.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
    }

    for (const std::string& name : names) {
        out.write(&name[0], name.size());
    }

    padToLen(out, 16, '\xab');
}

LMSError SLB1::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "SLB1", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }
    LOG_AND_RETURN_IF_ERR(HashTable::read(in));

    LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 16, "\xab"));

    return LMSError::NONE;
}

void SLB1::write(std::ostream &out) {
    SectionHeader::write(out);
    HashTable::write(out);
    padToLen(out, 16, '\xab');
}

LMSError SYL3::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "SYL3", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }

    if (!in.read(reinterpret_cast<char*>(&numStyles), sizeof(numStyles)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    Utility::Endian::toPlatform_inplace(eType::Big, numStyles);

    styles.reserve(numStyles);
    for (uint32_t i = 0; i < numStyles; i++) {
        Style& style = styles.emplace_back();
        if (!in.read(reinterpret_cast<char*>(&style.regionWidth), sizeof(style.regionWidth)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&style.lineNum), sizeof(style.lineNum)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&style.fontIdx), sizeof(style.fontIdx)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }
        if (!in.read(reinterpret_cast<char*>(&style.baseColorIdx), sizeof(style.baseColorIdx)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, style.regionWidth);
        Utility::Endian::toPlatform_inplace(eType::Big, style.lineNum);
        Utility::Endian::toPlatform_inplace(eType::Big, style.fontIdx);
        Utility::Endian::toPlatform_inplace(eType::Big, style.baseColorIdx);
    }

    LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 16, "\xab"));

    return LMSError::NONE;
}

void SYL3::write(std::ostream &out) {
    SectionHeader::write(out);

    numStyles = styles.size();
    Utility::Endian::toPlatform_inplace(eType::Big, numStyles);
    out.write(reinterpret_cast<const char*>(&numStyles), sizeof(numStyles));

    for (Style& style : styles) {
        Utility::Endian::toPlatform_inplace(eType::Big, style.regionWidth);
        Utility::Endian::toPlatform_inplace(eType::Big, style.lineNum);
        Utility::Endian::toPlatform_inplace(eType::Big, style.fontIdx);
        Utility::Endian::toPlatform_inplace(eType::Big, style.baseColorIdx);
        out.write(reinterpret_cast<const char*>(&style.regionWidth), sizeof(style.regionWidth));
        out.write(reinterpret_cast<const char*>(&style.lineNum), sizeof(style.lineNum));
        out.write(reinterpret_cast<const char*>(&style.fontIdx), sizeof(style.fontIdx));
        out.write(reinterpret_cast<const char*>(&style.baseColorIdx), sizeof(style.baseColorIdx));
    }

    padToLen(out, 16, '\xab');
}

LMSError CTI1::read(std::istream &in) {
    LOG_AND_RETURN_IF_ERR(SectionHeader::read(in));
    if (std::strncmp(magic, "CTI1", 4) != 0)
    {
        LOG_ERR_AND_RETURN(LMSError::UNKNOWN_SECTION);
    }

    if (!in.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount)))
    {
        LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }
    Utility::Endian::toPlatform_inplace(eType::Big, entryCount);

    offsets.reserve(entryCount);
    filenames.reserve(entryCount);
    for (uint32_t i = 0; i < entryCount; i++) {
        uint32_t& offset = offsets.emplace_back();
        if (!in.read(reinterpret_cast<char*>(&offset), sizeof(offset)))
        {
            LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, offset);
    }

    for (uint32_t i = 0; i < entryCount; i++) {
        std::string& name = filenames.emplace_back();
        
        name = readNullTerminatedStr(in, in.tellg());
        if(name.empty()) LOG_ERR_AND_RETURN(LMSError::REACHED_EOF);
    }

    LOG_AND_RETURN_IF_ERR(readPadding<LMSError>(in, 16, "\xab"));

    return LMSError::NONE;
}

void CTI1::write(std::ostream &out) {
    SectionHeader::write(out);

    entryCount = filenames.size();
    Utility::Endian::toPlatform_inplace(eType::Big, entryCount);
    out.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));

    for (uint32_t& offset : offsets) {
        Utility::Endian::toPlatform_inplace(eType::Big, offset);
        out.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
    }

    for (const std::string& name : filenames) {
        out.write(&name[0], name.size());
    }

    padToLen(out, 16, '\xab');
}

namespace FileTypes {

    void MSBPFile::initNew() {
        memcpy(&header.magic, "MsgPrjBn", 8);
        header.byteOrderMarker = 0xFEFF;
        header.unknown_0x00 = 0;
        header.encoding = 0x00;
        header.version_0x03 = 0x03;
        header.sectionCount = 4;
        header.unknown2_0x00 = 0;
        header.fileSize = 0;
        memset(&header.padding_0x00, '\0', 10);
    }

    MSBPFile MSBPFile::createNew() {
        MSBPFile newMSBP{};
        newMSBP.initNew();
        return newMSBP;
    }

    LMSError MSBPFile::loadFromBinary(std::istream& msbp) {
        LOG_AND_RETURN_IF_ERR(header.read(msbp));

        LOG_AND_RETURN_IF_ERR(colors.read(msbp));
        LOG_AND_RETURN_IF_ERR(colorLabels.read(msbp));
        LOG_AND_RETURN_IF_ERR(attributeInfo.read(msbp));
        LOG_AND_RETURN_IF_ERR(attributeLabels.read(msbp));
        LOG_AND_RETURN_IF_ERR(attributeLists.read(msbp));
        LOG_AND_RETURN_IF_ERR(tagGroups.read(msbp));
        LOG_AND_RETURN_IF_ERR(tags.read(msbp));
        LOG_AND_RETURN_IF_ERR(tagParams.read(msbp));
        LOG_AND_RETURN_IF_ERR(tagLists.read(msbp));
        LOG_AND_RETURN_IF_ERR(styles.read(msbp));
        LOG_AND_RETURN_IF_ERR(styleLabels.read(msbp));
        LOG_AND_RETURN_IF_ERR(sources.read(msbp));

        return LMSError::NONE;
    }

    LMSError MSBPFile::loadFromFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERR_AND_RETURN(LMSError::COULD_NOT_OPEN);
        }
        return loadFromBinary(file);
    }

    LMSError MSBPFile::writeToStream(std::ostream& out) {
        header.write(out);
        colors.write(out);
        colorLabels.write(out);
        attributeInfo.write(out);
        attributeLabels.write(out);
        attributeLists.write(out);
        tagGroups.write(out);
        tags.write(out);
        tagParams.write(out);
        tagLists.write(out);
        styles.write(out);
        styleLabels.write(out);
        sources.write(out);

        out.seekp(0, std::ios::end);
        header.fileSize = out.tellp();
        out.seekp(0x12, std::ios::beg);

        uint32_t fileSize_BE = Utility::Endian::toPlatform(eType::Big, header.fileSize);
        out.write(reinterpret_cast<const char*>(&fileSize_BE), sizeof(fileSize_BE)); //Update full file size

        return LMSError::NONE;
    }

    LMSError MSBPFile::writeToFile(const std::string& outFilePath) {
        std::ofstream outFile(outFilePath, std::ios::binary);
        if (!outFile.is_open()) {
            LOG_ERR_AND_RETURN(LMSError::COULD_NOT_OPEN);
        }
        return writeToStream(outFile);
    }

}
