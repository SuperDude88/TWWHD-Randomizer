#include "bflyt.hpp"

#include <cstring>
#include <unordered_set>

#include <utility/endian.hpp>
#include <utility/string.hpp>
#include <utility/math.hpp>
#include <utility/file.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;

static const std::unordered_set<std::string> sections = {
    "cnt1",
    "usd1",
    "pan1",
    "bnd1",
    "wnd1",
    "txt1",
    "pic1",
    "prt1",
    "grp1"
};



namespace NintendoWare::Layout { //"official" name was nw::lyt
    FLYTError lyt1::read(std::istream& bflyt) {
        std::streamoff sectionStart = bflyt.tellg();

        if (!bflyt.read(magicLYT1, 4)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (std::strncmp(magicLYT1, "lyt1", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&drawCentered), sizeof(drawCentered))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&width), sizeof(width))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&height), sizeof(height))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&maxPartWidth), sizeof(maxPartWidth))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&maxPartHeight), sizeof(maxPartHeight))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        if (padding_0x00[0] != 0x00 || padding_0x00[1] != 0x00 || padding_0x00[2] != 0x00) {
            LOG_ERR_AND_RETURN(FLYTError::UNEXPECTED_VALUE)
        }

        if (std::string name = Utility::Str::readNullTerminatedStr<std::string>(bflyt, bflyt.tellg()); name.empty()) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //empty string means it could not read a character from file
        }
        else {
            layoutName = name; //read the name properly
        }

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
        Utility::Endian::toPlatform_inplace(eType::Big, width);
        Utility::Endian::toPlatform_inplace(eType::Big, height);
        Utility::Endian::toPlatform_inplace(eType::Big, maxPartWidth);
        Utility::Endian::toPlatform_inplace(eType::Big, maxPartHeight);

        bflyt.seekg(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }

    FLYTError lyt1::save_changes(std::ostream& out) {
        sectionSize = roundUp<uint32_t>(0x1C + layoutName.size(), 4);

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
        Utility::Endian::toPlatform_inplace(eType::Big, width);
        Utility::Endian::toPlatform_inplace(eType::Big, height);
        Utility::Endian::toPlatform_inplace(eType::Big, maxPartWidth);
        Utility::Endian::toPlatform_inplace(eType::Big, maxPartHeight);

        out.write(magicLYT1, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize));
        out.write(reinterpret_cast<const char*>(&drawCentered), sizeof(drawCentered));
        out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));
        out.write(reinterpret_cast<const char*>(&width), sizeof(width));
        out.write(reinterpret_cast<const char*>(&height), sizeof(height));
        out.write(reinterpret_cast<const char*>(&maxPartWidth), sizeof(maxPartWidth));
        out.write(reinterpret_cast<const char*>(&maxPartHeight), sizeof(maxPartHeight));
        out.write(&layoutName[0], layoutName.size());
        padToLen(out, 4);

        return FLYTError::NONE;
    }


    FLYTError txl1::read(std::istream& bflyt) {
        std::streamoff sectionStart = bflyt.tellg();

        if (!bflyt.read(magicTXL1, 4)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        if (std::strncmp(magicTXL1, "txl1", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&numTextures), sizeof(numTextures))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
        Utility::Endian::toPlatform_inplace(eType::Big, numTextures);

        texStrOffsets.reserve(numTextures);
        for (unsigned int i = 0; i < numTextures; i++) {
            uint32_t strOffset = 0;
            if (!bflyt.read(reinterpret_cast<char*>(&strOffset), sizeof(strOffset))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            Utility::Endian::toPlatform_inplace(eType::Big, strOffset);
            texStrOffsets.push_back(strOffset);
        }

        texNames.reserve(numTextures);
        for (uint32_t strOffset : texStrOffsets) {
            if (std::string name = Utility::Str::readNullTerminatedStr<std::string>(bflyt, sectionStart + 0xC + strOffset); name.empty()) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //empty string means it could not read a character from file
            }
            else {
                texNames.push_back(name);
            }
        }

        bflyt.seekg(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }

    FLYTError txl1::save_changes(std::ostream& out) {
        std::streamoff sectionStart = out.tellp();
        
        numTextures = texNames.size();
        texStrOffsets.clear();
        texStrOffsets.reserve(numTextures);

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
        Utility::Endian::toPlatform_inplace(eType::Big, numTextures);

        out.write(magicTXL1, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize)); //will be inaccurate
        out.write(reinterpret_cast<const char*>(&numTextures), sizeof(numTextures));
        out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));

        {
            uint32_t strOffset = 0x4 * texNames.size();
            Utility::seek(out, sectionStart + 0xC + strOffset, std::ios::beg);
            for (const std::string& name : texNames) {
                texStrOffsets.push_back(strOffset);
                out.write(&name[0], name.size());
                strOffset += name.size();
            }
            padToLen(out, 4);
            sectionSize = out.tellp() - sectionStart; //section ends after names
        }

        out.seekp(sectionStart + 0xC);
        for (uint32_t& strOffset : texStrOffsets) { //write updated offsets
            Utility::Endian::toPlatform_inplace(eType::Big, strOffset);
            out.write(reinterpret_cast<const char*>(&strOffset), sizeof(strOffset));
        }

        uint32_t sectionSize_BE = Utility::Endian::toPlatform(eType::Big, sectionSize);
        out.seekp(sectionStart + 0x4, std::ios::beg);
        out.write(reinterpret_cast<const char*>(&sectionSize_BE), sizeof(sectionSize_BE)); //update value

        out.seekp(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }


    FLYTError fnl1::read(std::istream& bflyt) {
        std::streamoff sectionStart = bflyt.tellg();

        if (!bflyt.read(magicFNL1, 4)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        if (std::strncmp(magicFNL1, "fnl1", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&numFonts), sizeof(numFonts))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
        Utility::Endian::toPlatform_inplace(eType::Big, numFonts);

        fontStrOffsets.reserve(numFonts);
        for (unsigned int i = 0; i < numFonts; i++) {
            uint32_t strOffset = 0;
            if (!bflyt.read(reinterpret_cast<char*>(&strOffset), sizeof(strOffset))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            Utility::Endian::toPlatform_inplace(eType::Big, strOffset);
            fontStrOffsets.push_back(strOffset);
        }

        fontNames.reserve(numFonts);
        for (uint32_t strOffset : fontStrOffsets) {
            if (std::string name = Utility::Str::readNullTerminatedStr<std::string>(bflyt, sectionStart + 0xC + strOffset); name.empty()) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //empty string means it could not read a character from file
            }
            else {
                fontNames.push_back(name);
            }
        }

        bflyt.seekg(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }

    FLYTError fnl1::save_changes(std::ostream& out) {
        std::streamoff sectionStart = out.tellp();

        numFonts = fontNames.size();
        fontStrOffsets.clear();
        fontStrOffsets.reserve(numFonts);

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
        Utility::Endian::toPlatform_inplace(eType::Big, numFonts);

        out.write(magicFNL1, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize)); //will be inaccurate
        out.write(reinterpret_cast<const char*>(&numFonts), sizeof(numFonts));
        out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));

        {
            uint32_t strOffset = 0x4 * fontNames.size();
            Utility::seek(out, sectionStart + 0xC + strOffset, std::ios::beg);
            for (const std::string& name : fontNames) {
                fontStrOffsets.push_back(strOffset);
                out.write(&name[0], name.size());
                strOffset += name.size();
            }
            padToLen(out, 4);
            sectionSize = out.tellp() - sectionStart; //section ends after names
        }

        out.seekp(sectionStart + 0xC);
        for (uint32_t& strOffset : fontStrOffsets) { //write updated offsets
            Utility::Endian::toPlatform_inplace(eType::Big, strOffset);
            out.write(reinterpret_cast<const char*>(&strOffset), sizeof(strOffset));
        }

        uint32_t sectionSize_BE = Utility::Endian::toPlatform(eType::Big, sectionSize);
        out.seekp(sectionStart + 0x4, std::ios::beg);
        out.write(reinterpret_cast<const char*>(&sectionSize_BE), sizeof(sectionSize_BE)); //update value

        out.seekp(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }


    FLYTError material::read(std::istream& bflyt) {
        name.resize(0x1C);
        if (!bflyt.read(&name[0], 0x1C)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)

        if (!readRGBA(bflyt, bflyt.tellg(), blackColor)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!readRGBA(bflyt, bflyt.tellg(), whiteColor)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!bflyt.read(reinterpret_cast<char*>(&flags), sizeof(flags))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)

        Utility::Endian::toPlatform_inplace(eType::Big, flags);

        uint32_t texCount = flags & 3;
        uint32_t mtxCount = (flags >> 2) & 3;
        uint32_t texCoordGenCount = (flags >> 4) & 3;
        uint32_t tevStageCount = (flags >> 6) & 7;
        uint32_t enableAlphaCompare = (flags >> 9) & 1;
        uint32_t enableBlend = (flags >> 10) & 1;
        textureOnly = (flags >> 11) & 1;
        uint32_t blendLogic = (flags >> 12) & 1;
        uint32_t indParams = (flags >> 14) & 1;
        uint32_t projMapCount = (flags >> 15) & 3;
        uint32_t fontShadowParams = (flags >> 17) & 1;
        alphaInterpolation = (flags >> 18) & 1;

        texRefs.reserve(texCount);
        for (unsigned int i = 0; i < texCount; i++) {
            texRef ref{};
            if (!bflyt.read(reinterpret_cast<char*>(&ref.nameIndex), sizeof(ref.nameIndex))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&ref.wrapModeU), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&ref.wrapModeV), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)

            Utility::Endian::toPlatform_inplace(eType::Big, ref.nameIndex);

            texRefs.push_back(ref);
        }

        texTransforms.reserve(mtxCount);
        for (unsigned int i = 0; i < mtxCount; i++) {
            texTransform& transform = texTransforms.emplace_back();

            if (!readVec2(bflyt, bflyt.tellg(), transform.translation)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            if (!bflyt.read(reinterpret_cast<char*>(&transform.rotation), sizeof(transform.rotation))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)

            if (!readVec2(bflyt, bflyt.tellg(), transform.scale)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            Utility::Endian::toPlatform_inplace(eType::Big, transform.rotation);
        }

        texCoordGens.reserve(texCoordGenCount);
        for (unsigned int i = 0; i < texCoordGenCount; i++) {
            texCoordGen coordGen{};
            if (!bflyt.read(reinterpret_cast<char*>(&coordGen.matrix), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&coordGen.source), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&coordGen.unk), sizeof(coordGen.unk))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)

            texCoordGens.push_back(coordGen);
        }

        tevStages.reserve(tevStageCount);
        for (unsigned int i = 0; i < tevStageCount; i++) {
            tevStage stage{};
            if (!bflyt.read(reinterpret_cast<char*>(&stage.colorMode), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&stage.alphaMode), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&stage.padding_0x00), sizeof(stage.padding_0x00))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)

            tevStages.push_back(stage);
        }

        if (enableAlphaCompare) {
            alphaCompare compare{};
            if (!bflyt.read(reinterpret_cast<char*>(&compare.compareMode), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&compare.unk), 3)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&compare.value), sizeof(compare.value))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)

            Utility::Endian::toPlatform_inplace(eType::Big, compare.value);

            alphaComparison = compare;
        }

        if (enableBlend) {
            blendMode blend{};
            if (!bflyt.read(reinterpret_cast<char*>(&blend.blendOp), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&blend.sourceFactor), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&blend.destFactor), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&blend.logicOp), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)

            blendingMode = blend;
        }

        if (blendLogic) {
            blendMode blend{};
            if (!bflyt.read(reinterpret_cast<char*>(&blend.blendOp), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&blend.sourceFactor), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&blend.destFactor), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&blend.logicOp), 1)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)

            blendModeLogic = blend;
        }

        if (indParams) {
            indirectParam param{};
            if (!bflyt.read(reinterpret_cast<char*>(&param.rotation), sizeof(param.rotation))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&param.scaleX), sizeof(param.scaleX))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&param.scaleY), sizeof(param.scaleY))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)

            Utility::Endian::toPlatform_inplace(eType::Big, param.rotation);
            Utility::Endian::toPlatform_inplace(eType::Big, param.scaleX);
            Utility::Endian::toPlatform_inplace(eType::Big, param.scaleY);

            indParameter = param;
        }

        projectionMaps.reserve(projMapCount);
        for (unsigned int i = 0; i < projMapCount; i++) {
            projectionMap map{};
            if (!bflyt.read(reinterpret_cast<char*>(&map.posX), sizeof(map.posX))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&map.posY), sizeof(map.posY))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&map.scaleX), sizeof(map.scaleX))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&map.scaleY), sizeof(map.scaleY))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&map.flags), sizeof(map.flags))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            if (!bflyt.read(reinterpret_cast<char*>(&map.unk), sizeof(map.unk))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)

            Utility::Endian::toPlatform_inplace(eType::Big, map.posX);
            Utility::Endian::toPlatform_inplace(eType::Big, map.posY);
            Utility::Endian::toPlatform_inplace(eType::Big, map.scaleX);
            Utility::Endian::toPlatform_inplace(eType::Big, map.scaleY);

            projectionMaps.push_back(map);
        }

        if (fontShadowParams) {
            fontShadowParameter& param = fontShadowParam.emplace();

            if (!readRGBA(bflyt, bflyt.tellg(), param.blackColor)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
            }

            if (!readRGBA(bflyt, bflyt.tellg(), param.whiteColor)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
            }
        }

        return FLYTError::NONE;
    }

    FLYTError material::save_changes(std::ostream& out) {
        std::streamoff beg = out.tellp();
        out.write(&name[0], 0x1C);

        writeRGBA(out, blackColor);
        writeRGBA(out, whiteColor);

        Utility::seek(out, 4, std::ios::cur); //skip over flags for now

        setFlag(texRefs.size(), 0x00000003, 0);
        for (texRef& ref : texRefs) {
            Utility::Endian::toPlatform_inplace(eType::Big, ref.nameIndex);

            out.write(reinterpret_cast<const char*>(&ref.nameIndex), sizeof(ref.nameIndex));
            out.write(reinterpret_cast<const char*>(&ref.wrapModeU), 1);
            out.write(reinterpret_cast<const char*>(&ref.wrapModeV), 1);
        }

        setFlag(texTransforms.size(), 0x0000000C, 2);
        for (texTransform& transform : texTransforms) {
            Utility::Endian::toPlatform_inplace(eType::Big, transform.rotation);

            writeVec2(out, transform.translation);

            out.write(reinterpret_cast<const char*>(&transform.rotation), sizeof(transform.rotation));

            writeVec2(out, transform.scale);
        }

        setFlag(texCoordGens.size(), 0x00000030, 4);
        for (const texCoordGen& coordGen : texCoordGens) {
            out.write(reinterpret_cast<const char*>(&coordGen.matrix), 1);
            out.write(reinterpret_cast<const char*>(&coordGen.source), 1);
            out.write(reinterpret_cast<const char*>(&coordGen.unk), sizeof(coordGen.unk));
        }

        setFlag(tevStages.size(), 0x000001C0, 6);
        for (const tevStage& stage : tevStages) {
            out.write(reinterpret_cast<const char*>(&stage.colorMode), 1);
            out.write(reinterpret_cast<const char*>(&stage.alphaMode), 1);
            out.write(reinterpret_cast<const char*>(&stage.padding_0x00), sizeof(stage.padding_0x00));
        }

        setFlag(false, 0x00000200, 9);
        if (alphaComparison.has_value()) {
            setFlag(true, 0x00000200, 9);
            alphaCompare& compare = alphaComparison.value();

            Utility::Endian::toPlatform_inplace(eType::Big, compare.value);

            out.write(reinterpret_cast<const char*>(&compare.compareMode), 1);
            out.write(reinterpret_cast<const char*>(&compare.unk), 3);
            out.write(reinterpret_cast<const char*>(&compare.value), sizeof(compare.value));
        }

        setFlag(false, 0x00000400, 10);
        if (blendingMode.has_value()) {
            setFlag(true, 0x00000400, 10);
            const blendMode& blend = blendingMode.value();

            out.write(reinterpret_cast<const char*>(&blend.blendOp), 1);
            out.write(reinterpret_cast<const char*>(&blend.sourceFactor), 1);
            out.write(reinterpret_cast<const char*>(&blend.destFactor), 1);
            out.write(reinterpret_cast<const char*>(&blend.logicOp), 1);
        }

        setFlag(textureOnly, 0x00000800, 11);

        setFlag(false, 0x00001000, 12);
        if (blendModeLogic.has_value()) {
            setFlag(true, 0x00001000, 12);
            const blendMode& blend = blendModeLogic.value();

            out.write(reinterpret_cast<const char*>(&blend.blendOp), 1);
            out.write(reinterpret_cast<const char*>(&blend.sourceFactor), 1);
            out.write(reinterpret_cast<const char*>(&blend.destFactor), 1);
            out.write(reinterpret_cast<const char*>(&blend.logicOp), 1);
        }

        setFlag(false, 0x00004000, 14);
        if (indParameter.has_value()) {
            setFlag(true, 0x00004000, 14);
            indirectParam& param = indParameter.value();

            Utility::Endian::toPlatform_inplace(eType::Big, param.rotation);
            Utility::Endian::toPlatform_inplace(eType::Big, param.scaleX);
            Utility::Endian::toPlatform_inplace(eType::Big, param.scaleY);

            out.write(reinterpret_cast<const char*>(&param.rotation), sizeof(param.rotation));
            out.write(reinterpret_cast<const char*>(&param.scaleX), sizeof(param.scaleX));
            out.write(reinterpret_cast<const char*>(&param.scaleY), sizeof(param.scaleY));
        }

        setFlag(projectionMaps.size(), 0x00018000, 15);
        for (projectionMap& map : projectionMaps) {
            Utility::Endian::toPlatform_inplace(eType::Big, map.posX);
            Utility::Endian::toPlatform_inplace(eType::Big, map.posY);
            Utility::Endian::toPlatform_inplace(eType::Big, map.scaleX);
            Utility::Endian::toPlatform_inplace(eType::Big, map.scaleY);

            out.write(reinterpret_cast<const char*>(&map.posX), sizeof(map.posX));
            out.write(reinterpret_cast<const char*>(&map.posY), sizeof(map.posY));
            out.write(reinterpret_cast<const char*>(&map.scaleX), sizeof(map.scaleX));
            out.write(reinterpret_cast<const char*>(&map.scaleY), sizeof(map.scaleY));
            out.write(reinterpret_cast<const char*>(&map.flags), sizeof(map.flags));
            out.write(reinterpret_cast<const char*>(&map.unk), sizeof(map.unk));
        }

        setFlag(false, 0x00020000, 17);
        if (fontShadowParam.has_value()) {
            setFlag(true, 0x00020000, 17);

            writeRGBA(out, blackColor);
            writeRGBA(out, whiteColor);
        }

        setFlag(alphaInterpolation, 0x00040000, 18);

        std::streamoff end = out.tellp();
        out.seekp(beg + 0x24, std::ios::beg);

        Utility::Endian::toPlatform_inplace(eType::Big, flags);
        out.write(reinterpret_cast<const char*>(&flags), sizeof(flags)); //write flags

        out.seekp(end, std::ios::beg); //seek back to end of material so things are written sequentially
        return FLYTError::NONE;
    }


    FLYTError mat1::read(std::istream& bflyt) {
        std::streamoff sectionStart = bflyt.tellg();

        if (!bflyt.read(magicMAT1, 4)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        if (std::strncmp(magicMAT1, "mat1", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&numMats), sizeof(numMats))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
        Utility::Endian::toPlatform_inplace(eType::Big, numMats);

        matOffsets.reserve(numMats);
        for (unsigned int i = 0; i < numMats; i++) {
            uint32_t matOffset;
            if (!bflyt.read(reinterpret_cast<char*>(&matOffset), sizeof(matOffset))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            Utility::Endian::toPlatform_inplace(eType::Big, matOffset);
            matOffsets.push_back(matOffset);
        }

        materials.reserve(numMats);
        for (const uint32_t& matOffset : matOffsets) {
            bflyt.seekg(sectionStart + matOffset, std::ios::beg);
            LOG_AND_RETURN_IF_ERR(materials.emplace_back().read(bflyt));
        }

        bflyt.seekg(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }

    FLYTError mat1::save_changes(std::ostream& out) {
        std::streamoff sectionStart = out.tellp();

        numMats = materials.size();
        matOffsets.clear();
        matOffsets.reserve(numMats);

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
        Utility::Endian::toPlatform_inplace(eType::Big, numMats);

        out.write(magicMAT1, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize)); //will be inaccurate
        out.write(reinterpret_cast<const char*>(&numMats), sizeof(numMats));
        out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));

        {
            uint32_t matOffset = 0xC + 0x4 * materials.size();
            Utility::seek(out, sectionStart + matOffset, std::ios::beg);
            for (material& mat : materials) {
                matOffsets.push_back(matOffset);
                LOG_AND_RETURN_IF_ERR(mat.save_changes(out));

                matOffset = out.tellp() - sectionStart;
            }
            padToLen(out, 4);
            sectionSize = out.tellp() - sectionStart; //section ends after materials
        }

        out.seekp(sectionStart + 0xC);
        for (uint32_t& matOffset : matOffsets) { //write updated offsets
            Utility::Endian::toPlatform_inplace(eType::Big, matOffset);
            out.write(reinterpret_cast<const char*>(&matOffset), sizeof(matOffset));
        }

        uint32_t sectionSize_BE = Utility::Endian::toPlatform(eType::Big, sectionSize);
        out.seekp(sectionStart + 0x4, std::ios::beg);
        out.write(reinterpret_cast<const char*>(&sectionSize_BE), sizeof(sectionSize_BE)); //update value

        out.seekp(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }


    FLYTError usd1::read(std::istream& bflyt) {
        std::streamoff sectionStart = bflyt.tellg();

        if (!bflyt.read(magic, 4)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        if (std::strncmp(magic, "usd1", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&numEntries), sizeof(numEntries))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
        Utility::Endian::toPlatform_inplace(eType::Big, numEntries);

        for (unsigned int i = 0; i < numEntries; i++) {
            std::streamoff entryStart = bflyt.tellg();
            userDataEntry entry;
            if (!bflyt.read(reinterpret_cast<char*>(&entry.nameOffset), sizeof(entry.nameOffset))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&entry.dataOffset), sizeof(entry.dataOffset))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&entry.dataLen), sizeof(entry.dataLen))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&entry.dataType), 1)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&entry.unk), sizeof(entry.unk))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            Utility::Endian::toPlatform_inplace(eType::Big, entry.nameOffset);
            Utility::Endian::toPlatform_inplace(eType::Big, entry.dataOffset);
            Utility::Endian::toPlatform_inplace(eType::Big, entry.dataLen);

            if (entry.nameOffset != 0) {
                if (std::string name = Utility::Str::readNullTerminatedStr<std::string>(bflyt, entryStart + entry.nameOffset); name.empty()) {
                    LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //empty string means it could not read a character from file
                }
                else {
                    entry.name = name;
                }
            }

            if (entry.dataOffset != 0) {
                bflyt.seekg(entryStart + entry.dataOffset, std::ios::beg);
                switch (entry.dataType) {
                case UserDataType::STRING:
                    if (entry.dataLen != 0) {
                        std::string data;
                        data.resize(entry.dataLen);
                        if (!bflyt.read(reinterpret_cast<char*>(&data[0]), entry.dataLen)) {
                            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
                        }
                        entry.data = data;
                    }
                    else {
                        if (std::string data = Utility::Str::readNullTerminatedStr<std::string>(bflyt, sectionStart + entry.dataOffset); data.empty()) {
                            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //empty string means it could not read a character from file
                        }
                        else {
                            entry.data = data;
                        }
                    }
                    break;
                case UserDataType::INT:
                    entry.data.emplace<std::vector<int32_t>>();
                    std::get<std::vector<int32_t>>(entry.data).reserve(entry.dataLen);
                    for (unsigned int x = 0; x < entry.dataLen; x++) {
                        int32_t value;
                        if (!bflyt.read(reinterpret_cast<char*>(&value), sizeof(value))) {
                            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
                        }

                        Utility::Endian::toPlatform_inplace(eType::Big, value);

                        std::get<std::vector<int32_t>>(entry.data).push_back(value);
                    }
                    break;
                case UserDataType::FLOAT:
                    entry.data.emplace<std::vector<float>>();
                    std::get<std::vector<float>>(entry.data).reserve(entry.dataLen);
                    for (unsigned int x = 0; x < entry.dataLen; x++) {
                        float value;
                        if (!bflyt.read(reinterpret_cast<char*>(&value), sizeof(value))) {
                            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
                        }

                        Utility::Endian::toPlatform_inplace(eType::Big, value);

                        std::get<std::vector<float>>(entry.data).push_back(value);
                    }
                    break;
                case UserDataType::STRUCT:
                    break; //type not implemented, should not appear in Wii U files
                }
            }

            entries.push_back(entry);
        }

        bflyt.seekg(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }

    FLYTError usd1::save_changes(std::ostream& out) {
        std::streamoff sectionStart = out.tellp();

        numEntries = entries.size();
        Utility::Endian::toPlatform_inplace(eType::Big, numEntries);

        out.write(magic, 4);
        Utility::seek(out, 4, std::ios::cur); //skip section size for now
        out.write(reinterpret_cast<const char*>(&numEntries), sizeof(numEntries));
        out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));
        for (userDataEntry& entry : entries) {
            std::streamoff entryStart = out.tellp();
            entry.dataType = static_cast<UserDataType>(entry.data.index());
            Utility::seek(out, 0xA, std::ios::cur);
            out.write(reinterpret_cast<const char*>(&entry.dataType), 1);
            out.write(reinterpret_cast<const char*>(&entry.unk), sizeof(entry.unk));
            entry.dataOffset = out.tellp() - entryStart;
            switch (entry.dataType) {
            case UserDataType::STRING:
                out.write(&std::get<std::string>(entry.data)[0], std::get<std::string>(entry.data).size());
                padToLen(out, 4); //might not be padded, probably is though
                entry.dataLen = static_cast<uint32_t>(out.tellp()) - entry.dataOffset - entryStart;
                break;
            case UserDataType::INT:
                entry.dataLen = std::get<std::vector<int32_t>>(entry.data).size();
                for (int32_t& value : std::get<std::vector<int32_t>>(entry.data)) {
                    Utility::Endian::toPlatform_inplace(eType::Big, value);
                    out.write(reinterpret_cast<const char*>(&value), sizeof(value));
                }
                break;
            case UserDataType::FLOAT:
                entry.dataLen = std::get<std::vector<float>>(entry.data).size();
                for (float& value : std::get<std::vector<float>>(entry.data)) {
                    Utility::Endian::toPlatform_inplace(eType::Big, value);
                    out.write(reinterpret_cast<const char*>(&value), sizeof(value));
                }
                break;
            case UserDataType::STRUCT:
                LOG_ERR_AND_RETURN(FLYTError::UNEXPECTED_VALUE)
            }
            entry.nameOffset = out.tellp() - entryStart;
            out.write(&entry.name[0], entry.name.size());
            padToLen(out, 4);
            std::streamoff entryEnd = out.tellp();
            out.seekp(entryStart, std::ios::beg);

            Utility::Endian::toPlatform_inplace(eType::Big, entry.nameOffset);
            Utility::Endian::toPlatform_inplace(eType::Big, entry.dataOffset);
            Utility::Endian::toPlatform_inplace(eType::Big, entry.dataLen);

            out.write(reinterpret_cast<const char*>(&entry.nameOffset), sizeof(entry.nameOffset));
            out.write(reinterpret_cast<const char*>(&entry.dataOffset), sizeof(entry.dataOffset));
            out.write(reinterpret_cast<const char*>(&entry.dataLen), sizeof(entry.dataLen));
            out.seekp(entryEnd, std::ios::beg);
        }

        sectionSize = out.tellp() - sectionStart;
        out.seekp(sectionStart + 0x4, std::ios::beg);
        uint32_t sectionSize_BE = Utility::Endian::toPlatform(eType::Big, sectionSize);
        out.write(reinterpret_cast<const char*>(&sectionSize_BE), sizeof(sectionSize_BE)); //update value

        out.seekp(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }


    FLYTError cnt1::read(std::istream& bflyt) {
        std::streamoff sectionStart = bflyt.tellg();

        if (!bflyt.read(magic, 4)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        if (std::strncmp(magic, "cnt1", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&paneNamesOffset), sizeof(paneNamesOffset))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&paneCount), sizeof(paneCount))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&animCount), sizeof(animCount))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
        Utility::Endian::toPlatform_inplace(eType::Big, paneNamesOffset);
        Utility::Endian::toPlatform_inplace(eType::Big, paneCount);
        Utility::Endian::toPlatform_inplace(eType::Big, animCount);

        if (std::string name = Utility::Str::readNullTerminatedStr<std::string>(bflyt, bflyt.tellg()); name.empty()) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //empty string means it could not read a character from file
        }
        else {
            this->name = name;
        }

        bflyt.seekg(sectionStart + paneNamesOffset, std::ios::beg);
        paneNames.reserve(paneCount);
        for (unsigned int i = 0; i < paneCount; i++) {
            std::string paneName;
            paneName.resize(0x18);
            if (!bflyt.read(&paneName[0], 0x18)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            paneNames.push_back(paneName);
        }

        LOG_AND_RETURN_IF_ERR(readPadding<FLYTError>(bflyt, 4));
        animNameTableOffset = bflyt.tellg() - sectionStart;

        animNameOffsets.reserve(animCount);
        for (unsigned int i = 0; i < animCount; i++) {
            uint32_t nameOffset = 0;
            if (!bflyt.read(reinterpret_cast<char*>(&nameOffset), sizeof(nameOffset))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            Utility::Endian::toPlatform_inplace(eType::Big, nameOffset);
            animNameOffsets.push_back(nameOffset);
        }

        animNames.reserve(animCount);
        for (const uint32_t nameOffset : animNameOffsets) {
            if (std::string name = Utility::Str::readNullTerminatedStr<std::string>(bflyt, sectionStart + animNameTableOffset + nameOffset); name.empty()) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //empty string means it could not read a character from file
            }
            else {
                animNames.push_back(name);
            }
        }

        bflyt.seekg(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }

    FLYTError cnt1::save_changes(std::ostream& out) {
        std::streamoff sectionStart = out.tellp();

        paneCount = paneNames.size();
        animCount = animNames.size();

        //Utility::Endian::toPlatform_inplace(eType::Big, paneNamesOffset);
        Utility::Endian::toPlatform_inplace(eType::Big, paneCount);
        Utility::Endian::toPlatform_inplace(eType::Big, animCount);

        out.write(magic, 4);
        Utility::seek(out, 8, std::ios::cur); //skip section size and pane names offset
        out.write(reinterpret_cast<const char*>(&paneCount), sizeof(paneCount));
        out.write(reinterpret_cast<const char*>(&animCount), sizeof(animCount));
        out.write(&name[0], name.size());
        padToLen(out, 4);

        paneNamesOffset = out.tellp() - sectionStart;

        for (const std::string& paneName : paneNames) {
            out.write(&paneName[0], 0x18);
        }

        padToLen(out, 4);
        animNameTableOffset = out.tellp() - sectionStart;

        animNameOffsets.clear();
        animNameOffsets.reserve(animCount);
        {
            uint32_t nameOffset = animNames.size() * 0x4;
            for (const std::string& animName : animNames) {
                animNameOffsets.push_back(nameOffset);
                out.write(&animName[0], animName.size());
                nameOffset += animName.size();
            }
            padToLen(out, 4);
            sectionSize = out.tellp() - sectionStart; //section ends after materials
        }

        uint32_t sectionSize_BE = Utility::Endian::toPlatform(eType::Big, sectionSize);
        out.seekp(sectionStart + 0x4, std::ios::beg);
        out.write(reinterpret_cast<const char*>(&sectionSize_BE), sizeof(sectionSize_BE)); //update value

        Utility::Endian::toPlatform_inplace(eType::Big, paneNamesOffset);
        out.write(reinterpret_cast<const char*>(&paneNamesOffset), sizeof(paneNamesOffset));

        out.seekp(sectionStart + animNameTableOffset, std::ios::beg);
        for (uint32_t nameOffset : animNameOffsets) {
            Utility::Endian::toPlatform_inplace(eType::Big, nameOffset);
            out.write(reinterpret_cast<const char*>(&nameOffset), sizeof(nameOffset));
        }

        out.seekp(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }


    FLYTError grp1::read(std::istream& bflyt) {
        if (!bflyt.read(magic, 4)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        if (std::strncmp(magic, "grp1", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        groupName.resize(0x18);
        if (!bflyt.read(reinterpret_cast<char*>(&groupName[0]), 0x18)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&numPanes), sizeof(numPanes))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
        Utility::Endian::toPlatform_inplace(eType::Big, numPanes);

        paneNames.reserve(numPanes);
        for (unsigned int i = 0; i < numPanes; i++) {
            std::string paneName;
            paneName.resize(0x18);
            if (!bflyt.read(&paneName[0], 0x18)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            paneNames.push_back(paneName);
        }

        char nextSection[4];
        if (!bflyt.read(reinterpret_cast<char*>(&nextSection), sizeof(nextSection))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (std::strncmp(nextSection, "grs1", 4) == 0) {
            uint32_t grs1Size;
            if (!bflyt.read(reinterpret_cast<char*>(&grs1Size), sizeof(grs1Size))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            Utility::Endian::toPlatform_inplace(eType::Big, grs1Size);
            if (grs1Size != 0x8) {
                LOG_ERR_AND_RETURN(FLYTError::UNEXPECTED_VALUE)
            }

            if (!bflyt.read(reinterpret_cast<char*>(&nextSection), sizeof(nextSection))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            while (std::strncmp(nextSection, "gre1", 4) != 0) {
                bflyt.seekg(-4, std::ios::cur);

                if (std::strncmp(nextSection, "grp1", 4) != 0) LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION) //should never happen

                LOG_AND_RETURN_IF_ERR(children.emplace_back().read(bflyt));

                if (!bflyt.read(reinterpret_cast<char*>(&nextSection), sizeof(nextSection))) {
                    LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
                }
            }

            uint32_t gre1Size;
            if (!bflyt.read(reinterpret_cast<char*>(&gre1Size), sizeof(gre1Size))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            Utility::Endian::toPlatform_inplace(eType::Big, gre1Size);
            if (gre1Size != 0x8) {
                LOG_ERR_AND_RETURN(FLYTError::UNEXPECTED_VALUE)
            }
        }
        else {
            bflyt.seekg(-4, std::ios::cur);
        }

        return FLYTError::NONE;
    }

    FLYTError grp1::save_changes(std::ostream& out, uint16_t& sectionNum) {
        sectionNum += 1; //add each group to section count

        numPanes = paneNames.size();
        sectionSize = 0x24 + numPanes * 0x18;

        Utility::Endian::toPlatform_inplace(eType::Big, numPanes);
        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        out.write(magic, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize));
        out.write(&groupName[0], 0x18);
        out.write(reinterpret_cast<const char*>(&numPanes), sizeof(numPanes));
        out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));

        for (const std::string& paneName : paneNames) {
            out.write(&paneName[0], 0x18);
        }

        if (!children.empty()) {
            uint32_t size = 0x8;
            Utility::Endian::toPlatform_inplace(eType::Big, size);

            out.write("grs1", 4);
            out.write(reinterpret_cast<const char*>(&size), sizeof(size));
            sectionNum += 1; //grs counts as section

            for (grp1& group : children) {
                LOG_AND_RETURN_IF_ERR(group.save_changes(out, sectionNum));
            }

            out.write("gre1", 4);
            out.write(reinterpret_cast<const char*>(&size), sizeof(size));
            sectionNum += 1; //gre counts as section
        }

        return FLYTError::NONE;
    }

    FLYTError PaneBase::read(std::istream& bflyt) {
        if (!bflyt.read(magic, 4)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&bitFlags), 1)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&originFlags), 1)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&alpha), 1)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&paneMagFlags), 1)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        name.resize(0x18);
        userInfo.resize(0x8);
        if (!bflyt.read(&name[0], 0x18)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(&userInfo[0], 0x8)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        
        if (!readVec3(bflyt, bflyt.tellg(), translation)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
    
        if (!readVec3(bflyt, bflyt.tellg(), rotation)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
    
        if (!readVec2(bflyt, bflyt.tellg(), scale)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
    
        if (!bflyt.read(reinterpret_cast<char*>(&width), sizeof(width))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&height), sizeof(height))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
    
        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
        Utility::Endian::toPlatform_inplace(eType::Big, width);
        Utility::Endian::toPlatform_inplace(eType::Big, height);
    
        return FLYTError::NONE;
    }

    std::unique_ptr<PaneBase> PaneBase::clonePane() {
        return std::make_unique<PaneBase>(*this);
    }

    FLYTError PaneBase::save_changes(std::ostream& out) {
        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);
        Utility::Endian::toPlatform_inplace(eType::Big, width);
        Utility::Endian::toPlatform_inplace(eType::Big, height);

        out.write(magic, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize));
        out.write(reinterpret_cast<const char*>(&bitFlags), sizeof(bitFlags));
        out.write(reinterpret_cast<const char*>(&originFlags), sizeof(originFlags));
        out.write(reinterpret_cast<const char*>(&alpha), sizeof(alpha));
        out.write(reinterpret_cast<const char*>(&paneMagFlags), sizeof(paneMagFlags));
        out.write(&name[0], 0x18);
        out.write(&userInfo[0], 0x8);

        writeVec3(out, translation);
        writeVec3(out, rotation);
        writeVec2(out, scale);

        out.write(reinterpret_cast<const char*>(&width), sizeof(width));
        out.write(reinterpret_cast<const char*>(&height), sizeof(height));

        return FLYTError::NONE;
    }

    FLYTError pan1::read(std::istream& bflyt) {
        std::streamoff sectionStart = bflyt.tellg();

        LOG_AND_RETURN_IF_ERR(PaneBase::read(bflyt));
        if (std::strncmp(magic, "pan1", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
        }
        if (sectionSize != 0x54) {
            LOG_ERR_AND_RETURN(FLYTError::UNEXPECTED_VALUE)
        }

        bflyt.seekg(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }

    std::unique_ptr<PaneBase> pan1::clonePane() {
        return std::make_unique<pan1>(*this);
    }

    FLYTError pan1::save_changes(std::ostream& out) {
        LOG_AND_RETURN_IF_ERR(PaneBase::save_changes(out));
        return FLYTError::NONE;
    }


    FLYTError bnd1::read(std::istream& bflyt) {
        std::streamoff sectionStart = bflyt.tellg();

        LOG_AND_RETURN_IF_ERR(PaneBase::read(bflyt));
        if (std::strncmp(magic, "bnd1", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
        }
        if (sectionSize != 0x54) {
            LOG_ERR_AND_RETURN(FLYTError::UNEXPECTED_VALUE)
        }

        bflyt.seekg(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }

    std::unique_ptr<PaneBase> bnd1::clonePane() {
        return std::make_unique<bnd1>(*this);
    }

    FLYTError bnd1::save_changes(std::ostream& out) {
        LOG_AND_RETURN_IF_ERR(PaneBase::save_changes(out));
        return FLYTError::NONE;
    }

    FLYTError wnd1::read(std::istream& bflyt) {
        std::streamoff sectionStart = bflyt.tellg();
        
        LOG_AND_RETURN_IF_ERR(PaneBase::read(bflyt));
        if (std::strncmp(magic, "wnd1", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
        }

        if (!bflyt.read(reinterpret_cast<char*>(&leftStretch), sizeof(leftStretch))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&rightStretch), sizeof(rightStretch))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&topStretch), sizeof(topStretch))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&bottomStretch), sizeof(bottomStretch))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&frameSizeLeft), sizeof(frameSizeLeft))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&frameSizeRight), sizeof(frameSizeRight))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&frameSizeTop), sizeof(frameSizeTop))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&frameSizeBottom), sizeof(frameSizeBottom))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&frameNum), sizeof(frameNum))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&wndFlags), sizeof(wndFlags))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&contentOffset), sizeof(contentOffset))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&frameTableOffset), sizeof(frameTableOffset))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, leftStretch);
        Utility::Endian::toPlatform_inplace(eType::Big, rightStretch);
        Utility::Endian::toPlatform_inplace(eType::Big, topStretch);
        Utility::Endian::toPlatform_inplace(eType::Big, bottomStretch);
        Utility::Endian::toPlatform_inplace(eType::Big, frameSizeLeft);
        Utility::Endian::toPlatform_inplace(eType::Big, frameSizeRight);
        Utility::Endian::toPlatform_inplace(eType::Big, frameSizeTop);
        Utility::Endian::toPlatform_inplace(eType::Big, frameSizeBottom);
        Utility::Endian::toPlatform_inplace(eType::Big, contentOffset);
        Utility::Endian::toPlatform_inplace(eType::Big, frameTableOffset);

        bflyt.seekg(sectionStart + contentOffset, std::ios::beg);
        if (!readRGBA(bflyt, bflyt.tellg(), content.vertexColorTL)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!readRGBA(bflyt, bflyt.tellg(), content.vertexColorTR)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!readRGBA(bflyt, bflyt.tellg(), content.vertexColorBL)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!readRGBA(bflyt, bflyt.tellg(), content.vertexColorBR)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!bflyt.read(reinterpret_cast<char*>(&content.matIndex), sizeof(content.matIndex))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&content.numCoords), sizeof(content.numCoords))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&content.padding_0x00), sizeof(content.padding_0x00))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, content.matIndex);

        content.coords.reserve(content.numCoords);
        for (uint8_t i = 0; i < content.numCoords; i++) {
            UVCoords& coord = content.coords.emplace_back();
            if (!readVec2(bflyt, bflyt.tellg(), coord.coordTL)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            if (!readVec2(bflyt, bflyt.tellg(), coord.coordTR)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            if (!readVec2(bflyt, bflyt.tellg(), coord.coordBL)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            if (!readVec2(bflyt, bflyt.tellg(), coord.coordBR)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
        }


        bflyt.seekg(sectionStart + frameTableOffset);
        frameTable.reserve(frameNum);
        for (unsigned int i = 0; i < frameNum; i++) {
            uint32_t frameOffset = 0;
            if (!bflyt.read(reinterpret_cast<char*>(&frameOffset), sizeof(frameOffset))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            Utility::Endian::toPlatform_inplace(eType::Big, frameOffset);
            frameTable.push_back(frameOffset);
        }

        frames.reserve(frameNum);
        for (const uint32_t& frameOffset : frameTable) {
            bflyt.seekg(sectionStart + frameOffset, std::ios::beg);

            windowFrame frame{};
            if (!bflyt.read(reinterpret_cast<char*>(&frame.matIndex), sizeof(frame.matIndex))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&frame.texFlip), sizeof(frame.texFlip))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&frame.padding_0x00), sizeof(frame.padding_0x00))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            Utility::Endian::toPlatform_inplace(eType::Big, frame.matIndex);

            frames.push_back(frame);
        }

        bflyt.seekg(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }

    std::unique_ptr<PaneBase> wnd1::clonePane() {
        return std::make_unique<wnd1>(*this);
    }

    FLYTError wnd1::save_changes(std::ostream& out) {
        std::streamoff sectionStart = out.tellp();
        
        LOG_AND_RETURN_IF_ERR(PaneBase::save_changes(out));

        frameNum = frames.size();

        Utility::Endian::toPlatform_inplace(eType::Big, leftStretch);
        Utility::Endian::toPlatform_inplace(eType::Big, rightStretch);
        Utility::Endian::toPlatform_inplace(eType::Big, topStretch);
        Utility::Endian::toPlatform_inplace(eType::Big, bottomStretch);
        Utility::Endian::toPlatform_inplace(eType::Big, frameSizeLeft);
        Utility::Endian::toPlatform_inplace(eType::Big, frameSizeRight);
        Utility::Endian::toPlatform_inplace(eType::Big, frameSizeTop);
        Utility::Endian::toPlatform_inplace(eType::Big, frameSizeBottom);
        //update content and frame table offsets later

        out.write(reinterpret_cast<const char*>(&leftStretch), sizeof(leftStretch));
        out.write(reinterpret_cast<const char*>(&rightStretch), sizeof(rightStretch));
        out.write(reinterpret_cast<const char*>(&topStretch), sizeof(topStretch));
        out.write(reinterpret_cast<const char*>(&bottomStretch), sizeof(bottomStretch));
        out.write(reinterpret_cast<const char*>(&frameSizeLeft), sizeof(frameSizeLeft));
        out.write(reinterpret_cast<const char*>(&frameSizeRight), sizeof(frameSizeRight));
        out.write(reinterpret_cast<const char*>(&frameSizeTop), sizeof(frameSizeTop));
        out.write(reinterpret_cast<const char*>(&frameSizeBottom), sizeof(frameSizeBottom));
        out.write(reinterpret_cast<const char*>(&frameNum), sizeof(frameNum));
        out.write(reinterpret_cast<const char*>(&wndFlags), sizeof(wndFlags));
        out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));
        Utility::seek(out, 8, std::ios::cur); //skip content and frame table offsets

        contentOffset = out.tellp() - sectionStart;
        writeRGBA(out, content.vertexColorTL);
        writeRGBA(out, content.vertexColorTR);
        writeRGBA(out, content.vertexColorBL);
        writeRGBA(out, content.vertexColorBR);

        content.numCoords = content.coords.size();
        Utility::Endian::toPlatform_inplace(eType::Big, content.matIndex);

        out.write(reinterpret_cast<const char*>(&content.matIndex), sizeof(content.matIndex));
        out.write(reinterpret_cast<const char*>(&content.numCoords), sizeof(content.numCoords));
        out.write(reinterpret_cast<const char*>(&content.padding_0x00), sizeof(content.padding_0x00));

        for (UVCoords& coord : content.coords) {
            writeVec2(out, coord.coordTL);
            writeVec2(out, coord.coordTR);
            writeVec2(out, coord.coordBL);
            writeVec2(out, coord.coordBR);
        }


        frameTableOffset = out.tellp() - sectionStart;
        frameTable.clear();
        frameTable.reserve(frameNum);

        {
            uint32_t frameOffset = frameTableOffset + (frameNum * 0x4);
            Utility::seek(out, sectionStart + frameOffset, std::ios::beg);
            for (windowFrame& frame : frames) {
                frameTable.push_back(frameOffset);
                Utility::Endian::toPlatform_inplace(eType::Big, frame.matIndex);

                out.write(reinterpret_cast<const char*>(&frame.matIndex), sizeof(frame.matIndex));
                out.write(reinterpret_cast<const char*>(&frame.texFlip), sizeof(frame.texFlip));
                out.write(reinterpret_cast<const char*>(&frame.padding_0x00), sizeof(frame.padding_0x00));

                frameOffset = out.tellp() - sectionStart;
            }
            padToLen(out, 4);
            sectionSize = out.tellp() - sectionStart; //section ends after frames
        }

        out.seekp(sectionStart + 0x4, std::ios::beg);
        uint32_t sectionSize_BE = Utility::Endian::toPlatform(eType::Big, sectionSize);
        out.write(reinterpret_cast<const char*>(&sectionSize_BE), sizeof(sectionSize_BE)); //update value

        out.seekp(sectionStart + 0x68, std::ios::beg);

        Utility::Endian::toPlatform_inplace(eType::Big, contentOffset);
        uint32_t frameTableOffset_BE = Utility::Endian::toPlatform(eType::Big, frameTableOffset);

        out.write(reinterpret_cast<const char*>(&contentOffset), sizeof(contentOffset));
        out.write(reinterpret_cast<const char*>(&frameTableOffset_BE), sizeof(frameTableOffset_BE));

        out.seekp(sectionStart + frameTableOffset, std::ios::beg);

        for (uint32_t& frameOffset : frameTable) {
            Utility::Endian::toPlatform_inplace(eType::Big, frameOffset);
            out.write(reinterpret_cast<const char*>(&frameOffset), sizeof(frameOffset));
        }

        out.seekp(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }


    FLYTError txt1::read(std::istream& bflyt) {
        std::streamoff sectionStart = bflyt.tellg();
        
        LOG_AND_RETURN_IF_ERR(PaneBase::read(bflyt));
        if (std::strncmp(magic, "txt1", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
        }

        if (!bflyt.read(reinterpret_cast<char*>(&texLen), sizeof(texLen))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&restrictedLen), sizeof(restrictedLen))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&matIndex), sizeof(matIndex))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&fontIndex), sizeof(fontIndex))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&textAlignment), sizeof(textAlignment))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&lineAlignment), sizeof(lineAlignment))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&txtFlags), sizeof(txtFlags))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&italicTilt), sizeof(italicTilt))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&textOffset), sizeof(textOffset))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        if (!readRGBA(bflyt, bflyt.tellg(), fontColorTop)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!readRGBA(bflyt, bflyt.tellg(), fontColorBottom)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!bflyt.read(reinterpret_cast<char*>(&fontSizeX), sizeof(fontSizeX))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&fontSizeY), sizeof(fontSizeY))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&charSpace), sizeof(charSpace))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&lineSpace), sizeof(lineSpace))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&nameOffset), sizeof(nameOffset))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&shadowPosX), sizeof(shadowPosX))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&shadowPosY), sizeof(shadowPosY))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&shadowSizeX), sizeof(shadowSizeX))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&shadowSizeY), sizeof(shadowSizeY))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        if (!readRGBA(bflyt, bflyt.tellg(), shadowColorTop)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!readRGBA(bflyt, bflyt.tellg(), shadowColorBottom)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!bflyt.read(reinterpret_cast<char*>(&shadowItalicTilt), sizeof(shadowItalicTilt))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, texLen);
        Utility::Endian::toPlatform_inplace(eType::Big, restrictedLen);
        Utility::Endian::toPlatform_inplace(eType::Big, matIndex);
        Utility::Endian::toPlatform_inplace(eType::Big, fontIndex);
        Utility::Endian::toPlatform_inplace(eType::Big, italicTilt);
        Utility::Endian::toPlatform_inplace(eType::Big, textOffset);
        Utility::Endian::toPlatform_inplace(eType::Big, fontSizeX);
        Utility::Endian::toPlatform_inplace(eType::Big, fontSizeY);
        Utility::Endian::toPlatform_inplace(eType::Big, charSpace);
        Utility::Endian::toPlatform_inplace(eType::Big, lineSpace);
        Utility::Endian::toPlatform_inplace(eType::Big, nameOffset);
        Utility::Endian::toPlatform_inplace(eType::Big, shadowPosX);
        Utility::Endian::toPlatform_inplace(eType::Big, shadowPosY);
        Utility::Endian::toPlatform_inplace(eType::Big, shadowSizeX);
        Utility::Endian::toPlatform_inplace(eType::Big, shadowSizeY);
        Utility::Endian::toPlatform_inplace(eType::Big, shadowItalicTilt);

        if (textOffset != 0) {
            if (std::u16string name = Utility::Str::readNullTerminatedStr<std::u16string>(bflyt, sectionStart + textOffset); name.empty()) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //empty string means it could not read a character from file
            }
            else {
                text = name;
                Utility::Endian::toPlatform_inplace(eType::Big, text);
            }
        }
        else {
            text.clear();
        }

        if (nameOffset != 0) {
            if (std::string name = Utility::Str::readNullTerminatedStr<std::string>(bflyt, sectionStart + nameOffset); name.empty()) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //empty string means it could not read a character from file
            }
            else {
                textBoxName = name;
            }
        }
        else {
            textBoxName.clear();
        }

        if ((txtFlags >> 4) & 0x01) { //per char transform flag
            uint32_t transformOffset = 0;
            bflyt.seekg(sectionStart + 0xA0, std::ios::beg);
            if (!bflyt.read(reinterpret_cast<char*>(&transformOffset), sizeof(transformOffset))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            Utility::Endian::toPlatform_inplace(eType::Big, transformOffset);
            charTransformOffset = transformOffset;

            bflyt.seekg(sectionStart + transformOffset);
            perCharTransform transform{};
            if (!bflyt.read(reinterpret_cast<char*>(&transform.curveTimeOffset), sizeof(transform.curveTimeOffset))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&transform.animCurveWidth), sizeof(transform.animCurveWidth))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&transform.loopType), sizeof(transform.loopType))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&transform.verticalOrigin), sizeof(transform.verticalOrigin))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&transform.hasAnimInfo), 1)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&transform.padding_0x00), sizeof(transform.padding_0x00))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            Utility::Endian::toPlatform_inplace(eType::Big, transform.curveTimeOffset);
            Utility::Endian::toPlatform_inplace(eType::Big, transform.animCurveWidth);

            charTransform = transform;
        }
        else {
            charTransformOffset = std::nullopt;
            charTransform = std::nullopt;
        }

        bflyt.seekg(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }

    std::unique_ptr<PaneBase> txt1::clonePane() {
        return std::make_unique<txt1>(*this);
    }

    FLYTError txt1::save_changes(std::ostream& out) {
        std::streamoff sectionStart = out.tellp();
        
        LOG_AND_RETURN_IF_ERR(PaneBase::save_changes(out));

        text = Utility::Str::assureNullTermination(text);
        texLen = text.size() * 2; //need len in bytes, size returns num chars
        //might want to set restricted len?

        Utility::Endian::toPlatform_inplace(eType::Big, texLen);
        Utility::Endian::toPlatform_inplace(eType::Big, restrictedLen);
        Utility::Endian::toPlatform_inplace(eType::Big, matIndex);
        Utility::Endian::toPlatform_inplace(eType::Big, fontIndex);
        Utility::Endian::toPlatform_inplace(eType::Big, italicTilt);
        Utility::Endian::toPlatform_inplace(eType::Big, fontSizeX);
        Utility::Endian::toPlatform_inplace(eType::Big, fontSizeY);
        Utility::Endian::toPlatform_inplace(eType::Big, charSpace);
        Utility::Endian::toPlatform_inplace(eType::Big, lineSpace);
        Utility::Endian::toPlatform_inplace(eType::Big, shadowPosX);
        Utility::Endian::toPlatform_inplace(eType::Big, shadowPosY);
        Utility::Endian::toPlatform_inplace(eType::Big, shadowSizeX);
        Utility::Endian::toPlatform_inplace(eType::Big, shadowSizeY);
        Utility::Endian::toPlatform_inplace(eType::Big, shadowItalicTilt);

        out.write(reinterpret_cast<const char*>(&texLen), sizeof(texLen));
        out.write(reinterpret_cast<const char*>(&restrictedLen), sizeof(restrictedLen));
        out.write(reinterpret_cast<const char*>(&matIndex), sizeof(matIndex));
        out.write(reinterpret_cast<const char*>(&fontIndex), sizeof(fontIndex));
        out.write(reinterpret_cast<const char*>(&textAlignment), sizeof(textAlignment));
        out.write(reinterpret_cast<const char*>(&lineAlignment), 1);
        out.write(reinterpret_cast<const char*>(&txtFlags), sizeof(txtFlags));
        out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));
        out.write(reinterpret_cast<const char*>(&italicTilt), sizeof(italicTilt));
        Utility::seek(out, 4, std::ios::cur); //skip text offset

        writeRGBA(out, fontColorTop);
        writeRGBA(out, fontColorBottom);

        out.write(reinterpret_cast<const char*>(&fontSizeX), sizeof(fontSizeX));
        out.write(reinterpret_cast<const char*>(&fontSizeY), sizeof(fontSizeY));
        out.write(reinterpret_cast<const char*>(&charSpace), sizeof(charSpace));
        out.write(reinterpret_cast<const char*>(&lineSpace), sizeof(lineSpace));
        Utility::seek(out, 4, std::ios::cur); //skip name offset
        out.write(reinterpret_cast<const char*>(&shadowPosX), sizeof(shadowPosX));
        out.write(reinterpret_cast<const char*>(&shadowPosY), sizeof(shadowPosY));
        out.write(reinterpret_cast<const char*>(&shadowSizeX), sizeof(shadowSizeX));
        out.write(reinterpret_cast<const char*>(&shadowSizeY), sizeof(shadowSizeY));

        writeRGBA(out, shadowColorTop);
        writeRGBA(out, shadowColorBottom);

        out.write(reinterpret_cast<const char*>(&shadowItalicTilt), sizeof(shadowItalicTilt));
        if (charTransform.has_value()) {
            Utility::seek(out, 4, std::ios::cur); //skip char transform offset
        }

        textOffset = 0;
        if (!text.empty()) {
            textOffset = out.tellp() - sectionStart;
            Utility::Endian::toPlatform_inplace(eType::Big, text);
            out.write(reinterpret_cast<const char*>(&text[0]), text.size() * 2);
            padToLen(out, 4);
        }

        nameOffset = 0;
        if (!textBoxName.empty()) {
            nameOffset = out.tellp() - sectionStart;
            out.write(&textBoxName[0], textBoxName.size());
            padToLen(out, 4);
        }

        charTransformOffset = std::nullopt;
        if (charTransform.has_value()) {
            charTransformOffset = out.tellp() - sectionStart;
            perCharTransform& transform = charTransform.value();

            Utility::Endian::toPlatform_inplace(eType::Big, transform.curveTimeOffset);
            Utility::Endian::toPlatform_inplace(eType::Big, transform.animCurveWidth);

            out.write(reinterpret_cast<const char*>(&transform.curveTimeOffset), sizeof(transform.curveTimeOffset));
            out.write(reinterpret_cast<const char*>(&transform.animCurveWidth), sizeof(transform.animCurveWidth));
            out.write(reinterpret_cast<const char*>(&transform.loopType), sizeof(transform.loopType));
            out.write(reinterpret_cast<const char*>(&transform.verticalOrigin), sizeof(transform.verticalOrigin));
            out.write(reinterpret_cast<const char*>(&transform.hasAnimInfo), 1);
            out.write(reinterpret_cast<const char*>(&transform.padding_0x00), sizeof(transform.padding_0x00));
        }

        sectionSize = out.tellp() - sectionStart; //section ends after text/transform

        out.seekp(sectionStart + 0x4, std::ios::beg);
        uint32_t sectionSize_BE = Utility::Endian::toPlatform(eType::Big, sectionSize);
        out.write(reinterpret_cast<const char*>(&sectionSize_BE), sizeof(sectionSize_BE)); //update value

        if (textOffset != 0) {
            out.seekp(sectionStart + 0x64, std::ios::beg);
            Utility::Endian::toPlatform_inplace(eType::Big, textOffset);
            out.write(reinterpret_cast<const char*>(&textOffset), sizeof(textOffset)); //update value
        }

        if (nameOffset != 0) {
            out.seekp(sectionStart + 0x80, std::ios::beg);
            Utility::Endian::toPlatform_inplace(eType::Big, nameOffset);
            out.write(reinterpret_cast<const char*>(&nameOffset), sizeof(nameOffset)); //update value
        }

        if (charTransformOffset.has_value()) {
            uint32_t& transformOffset = charTransformOffset.value();
            out.seekp(sectionStart + 0xA0, std::ios::beg);
            Utility::Endian::toPlatform_inplace(eType::Big, transformOffset);
            out.write(reinterpret_cast<const char*>(&transformOffset), sizeof(transformOffset)); //update value
        }

        out.seekp(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }

    FLYTError pic1::read(std::istream& bflyt) {
        std::streamoff sectionStart = bflyt.tellg();
        
        LOG_AND_RETURN_IF_ERR(PaneBase::read(bflyt));
        if (std::strncmp(magic, "pic1", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
        }

        if (!readRGBA(bflyt, bflyt.tellg(), vertexColorTL)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!readRGBA(bflyt, bflyt.tellg(), vertexColorTR)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!readRGBA(bflyt, bflyt.tellg(), vertexColorBL)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!readRGBA(bflyt, bflyt.tellg(), vertexColorBR)) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //returns false if read fails
        }

        if (!bflyt.read(reinterpret_cast<char*>(&matIndex), sizeof(matIndex))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&numCoords), sizeof(numCoords))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, matIndex);

        coords.reserve(numCoords);
        for (uint8_t i = 0; i < numCoords; i++) {
            UVCoords& coord = coords.emplace_back();
            if (!readVec2(bflyt, bflyt.tellg(), coord.coordTL)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            if (!readVec2(bflyt, bflyt.tellg(), coord.coordTR)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            if (!readVec2(bflyt, bflyt.tellg(), coord.coordBL)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            if (!readVec2(bflyt, bflyt.tellg(), coord.coordBR)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
        }

        bflyt.seekg(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }

    std::unique_ptr<PaneBase> pic1::clonePane() {
        return std::make_unique<pic1>(*this);
    }

    FLYTError pic1::save_changes(std::ostream& out) {
        std::streamoff sectionStart = out.tellp();
        
        LOG_AND_RETURN_IF_ERR(PaneBase::save_changes(out));

        writeRGBA(out, vertexColorTL);
        writeRGBA(out, vertexColorTR);
        writeRGBA(out, vertexColorBL);
        writeRGBA(out, vertexColorBR);

        numCoords = coords.size();
        Utility::Endian::toPlatform_inplace(eType::Big, matIndex);

        out.write(reinterpret_cast<const char*>(&matIndex), sizeof(matIndex));
        out.write(reinterpret_cast<const char*>(&numCoords), sizeof(numCoords));
        out.write(reinterpret_cast<const char*>(&padding_0x00), sizeof(padding_0x00));

        for (UVCoords& coord : coords) {
            writeVec2(out, coord.coordTL);
            writeVec2(out, coord.coordTR);
            writeVec2(out, coord.coordBL);
            writeVec2(out, coord.coordBR);
        }

        //doesnt need padding, length will always be a multiple of 4
        sectionSize = out.tellp() - sectionStart; //section ends after coords

        uint32_t sectionSize_BE = Utility::Endian::toPlatform(eType::Big, sectionSize);
        out.seekp(sectionStart + 0x4, std::ios::beg);
        out.write(reinterpret_cast<const char*>(&sectionSize_BE), sizeof(sectionSize_BE)); //update value

        out.seekp(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }


    partProperty::partProperty(const partProperty& property) : 
        propName(property.propName),
        usageFlag(property.usageFlag),
        basicUsageFlag(property.basicUsageFlag),
        matUsageFlag(property.matUsageFlag),
        padding_0x00(property.padding_0x00),
        propOffset(property.propOffset),
        userDataOffset(property.userDataOffset),
        panelInfoOffset(property.panelInfoOffset),
        userData(property.userData),
        paneInfo(property.paneInfo)
    {
        if(prop.has_value()) {
            prop = property.prop.value()->clonePane();
        }
        else {
            prop = std::nullopt;
        }
    }


    FLYTError prt1::read(std::istream& bflyt) {
        std::streamoff sectionStart = bflyt.tellg();
        
        LOG_AND_RETURN_IF_ERR(PaneBase::read(bflyt));
        if (std::strncmp(magic, "prt1", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
        }

        if (!bflyt.read(reinterpret_cast<char*>(&propCount), sizeof(propCount))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&magnifyX), sizeof(magnifyX))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&magnifyY), sizeof(magnifyY))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, propCount);
        Utility::Endian::toPlatform_inplace(eType::Big, magnifyX);
        Utility::Endian::toPlatform_inplace(eType::Big, magnifyY);

        properties.reserve(propCount);
        for (unsigned int i = 0; i < propCount; i++) {
            partProperty& prop = properties.emplace_back();
            if (!bflyt.read(reinterpret_cast<char*>(&prop.propName), 0x18)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&prop.usageFlag), 1)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&prop.basicUsageFlag), 1)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&prop.matUsageFlag), 1)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&prop.padding_0x00), 1)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&prop.propOffset), sizeof(prop.propOffset))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&prop.userDataOffset), sizeof(prop.userDataOffset))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            if (!bflyt.read(reinterpret_cast<char*>(&prop.panelInfoOffset), sizeof(prop.panelInfoOffset))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }

            Utility::Endian::toPlatform_inplace(eType::Big, prop.propOffset);
            Utility::Endian::toPlatform_inplace(eType::Big, prop.userDataOffset);
            Utility::Endian::toPlatform_inplace(eType::Big, prop.panelInfoOffset);

            if (prop.propOffset != 0) {
                bflyt.seekg(sectionStart + prop.propOffset, std::ios::beg);

                char propMagic[4];
                if (!bflyt.read(reinterpret_cast<char*>(&propMagic), sizeof(propMagic))) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
                bflyt.seekg(-4, std::ios::cur); //seek back to start of the section

                if (std::strncmp(propMagic, "pic1", 4) == 0) {
                    prop.prop = std::make_unique<pic1>();
                }
                else if (std::strncmp(propMagic, "txt1", 4) == 0) {
                    prop.prop = std::make_unique<txt1>();
                }
                else if (std::strncmp(propMagic, "wnd1", 4) == 0) {
                    prop.prop = std::make_unique<wnd1>();
                }
                else if (std::strncmp(propMagic, "bnd1", 4) == 0) {
                    prop.prop = std::make_unique<bnd1>();
                }
                else if (std::strncmp(propMagic, "prt1", 4) == 0) {
                    prop.prop = std::make_unique<prt1>();
                }
                else {
                    LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
                }
                LOG_AND_RETURN_IF_ERR(prop.prop.value()->read(bflyt));
            }
            else {
                prop.prop = std::nullopt;
            }

            if (prop.userDataOffset != 0) {
                LOG_AND_RETURN_IF_ERR(prop.userData.emplace().read(bflyt));
            }
            else {
                prop.prop = std::nullopt;
            }

            if (prop.panelInfoOffset != 0) {
                std::string& info = prop.paneInfo.emplace(0x34, '\x00');
                if (!bflyt.read(reinterpret_cast<char*>(&info[0]), 0x34)) {
                    LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
                }
            }
            else {
                prop.paneInfo = std::nullopt;
            }
        }

        if (std::string name = Utility::Str::readNullTerminatedStr<std::string>(bflyt, bflyt.tellg()); name.empty()) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF) //empty string means it could not read a character from file
        }
        else {
            lytFilename = name;
        }

        bflyt.seekg(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }

    std::unique_ptr<PaneBase> prt1::clonePane() {
        return std::make_unique<prt1>(*this);
    }

    FLYTError prt1::save_changes(std::ostream& out) {
        std::streamoff sectionStart = out.tellp();
        
        LOG_AND_RETURN_IF_ERR(PaneBase::save_changes(out));

        propCount = properties.size();

        Utility::Endian::toPlatform_inplace(eType::Big, propCount);
        Utility::Endian::toPlatform_inplace(eType::Big, magnifyX);
        Utility::Endian::toPlatform_inplace(eType::Big, magnifyY);

        out.write(reinterpret_cast<const char*>(&propCount), sizeof(propCount));
        out.write(reinterpret_cast<const char*>(&magnifyX), sizeof(magnifyX));
        out.write(reinterpret_cast<const char*>(&magnifyY), sizeof(magnifyY));

        for (partProperty& prop : properties) {
            std::streamoff propOffset = out.tellp();
            out.write(reinterpret_cast<const char*>(&prop.propName), 0x18);
            out.write(reinterpret_cast<const char*>(&prop.usageFlag), 1);
            out.write(reinterpret_cast<const char*>(&prop.basicUsageFlag), 1);
            out.write(reinterpret_cast<const char*>(&prop.matUsageFlag), 1);
            out.write(reinterpret_cast<const char*>(&prop.padding_0x00), 1);
            Utility::seek(out, 0xC, std::ios::cur); //skip over offsets for now

            prop.propOffset = 0;
            if (prop.prop.has_value()) {
                prop.propOffset = out.tellp() - sectionStart;
                LOG_AND_RETURN_IF_ERR(prop.prop.value()->save_changes(out));
            }

            prop.userDataOffset = 0;
            if (prop.userData.has_value()) {
                prop.userDataOffset = out.tellp() - sectionStart;
                LOG_AND_RETURN_IF_ERR(prop.userData.value().save_changes(out));
            }

            prop.panelInfoOffset = 0;
            if (prop.paneInfo.has_value()) {
                prop.panelInfoOffset = out.tellp() - sectionStart;
                out.write(&prop.paneInfo.value()[0], 0x34);
            }

            std::streamoff propEnd = out.tellp();
            out.seekp(propOffset + 0x1C, std::ios::beg);

            Utility::Endian::toPlatform_inplace(eType::Big, prop.propOffset);
            Utility::Endian::toPlatform_inplace(eType::Big, prop.userDataOffset);
            Utility::Endian::toPlatform_inplace(eType::Big, prop.panelInfoOffset);

            out.write(reinterpret_cast<const char*>(&prop.propOffset), sizeof(prop.propOffset));
            out.write(reinterpret_cast<const char*>(&prop.userDataOffset), sizeof(prop.userDataOffset));
            out.write(reinterpret_cast<const char*>(&prop.panelInfoOffset), sizeof(prop.panelInfoOffset));

            out.seekp(propEnd, std::ios::beg);
        }

        out.write(&lytFilename[0], lytFilename.size());
        padToLen(out, 4);
        sectionSize = out.tellp() - sectionStart;

        uint32_t sectionSize_BE = Utility::Endian::toPlatform(eType::Big, sectionSize);
        out.seekp(sectionStart + 0x4, std::ios::beg);
        out.write(reinterpret_cast<const char*>(&sectionSize_BE), sizeof(sectionSize_BE)); //update value

        out.seekp(sectionStart + sectionSize, std::ios::beg);
        return FLYTError::NONE;
    }
}

FLYTError Pane::read(std::istream& bflyt) {
    userData = std::nullopt;

    if (!bflyt.read(magic, 4)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
    
    if (std::strncmp(magic, "pan1", 4) == 0) {
        pane = std::make_unique<NintendoWare::Layout::pan1>();
    }
    else if (std::strncmp(magic, "bnd1", 4) == 0) {
        pane = std::make_unique<NintendoWare::Layout::bnd1>();
    }
    else if (std::strncmp(magic, "wnd1", 4) == 0) {
        pane = std::make_unique<NintendoWare::Layout::wnd1>();
    }
    else if (std::strncmp(magic, "txt1", 4) == 0) {
        pane = std::make_unique<NintendoWare::Layout::txt1>();
    }
    else if (std::strncmp(magic, "pic1", 4) == 0) {
        pane = std::make_unique<NintendoWare::Layout::pic1>();
    }
    else if (std::strncmp(magic, "prt1", 4) == 0) {
        pane = std::make_unique<NintendoWare::Layout::prt1>();
    }
    else {
        LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_SECTION)
    }
    bflyt.seekg(-4, std::ios::cur);
    LOG_AND_RETURN_IF_ERR(pane->read(bflyt));

    char nextSection[4];
    if (!bflyt.read(reinterpret_cast<char*>(&nextSection), sizeof(nextSection))) {
        LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
    }
    if (std::strncmp(nextSection, "pas1", 4) == 0) {
        uint32_t pas1Size;
        if (!bflyt.read(reinterpret_cast<char*>(&pas1Size), sizeof(pas1Size))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        Utility::Endian::toPlatform_inplace(eType::Big, pas1Size);
        if (pas1Size != 0x8) {
            LOG_ERR_AND_RETURN(FLYTError::UNEXPECTED_VALUE)
        }

        if (!bflyt.read(reinterpret_cast<char*>(&nextSection), sizeof(nextSection))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        while (std::strncmp(nextSection, "pae1", 4) != 0) {
            bflyt.seekg(-4, std::ios::cur);

            LOG_AND_RETURN_IF_ERR(children.emplace_back().read(bflyt));
            
            if (!bflyt.read(reinterpret_cast<char*>(&nextSection), sizeof(nextSection))) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
        }

        uint32_t pae1Size;
        if (!bflyt.read(reinterpret_cast<char*>(&pae1Size), sizeof(pae1Size))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        Utility::Endian::toPlatform_inplace(eType::Big, pae1Size);
        if (pae1Size != 0x8) {
            LOG_ERR_AND_RETURN(FLYTError::UNEXPECTED_VALUE)
        }
    }
    else if(std::strncmp(nextSection, "usd1", 4) == 0) {
        bflyt.seekg(-4, std::ios::cur);

        NintendoWare::Layout::usd1& data = userData.emplace();
        LOG_AND_RETURN_IF_ERR(data.read(bflyt));
    }
    else {
        bflyt.seekg(-4, std::ios::cur);
    }

    return FLYTError::NONE;
}

Pane& Pane::duplicateChildPane(const unsigned int originalIndex) {
    Pane& pane = children.emplace_back();
    memcpy(&pane.magic, &children[originalIndex].magic, 4);
    pane.userData = children[originalIndex].userData;
    pane.pane = children[originalIndex].pane->clonePane();
    return pane; //leaves offset unintialized
}

FLYTError Pane::save_changes(std::ostream& out, uint16_t& sectionNum) {
    sectionNum += 1; //add each pane to section num    

    LOG_AND_RETURN_IF_ERR(pane->save_changes(out));

    if (userData.has_value()) {
        sectionNum += 1;
        LOG_AND_RETURN_IF_ERR(userData.value().save_changes(out));
    }

    if (!children.empty()) {
        uint32_t size = 0x8;
        Utility::Endian::toPlatform_inplace(eType::Big, size);

        out.write("pas1", 4);
        out.write(reinterpret_cast<const char*>(&size), sizeof(size));
        sectionNum += 1; //pas counts as section

        for (Pane& child : children) {
            LOG_AND_RETURN_IF_ERR(child.save_changes(out, sectionNum));
        }

        out.write("pae1", 4);
        out.write(reinterpret_cast<const char*>(&size), sizeof(size));
        sectionNum += 1; //pae counts as section
    }

    return FLYTError::NONE;
}


namespace FileTypes {

    const char* FLYTErrorGetName(FLYTError err) {
        switch (err) {

            case FLYTError::NONE:
                return "NONE";
            case FLYTError::COULD_NOT_OPEN:
                return "COULD_NOT_OPEN";
            case FLYTError::NOT_FLYT:
                return "NOT_FLYT";
            case FLYTError::UNKNOWN_VERSION:
                return "UNKNOWN_VERSION";
            case FLYTError::UNEXPECTED_VALUE:
                return "UNEXPECTED_VALUE";
            case FLYTError::UNKNOWN_SECTION:
                return "UNKNOWN_SECTION";
            case FLYTError::REACHED_EOF:
                return "REACHED_EOF";
            default:
                return "UNKNOWN";
        }
    }

    void FLYTFile::initNew() {
        memcpy(header.magicFLYT, "FLYT", 4);
        header.byteOrderMarker = 0xFEFF;
        header.headerSize_0x14 = 0x14;
        header.version_0x0202 = 0x02020000;
        header.fileSize = 0;
        header.numSections = 0;

        LYT1.drawCentered = true;
        LYT1.width = 1280;
        LYT1.height = 720;
        LYT1.maxPartWidth = 0; //unknown purpose
        LYT1.maxPartHeight = 0; //unknown purpose
        LYT1.layoutName = "";

        textures = std::nullopt;
        fonts = std::nullopt;
        materials = std::nullopt;
        container = std::nullopt;
        userData = std::nullopt;
   }

    FLYTFile FLYTFile::createNew() {
        FLYTFile newFLYT{};
        newFLYT.initNew();
        return newFLYT;
    }

    FLYTError FLYTFile::loadFromBinary(std::istream& bflyt) {
        if (!bflyt.read(header.magicFLYT, 4)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        if (std::strncmp(header.magicFLYT, "FLYT", 4) != 0) {
            LOG_ERR_AND_RETURN(FLYTError::NOT_FLYT)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&header.byteOrderMarker), sizeof(header.byteOrderMarker))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&header.headerSize_0x14), sizeof(header.headerSize_0x14))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&header.version_0x0202), sizeof(header.version_0x0202))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&header.fileSize), sizeof(header.fileSize))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&header.numSections), sizeof(header.numSections))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }
        if (!bflyt.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00))) {
            LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, header.byteOrderMarker);
        Utility::Endian::toPlatform_inplace(eType::Big, header.headerSize_0x14);
        Utility::Endian::toPlatform_inplace(eType::Big, header.version_0x0202);
        Utility::Endian::toPlatform_inplace(eType::Big, header.fileSize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.numSections);

        if (header.byteOrderMarker != 0xFEFF) LOG_ERR_AND_RETURN(FLYTError::UNEXPECTED_VALUE)
        if (header.headerSize_0x14 != 0x14) LOG_ERR_AND_RETURN(FLYTError::UNEXPECTED_VALUE)
        if (header.version_0x0202 != 0x02020000) LOG_ERR_AND_RETURN(FLYTError::UNKNOWN_VERSION)
        if (header.padding_0x00[0] != 0x00 || header.padding_0x00[1] != 0x00) LOG_ERR_AND_RETURN(FLYTError::UNEXPECTED_VALUE)

        LOG_AND_RETURN_IF_ERR(LYT1.read(bflyt)); //should always come immediately after the header

        std::string magic(4, '\0');
        if (!bflyt.read(reinterpret_cast<char*>(&magic[0]), 4)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
        bflyt.seekg(-4, std::ios::cur); //seek back to start of the section

        if (magic == "txl1") {
            LOG_AND_RETURN_IF_ERR(textures.emplace().read(bflyt));

            if (!bflyt.read(reinterpret_cast<char*>(&magic[0]), 4)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            bflyt.seekg(-4, std::ios::cur); //seek back to start of the section
        }

        if (magic == "fnl1") {
            LOG_AND_RETURN_IF_ERR(fonts.emplace().read(bflyt));

            if (!bflyt.read(reinterpret_cast<char*>(&magic[0]), 4)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            bflyt.seekg(-4, std::ios::cur); //seek back to start of the section
        }

        if (magic == "mat1") {
            LOG_AND_RETURN_IF_ERR(materials.emplace().read(bflyt));

            if (!bflyt.read(reinterpret_cast<char*>(&magic[0]), 4)) LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            bflyt.seekg(-4, std::ios::cur); //seek back to start of the section
        }

        while (sections.contains(magic)) {
            if (magic == "usd1") {
                LOG_AND_RETURN_IF_ERR(userData.emplace().read(bflyt));
            }
            else if (magic == "cnt1") {
                LOG_AND_RETURN_IF_ERR(container.emplace().read(bflyt));
            }
            else if (magic == "grp1") {
                LOG_AND_RETURN_IF_ERR(rootGroup.read(bflyt));
            }
            else {
                LOG_AND_RETURN_IF_ERR(rootPane.read(bflyt));
            }

            if (bflyt.tellg() >= header.fileSize) break; //reached end of last section
            if (!bflyt.read(reinterpret_cast<char*>(&magic[0]), 4)) {
                LOG_ERR_AND_RETURN(FLYTError::REACHED_EOF)
            }
            bflyt.seekg(-4, std::ios::cur); //seek back to start of the section
        }

        return FLYTError::NONE;
    }

    FLYTError FLYTFile::loadFromFile(const fspath& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERR_AND_RETURN(FLYTError::COULD_NOT_OPEN)
        }
        return loadFromBinary(file);
    }

    FLYTError FLYTFile::writeToStream(std::ostream& out) {
        out.write(header.magicFLYT, 4);

        Utility::Endian::toPlatform_inplace(eType::Big, header.byteOrderMarker);
        Utility::Endian::toPlatform_inplace(eType::Big, header.headerSize_0x14);
        Utility::Endian::toPlatform_inplace(eType::Big, header.version_0x0202);

        out.write(reinterpret_cast<const char*>(&header.byteOrderMarker), sizeof(header.byteOrderMarker));
        out.write(reinterpret_cast<const char*>(&header.headerSize_0x14), sizeof(header.headerSize_0x14));
        out.write(reinterpret_cast<const char*>(&header.version_0x0202), sizeof(header.version_0x0202));
        Utility::seek(out, 6, std::ios::cur); //skip filesize and section count for now
        out.write(reinterpret_cast<const char*>(&header.padding_0x00), sizeof(header.padding_0x00));

        header.numSections = 1;

        LOG_AND_RETURN_IF_ERR(LYT1.save_changes(out));
        if (textures.has_value()) {
            LOG_AND_RETURN_IF_ERR(textures.value().save_changes(out));
            header.numSections += 1;
        }
        if (fonts.has_value()) {
            LOG_AND_RETURN_IF_ERR(fonts.value().save_changes(out));
            header.numSections += 1;
        }
        if (materials.has_value()) {
            LOG_AND_RETURN_IF_ERR(materials.value().save_changes(out));
            header.numSections += 1;
        }

        LOG_AND_RETURN_IF_ERR(rootPane.save_changes(out, header.numSections));
        LOG_AND_RETURN_IF_ERR(rootGroup.save_changes(out, header.numSections));

        if (container.has_value()) {
            LOG_AND_RETURN_IF_ERR(container.value().save_changes(out));
            header.numSections += 1;
        }
        if (userData.has_value()) {
            LOG_AND_RETURN_IF_ERR(userData.value().save_changes(out));
            header.numSections += 1;
        }

        header.fileSize = out.tellp();

        Utility::Endian::toPlatform_inplace(eType::Big, header.fileSize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.numSections);

        out.seekp(0xC, std::ios::beg);
        out.write(reinterpret_cast<const char*>(&header.fileSize), sizeof(header.fileSize));
        out.write(reinterpret_cast<const char*>(&header.numSections), sizeof(header.numSections));

        return FLYTError::NONE;
    }

    FLYTError FLYTFile::writeToFile(const fspath& outFilePath) {
        std::ofstream outFile(outFilePath, std::ios::binary);
        if (!outFile.is_open()) {
            LOG_ERR_AND_RETURN(FLYTError::COULD_NOT_OPEN)
        }
        return writeToStream(outFile);
    }
}
