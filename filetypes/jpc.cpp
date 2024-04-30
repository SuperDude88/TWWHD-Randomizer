#include "jpc.hpp"

#include <cstring>
#include <algorithm>

#include <utility/endian.hpp>
#include <utility/file.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;

std::vector<JParticle::ColorAnimationKeyframe> readColorTable(std::istream& in, const std::streamoff offset, const uint8_t dataCount) {
    in.seekg(offset, std::ios::beg);

    std::vector<JParticle::ColorAnimationKeyframe> table;
    table.reserve(dataCount);
    for (uint8_t i = 0; i < dataCount; i++) {
        JParticle::ColorAnimationKeyframe& keyframe = table.emplace_back();

        if (!in.read(reinterpret_cast<char*>(&keyframe.time), sizeof(keyframe.time))) {
            table.clear(); //return empty vector to signal error
            return table;
        }
        Utility::Endian::toPlatform_inplace(eType::Big, keyframe.time);

        if (!readRGBA(in, in.tellg(), keyframe.color)) {
            table.clear(); //return empty vector to signal error
            return table;
        }
    }

    return table;
}

void writeColorTable(std::ostream& out, const std::vector<JParticle::ColorAnimationKeyframe>& table) {
    for (const JParticle::ColorAnimationKeyframe& keyframe : table) {
        uint16_t time = Utility::Endian::toPlatform(eType::Big, keyframe.time);

        out.write(reinterpret_cast<const char*>(&time), sizeof(time));
        writeRGBA(out, keyframe.color);
    }
}



namespace JParticle {
    JPCError BEM1::read(std::istream& jpc) {
        if (!jpc.read(magic, 4)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        jpc.seekg(4, std::ios::cur);

        if (std::strncmp(magic, "BEM1", 4) != 0) LOG_ERR_AND_RETURN(JPCError::UNKNOWN_CHUNK);

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        if (!jpc.read(reinterpret_cast<char*>(&flags), sizeof(flags))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        Utility::Endian::toPlatform_inplace(eType::Big, flags);

        emitFlags = static_cast<EmitFlags>(flags & 0xFF);
        volumeType = static_cast<VolumeType>((flags >> 8) & 0x07);

        if (!jpc.read(reinterpret_cast<char*>(&volumeSweep), sizeof(volumeSweep))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&volumeMinRad), sizeof(volumeMinRad))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&volumeSize), sizeof(volumeSize))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&divNumber), sizeof(divNumber))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&rate), sizeof(rate))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&rateRndm), sizeof(rateRndm))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&rateStep), sizeof(rateStep))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        jpc.seekg(1, std::ios::cur);

        if (!jpc.read(reinterpret_cast<char*>(&maxFrame), sizeof(maxFrame))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&startFrame), sizeof(startFrame))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&lifeTime), sizeof(lifeTime))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&lifeTimeRndm), sizeof(lifeTimeRndm))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        if (!jpc.read(reinterpret_cast<char*>(&initialVelOmni), sizeof(initialVelOmni))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&initialVelAxis), sizeof(initialVelAxis))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&initialVelRndm), sizeof(initialVelRndm))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&initialVelDir), sizeof(initialVelDir))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&initialVelRatio), sizeof(initialVelRatio))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        if (!jpc.read(reinterpret_cast<char*>(&spread), sizeof(spread))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&airResist), sizeof(airResist))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&airResistRndm), sizeof(airResistRndm))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        if (!jpc.read(reinterpret_cast<char*>(&moment), sizeof(moment))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&momentRndm), sizeof(momentRndm))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&accel), sizeof(accel))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&accelRndm), sizeof(accelRndm))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        Utility::Endian::toPlatform_inplace(eType::Big, volumeSweep);
        Utility::Endian::toPlatform_inplace(eType::Big, volumeMinRad);
        Utility::Endian::toPlatform_inplace(eType::Big, volumeSize);
        Utility::Endian::toPlatform_inplace(eType::Big, divNumber);
        Utility::Endian::toPlatform_inplace(eType::Big, rate);
        Utility::Endian::toPlatform_inplace(eType::Big, rateRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, maxFrame);
        Utility::Endian::toPlatform_inplace(eType::Big, startFrame);
        Utility::Endian::toPlatform_inplace(eType::Big, lifeTime);
        Utility::Endian::toPlatform_inplace(eType::Big, lifeTime);
        Utility::Endian::toPlatform_inplace(eType::Big, initialVelOmni);
        Utility::Endian::toPlatform_inplace(eType::Big, initialVelAxis);
        Utility::Endian::toPlatform_inplace(eType::Big, initialVelRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, initialVelDir);
        Utility::Endian::toPlatform_inplace(eType::Big, initialVelRatio);
        Utility::Endian::toPlatform_inplace(eType::Big, spread);
        Utility::Endian::toPlatform_inplace(eType::Big, airResist);
        Utility::Endian::toPlatform_inplace(eType::Big, airResistRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, moment);
        Utility::Endian::toPlatform_inplace(eType::Big, momentRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, accel);
        Utility::Endian::toPlatform_inplace(eType::Big, accelRndm);

        if (!readVec3(jpc, jpc.tellg(), emitterScale)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!readVec3(jpc, jpc.tellg(), emitterTrans)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!readVec3(jpc, jpc.tellg(), emitterDir)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!readVec3(jpc, jpc.tellg(), emitterRot)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        return JPCError::NONE;
    }

    JPCError BEM1::save_changes(std::ostream& out) {
        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        out.write(magic, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldn't need to update?
        Utility::seek(out, 4, std::ios::cur);

        flags = (flags & 0xFFFFFF00) | (static_cast<uint8_t>(emitFlags) & 0x000000FF);
        flags = (flags & 0xFFFF00FF) | ((static_cast<uint8_t>(volumeType) & 0x000000FF) << 8);

        Utility::Endian::toPlatform_inplace(eType::Big, flags);
        out.write(reinterpret_cast<const char*>(&flags), sizeof(flags));

        Utility::Endian::toPlatform_inplace(eType::Big, volumeSweep);
        Utility::Endian::toPlatform_inplace(eType::Big, volumeMinRad);
        Utility::Endian::toPlatform_inplace(eType::Big, volumeSize);
        Utility::Endian::toPlatform_inplace(eType::Big, divNumber);
        Utility::Endian::toPlatform_inplace(eType::Big, rate);
        Utility::Endian::toPlatform_inplace(eType::Big, rateRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, maxFrame);
        Utility::Endian::toPlatform_inplace(eType::Big, startFrame);
        Utility::Endian::toPlatform_inplace(eType::Big, lifeTime);
        Utility::Endian::toPlatform_inplace(eType::Big, lifeTime);
        Utility::Endian::toPlatform_inplace(eType::Big, initialVelOmni);
        Utility::Endian::toPlatform_inplace(eType::Big, initialVelAxis);
        Utility::Endian::toPlatform_inplace(eType::Big, initialVelRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, initialVelDir);
        Utility::Endian::toPlatform_inplace(eType::Big, initialVelRatio);
        Utility::Endian::toPlatform_inplace(eType::Big, spread);
        Utility::Endian::toPlatform_inplace(eType::Big, airResist);
        Utility::Endian::toPlatform_inplace(eType::Big, airResistRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, moment);
        Utility::Endian::toPlatform_inplace(eType::Big, momentRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, accel);
        Utility::Endian::toPlatform_inplace(eType::Big, accelRndm);

        out.write(reinterpret_cast<const char*>(&volumeSweep), sizeof(volumeSweep));
        out.write(reinterpret_cast<const char*>(&volumeMinRad), sizeof(volumeMinRad));
        out.write(reinterpret_cast<const char*>(&volumeSize), sizeof(volumeSize));
        out.write(reinterpret_cast<const char*>(&divNumber), sizeof(divNumber));
        out.write(reinterpret_cast<const char*>(&rate), sizeof(rate));
        out.write(reinterpret_cast<const char*>(&rateRndm), sizeof(rateRndm));
        out.write(reinterpret_cast<const char*>(&rateStep), sizeof(rateStep));
        Utility::seek(out, 1, std::ios::cur); //what is this?

        out.write(reinterpret_cast<const char*>(&maxFrame), sizeof(maxFrame));
        out.write(reinterpret_cast<const char*>(&startFrame), sizeof(startFrame));
        out.write(reinterpret_cast<const char*>(&lifeTime), sizeof(lifeTime));
        out.write(reinterpret_cast<const char*>(&lifeTimeRndm), sizeof(lifeTimeRndm));

        out.write(reinterpret_cast<const char*>(&initialVelOmni), sizeof(initialVelOmni));
        out.write(reinterpret_cast<const char*>(&initialVelAxis), sizeof(initialVelAxis));
        out.write(reinterpret_cast<const char*>(&initialVelRndm), sizeof(initialVelRndm));
        out.write(reinterpret_cast<const char*>(&initialVelDir), sizeof(initialVelDir));
        out.write(reinterpret_cast<const char*>(&initialVelRatio), sizeof(initialVelRatio));

        out.write(reinterpret_cast<const char*>(&spread), sizeof(spread));
        out.write(reinterpret_cast<const char*>(&airResist), sizeof(airResist));
        out.write(reinterpret_cast<const char*>(&airResistRndm), sizeof(airResistRndm));

        out.write(reinterpret_cast<const char*>(&moment), sizeof(moment));
        out.write(reinterpret_cast<const char*>(&momentRndm), sizeof(momentRndm));
        out.write(reinterpret_cast<const char*>(&accel), sizeof(accel));
        out.write(reinterpret_cast<const char*>(&accelRndm), sizeof(accelRndm));

        writeVec3(out, emitterScale);
        writeVec3(out, emitterTrans);
        writeVec3(out, emitterDir);
        writeVec3(out, emitterRot);

        return JPCError::NONE;
    }


    JPCError BSP1::read(std::istream& jpc) {
        const std::streamoff sectionStart = jpc.tellg();

        if (!jpc.read(magic, 4)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        jpc.seekg(4, std::ios::cur);

        if (std::strncmp(magic, "BSP1", 4) != 0) LOG_ERR_AND_RETURN(JPCError::UNKNOWN_CHUNK);

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        if (!jpc.read(reinterpret_cast<char*>(&flags), sizeof(flags))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        Utility::Endian::toPlatform_inplace(eType::Big, flags);

        shapeType = static_cast<ShapeType>(flags & 0xF);
        dirType = static_cast<DirType>((flags >> 0x4) & 0x7);
        rotType = static_cast<RotType>((flags >> 0x7) & 0x7);
        planeType = static_cast<PlaneType>((flags >> 0xA) & 0x1);
        if (shapeType == ShapeType::DIRECTION_CROSS || shapeType == ShapeType::ROTATION_CROSS) {
            planeType = PlaneType::X;
        }

        colorInSelect = (flags >> 0xF) & 0x07;
        alphaInSelect = (flags >> 0x12) & 0x1;
        isEnableTexScrollAnm = (flags & 0x01000000) != 0;
        isDrawPrntAhead = (flags & 0x00400000) != 0;
        isDrawFwdAhead = (flags & 0x00200000) != 0;
        isEnableProjection = (flags & 0x00100000) != 0;
        isGlblTexAnm = (flags & 0x00004000) != 0;
        isGlblClrAnm = (flags & 0x00001000) != 0;

        uint16_t colorPrmAnimDataOff;
        uint16_t colorEnvAnimDataOff;
        if (!jpc.read(reinterpret_cast<char*>(&colorPrmAnimDataOff), sizeof(colorPrmAnimDataOff))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&colorEnvAnimDataOff), sizeof(colorEnvAnimDataOff))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        Utility::Endian::toPlatform_inplace(eType::Big, colorPrmAnimDataOff);
        Utility::Endian::toPlatform_inplace(eType::Big, colorEnvAnimDataOff);

        if (!readVec2(jpc, jpc.tellg(), baseSize)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        if (!jpc.read(reinterpret_cast<char*>(&anmRndm), sizeof(anmRndm))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        Utility::Endian::toPlatform_inplace(eType::Big, anmRndm);

        colorLoopOfstMask = -((flags >> 0x0B) & 0x01);
        texIdxLoopOfstMask = -((flags >> 0x0D) & 0x01);

        uint8_t texIdxAnimCount;
        uint8_t colorPrmAnimDataCount;
        uint8_t colorEnvAnimDataCount;
        if (!jpc.read(reinterpret_cast<char*>(&blendModeFlags), sizeof(blendModeFlags))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&alphaCompareFlags), sizeof(alphaCompareFlags))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&alphaRef0), sizeof(alphaRef0))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&alphaRef1), sizeof(alphaRef1))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&zModeFlags), sizeof(zModeFlags))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&texFlags), sizeof(texFlags))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&texIdxAnimCount), sizeof(texIdxAnimCount))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&texIdx), sizeof(texIdx))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&colorFlags), sizeof(colorFlags))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&colorPrmAnimDataCount), sizeof(colorPrmAnimDataCount))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&colorEnvAnimDataCount), sizeof(colorEnvAnimDataCount))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&colorAnimMaxFrm), sizeof(colorAnimMaxFrm))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        Utility::Endian::toPlatform_inplace(eType::Big, blendModeFlags);
        Utility::Endian::toPlatform_inplace(eType::Big, colorAnimMaxFrm);

        if (!readRGBA(jpc, jpc.tellg(), colorPrm)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!readRGBA(jpc, jpc.tellg(), colorEnv)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        colorCalcIdxType = static_cast<CalcIdxType>((colorFlags >> 4) & 0x07);

        if (!jpc.read(reinterpret_cast<char*>(&tilingS), sizeof(tilingS))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&tilingT), sizeof(tilingT))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        Utility::Endian::toPlatform_inplace(eType::Big, tilingS);
        Utility::Endian::toPlatform_inplace(eType::Big, tilingT);

        texCalcIdxType = static_cast<CalcIdxType>((texFlags >> 2) & 0x07);

        if (!readVec2(jpc, jpc.tellg(), texInitTrans)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!readVec2(jpc, jpc.tellg(), texInitScale)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!readVec2(jpc, jpc.tellg(), texIncTrans)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!readVec2(jpc, jpc.tellg(), texIncScale)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&texIncRot), sizeof(texIncRot))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        Utility::Endian::toPlatform_inplace(eType::Big, texIncRot);

        if ((texFlags & 0x00000001) != 0) {
            for (uint8_t i = 0; i < texIdxAnimCount; i++) {
                uint8_t& val = texIdxAnimData.emplace_back();
                if (!jpc.read(reinterpret_cast<char*>(&val), sizeof(val))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
            }
        }

        isEnableTexture = (texFlags & 0x00000002) != 0;

        if ((colorFlags & 0x02) != 0) {
            if (colorPrmAnimData = readColorTable(jpc, sectionStart + colorPrmAnimDataOff, colorPrmAnimDataCount); colorPrmAnimData.empty()) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        }

        if ((colorFlags & 0x08) != 0) {
            if (colorEnvAnimData = readColorTable(jpc, sectionStart + colorEnvAnimDataOff, colorEnvAnimDataCount); colorEnvAnimData.empty()) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        }

        return JPCError::NONE;
    }

    JPCError BSP1::save_changes(std::ostream& out) {
        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        out.write(magic, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
        Utility::seek(out, 4, std::ios::cur);

        flags = (flags & ~0x00038000) | ((colorInSelect & 0x7) << 0xF);
        flags = (flags & ~0x00040000) | ((alphaInSelect & 0x1) << 0x12);
        flags = (flags & ~0x01000000) | ((isEnableTexScrollAnm & 0x1) << 0x18);
        flags = (flags & ~0x00400000) | ((isDrawPrntAhead & 0x1) << 0x16);
        flags = (flags & ~0x00200000) | ((isDrawFwdAhead & 0x1) << 0x15);
        flags = (flags & ~0x00100000) | ((isEnableProjection & 0x1) << 0x14);
        flags = (flags & ~0x00004000) | ((isGlblTexAnm & 0x1) << 0xE);
        flags = (flags & ~0x00001000) | ((isGlblClrAnm & 0x1) << 0xC);
        flags = (flags & ~0x00000800) | ((-colorLoopOfstMask & 0x1) << 0xB);
        flags = (flags & ~0x00002000) | ((-texIdxLoopOfstMask & 0x1) << 0xD);

        flags = (flags & ~0x0000000F) | ((static_cast<uint8_t>(shapeType) & 0xF) << 0x0);
        flags = (flags & ~0x00000070) | ((static_cast<uint8_t>(dirType) & 0x7) << 0x4);
        flags = (flags & ~0x00000380) | ((static_cast<uint8_t>(rotType) & 0x7) << 0x7);
        flags = (flags & ~0x00000400) | ((static_cast<uint8_t>(planeType) & 0x1) << 0xA);

        Utility::Endian::toPlatform_inplace(eType::Big, flags);

        out.write(reinterpret_cast<const char*>(&flags), sizeof(flags));

        uint16_t colorPrmAnimDataOff = 0;
        uint16_t colorEnvAnimDataOff = 0;

        colorPrmAnimDataOff = 0x60 + texIdxAnimData.size();
        unsigned int numPaddingBytes = 4 - (colorPrmAnimDataOff % 4);
        if (numPaddingBytes == 4) {
            numPaddingBytes = 0;
        }
        colorPrmAnimDataOff += numPaddingBytes;

        if (!colorEnvAnimData.empty()) colorEnvAnimDataOff = colorPrmAnimDataOff + colorPrmAnimData.size() * 0x6;
        numPaddingBytes = 4 - (colorEnvAnimDataOff % 4);
        if (numPaddingBytes == 4) {
            numPaddingBytes = 0;
        }
        colorEnvAnimDataOff += numPaddingBytes;

        if (colorPrmAnimData.empty()) colorPrmAnimDataOff = 0;
        if (colorEnvAnimData.empty()) colorEnvAnimDataOff = 0;

        Utility::Endian::toPlatform_inplace(eType::Big, colorPrmAnimDataOff);
        Utility::Endian::toPlatform_inplace(eType::Big, colorEnvAnimDataOff);

        out.write(reinterpret_cast<const char*>(&colorPrmAnimDataOff), sizeof(colorPrmAnimDataOff));
        out.write(reinterpret_cast<const char*>(&colorEnvAnimDataOff), sizeof(colorEnvAnimDataOff));

        writeVec2(out, baseSize);

        Utility::Endian::toPlatform_inplace(eType::Big, anmRndm);

        out.write(reinterpret_cast<const char*>(&anmRndm), sizeof(anmRndm));


        uint8_t texIdxAnimCount = texIdxAnimData.size();
        uint8_t colorPrmAnimDataCount = colorPrmAnimData.size();
        uint8_t colorEnvAnimDataCount = colorEnvAnimData.size();

        Utility::Endian::toPlatform_inplace(eType::Big, blendModeFlags);
        Utility::Endian::toPlatform_inplace(eType::Big, colorAnimMaxFrm);

        colorFlags = (colorFlags & ~0x70) | ((static_cast<uint8_t>(colorCalcIdxType) & 0x07) << 4);
        colorFlags &= ~0x02;
        if (!colorPrmAnimData.empty()) colorFlags |= 0x2;
        colorFlags &= ~0x08;
        if (!colorEnvAnimData.empty()) colorFlags |= 0x8;

        texFlags = (texFlags & ~0x28) | ((static_cast<uint8_t>(texCalcIdxType) & 0x07) << 2);
        texFlags &= ~0x01;
        if (texIdxAnimData.empty()) texFlags |= 0x1;

        texFlags &= ~0x02;
    	if(isEnableTexture) texFlags |= 0x2;

        out.write(reinterpret_cast<const char*>(&blendModeFlags), sizeof(blendModeFlags));
        out.write(reinterpret_cast<const char*>(&alphaCompareFlags), sizeof(alphaCompareFlags));
        out.write(reinterpret_cast<const char*>(&alphaRef0), sizeof(alphaRef0));
        out.write(reinterpret_cast<const char*>(&alphaRef1), sizeof(alphaRef1));
        out.write(reinterpret_cast<const char*>(&zModeFlags), sizeof(zModeFlags));
        out.write(reinterpret_cast<const char*>(&texFlags), sizeof(texFlags));
        out.write(reinterpret_cast<const char*>(&texIdxAnimCount), sizeof(texIdxAnimCount));
        out.write(reinterpret_cast<const char*>(&texIdx), sizeof(texIdx));
        out.write(reinterpret_cast<const char*>(&colorFlags), sizeof(colorFlags));
        out.write(reinterpret_cast<const char*>(&colorPrmAnimDataCount), sizeof(colorPrmAnimDataCount));
        out.write(reinterpret_cast<const char*>(&colorEnvAnimDataCount), sizeof(colorEnvAnimDataCount));
        out.write(reinterpret_cast<const char*>(&colorAnimMaxFrm), sizeof(colorAnimMaxFrm));

        writeRGBA(out, colorPrm);
        writeRGBA(out, colorEnv);

        Utility::Endian::toPlatform_inplace(eType::Big, tilingS);
        Utility::Endian::toPlatform_inplace(eType::Big, tilingT);

        out.write(reinterpret_cast<const char*>(&tilingS), sizeof(tilingS));
        out.write(reinterpret_cast<const char*>(&tilingT), sizeof(tilingT));

        writeVec2(out, texInitTrans);
        writeVec2(out, texInitScale);
        writeVec2(out, texIncTrans);
        writeVec2(out, texIncScale);

        Utility::Endian::toPlatform_inplace(eType::Big, texIncRot);

        out.write(reinterpret_cast<const char*>(&texIncRot), sizeof(texIncRot));

        for (const uint8_t& val : texIdxAnimData) {
            out.write(reinterpret_cast<const char*>(&val), sizeof(val));
        }
        padToLen(out, 4);

        if ((colorFlags & 0x02) != 0) {
            writeColorTable(out, colorPrmAnimData);
        }
        padToLen(out, 4);

        if ((colorFlags & 0x08) != 0) {
            writeColorTable(out, colorEnvAnimData);
        }

        return JPCError::NONE;
    }


    JPCError ESP1::read(std::istream& jpc) {
        if (!jpc.read(magic, 4)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        jpc.seekg(4, std::ios::cur);

        if (std::strncmp(magic, "ESP1", 4) != 0) LOG_ERR_AND_RETURN(JPCError::UNKNOWN_CHUNK);

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        if (!jpc.read(reinterpret_cast<char*>(&flags), sizeof(flags))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        Utility::Endian::toPlatform_inplace(eType::Big, flags);

        isEnableScale = (flags & 0x00000100) != 0;
        isDiffXY = (flags & 0x00000200) != 0;
        isEnableScaleAnmY = (flags & 0x00000400) != 0;
        isEnableScaleAnmX = (flags & 0x00000800) != 0;
        isEnableScaleBySpeedY = (flags & 0x00001000) != 0;
        isEnableScaleBySpeedX = (flags & 0x00002000) != 0;
        isEnableAlpha = (flags & 0x00000001) != 0;
        isEnableSinWave = (flags & 0x00000002) != 0;
        isEnableRotate = (flags & 0x01000000) != 0;

        alphaWaveType = static_cast<CalcAlphaWaveType>((flags >> 0x02) & 0x03);

        anmTypeX = ((flags >> 0x12) & 0x01) != 0;
        anmTypeY = ((flags >> 0x13) & 0x01) != 0;
        pivotX = (flags >> 0x0E) & 0x03;
        pivotY = (flags >> 0x10) & 0x03;

        jpc.seekg(4, std::ios::cur);

        if (!jpc.read(reinterpret_cast<char*>(&alphaInTiming), sizeof(alphaInTiming))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&alphaOutTiming), sizeof(alphaOutTiming))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&alphaInValue), sizeof(alphaInValue))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&alphaBaseValue), sizeof(alphaBaseValue))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&alphaOutValue), sizeof(alphaOutValue))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        if (!jpc.read(reinterpret_cast<char*>(&alphaWaveParam1), sizeof(alphaWaveParam1))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&alphaWaveParam2), sizeof(alphaWaveParam2))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&alphaWaveParam3), sizeof(alphaWaveParam3))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&alphaWaveRandom), sizeof(alphaWaveRandom))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        if (!jpc.read(reinterpret_cast<char*>(&scaleInTiming), sizeof(scaleInTiming))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&scaleOutTiming), sizeof(scaleOutTiming))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&scaleInValueX), sizeof(scaleInValueX))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&scaleOutValueX), sizeof(scaleOutValueX))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&scaleInValueY), sizeof(scaleInValueY))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&scaleOutValueY), sizeof(scaleOutValueY))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&scaleOutRandom), sizeof(scaleOutRandom))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!readVec2(jpc, jpc.tellg(), scaleAnmMaxFrame)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        if (!jpc.read(reinterpret_cast<char*>(&rotateAngle), sizeof(rotateAngle))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&rotateSpeed), sizeof(rotateSpeed))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&rotateAngleRandom), sizeof(rotateAngleRandom))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&rotateSpeedRandom), sizeof(rotateSpeedRandom))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&rotateDirection), sizeof(rotateDirection))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        Utility::Endian::toPlatform_inplace(eType::Big, alphaInTiming);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaOutTiming);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaInValue);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaBaseValue);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaOutValue);

        Utility::Endian::toPlatform_inplace(eType::Big, alphaWaveParam1);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaWaveParam2);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaWaveParam3);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaWaveRandom);

        Utility::Endian::toPlatform_inplace(eType::Big, scaleInTiming);
        Utility::Endian::toPlatform_inplace(eType::Big, scaleOutTiming);
        Utility::Endian::toPlatform_inplace(eType::Big, scaleInValueX);
        Utility::Endian::toPlatform_inplace(eType::Big, scaleOutValueX);
        Utility::Endian::toPlatform_inplace(eType::Big, scaleInValueY);
        Utility::Endian::toPlatform_inplace(eType::Big, scaleOutValueY);
        Utility::Endian::toPlatform_inplace(eType::Big, scaleOutRandom);

        Utility::Endian::toPlatform_inplace(eType::Big, rotateAngle);
        Utility::Endian::toPlatform_inplace(eType::Big, rotateSpeed);
        Utility::Endian::toPlatform_inplace(eType::Big, rotateAngleRandom);
        Utility::Endian::toPlatform_inplace(eType::Big, rotateSpeedRandom);
        Utility::Endian::toPlatform_inplace(eType::Big, rotateDirection);

        return JPCError::NONE;
    }

    JPCError ESP1::save_changes(std::ostream& out) {
        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        out.write(magic, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
        Utility::seek(out, 4, std::ios::cur);

        flags = (flags & ~0x00000100) | ((isEnableScale & 0x1) << 0x8);
        flags = (flags & ~0x00000200) | ((isDiffXY & 0x1) << 0x9);

        flags = (flags & ~0x00000400) | ((isEnableScaleAnmY & 0x1) << 0xA);
        flags = (flags & ~0x00000800) | ((isEnableScaleAnmX & 0x1) << 0xB);
        flags = (flags & ~0x00040000) | ((anmTypeX & 0x1) << 0x12);
        flags = (flags & ~0x00080000) | ((anmTypeY & 0x1) << 0x13);

        flags = (flags & ~0x00001000) | ((isEnableScaleBySpeedY & 0x1) << 0xC);
        flags = (flags & ~0x00002000) | ((isEnableScaleBySpeedX & 0x1) << 0xD);
        flags = (flags & ~0x00000001) | (isEnableAlpha & 0x1);
        flags = (flags & ~0x00000002) | ((isEnableSinWave & 0x1) << 0x1);
        flags = (flags & ~0x01000000) | ((isEnableRotate & 0x1) << 0x18);

        flags = (flags & ~0x0000000C) | ((static_cast<uint8_t>(alphaWaveType) & 0x3) << 0x2);

        flags = (flags & ~0x0000C000) | ((pivotX & 0x3) << 0x0E);
        flags = (flags & ~0x00030000) | ((pivotY & 0x3) << 0x10);

        Utility::Endian::toPlatform_inplace(eType::Big, flags);

        out.write(reinterpret_cast<const char*>(&flags), sizeof(flags));

        Utility::seek(out, 4, std::ios::cur);

        Utility::Endian::toPlatform_inplace(eType::Big, alphaInTiming);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaOutTiming);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaInValue);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaBaseValue);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaOutValue);

        Utility::Endian::toPlatform_inplace(eType::Big, alphaWaveParam1);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaWaveParam2);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaWaveParam3);
        Utility::Endian::toPlatform_inplace(eType::Big, alphaWaveRandom);

        Utility::Endian::toPlatform_inplace(eType::Big, scaleInTiming);
        Utility::Endian::toPlatform_inplace(eType::Big, scaleOutTiming);
        Utility::Endian::toPlatform_inplace(eType::Big, scaleInValueX);
        Utility::Endian::toPlatform_inplace(eType::Big, scaleOutValueX);
        Utility::Endian::toPlatform_inplace(eType::Big, scaleInValueY);
        Utility::Endian::toPlatform_inplace(eType::Big, scaleOutValueY);
        Utility::Endian::toPlatform_inplace(eType::Big, scaleOutRandom);

        Utility::Endian::toPlatform_inplace(eType::Big, rotateAngle);
        Utility::Endian::toPlatform_inplace(eType::Big, rotateSpeed);
        Utility::Endian::toPlatform_inplace(eType::Big, rotateAngleRandom);
        Utility::Endian::toPlatform_inplace(eType::Big, rotateSpeedRandom);
        Utility::Endian::toPlatform_inplace(eType::Big, rotateDirection);

        out.write(reinterpret_cast<const char*>(&alphaInTiming), sizeof(alphaInTiming));
        out.write(reinterpret_cast<const char*>(&alphaOutTiming), sizeof(alphaOutTiming));
        out.write(reinterpret_cast<const char*>(&alphaInValue), sizeof(alphaInValue));
        out.write(reinterpret_cast<const char*>(&alphaBaseValue), sizeof(alphaBaseValue));
        out.write(reinterpret_cast<const char*>(&alphaOutValue), sizeof(alphaOutValue));

        out.write(reinterpret_cast<const char*>(&alphaWaveParam1), sizeof(alphaWaveParam1));
        out.write(reinterpret_cast<const char*>(&alphaWaveParam2), sizeof(alphaWaveParam2));
        out.write(reinterpret_cast<const char*>(&alphaWaveParam3), sizeof(alphaWaveParam3));
        out.write(reinterpret_cast<const char*>(&alphaWaveRandom), sizeof(alphaWaveRandom));

        out.write(reinterpret_cast<const char*>(&scaleInTiming), sizeof(scaleInTiming));
        out.write(reinterpret_cast<const char*>(&scaleOutTiming), sizeof(scaleOutTiming));
        out.write(reinterpret_cast<const char*>(&scaleInValueX), sizeof(scaleInValueX));
        out.write(reinterpret_cast<const char*>(&scaleOutValueX), sizeof(scaleOutValueX));
        out.write(reinterpret_cast<const char*>(&scaleInValueY), sizeof(scaleInValueY));
        out.write(reinterpret_cast<const char*>(&scaleOutValueY), sizeof(scaleOutValueY));
        out.write(reinterpret_cast<const char*>(&scaleOutRandom), sizeof(scaleOutRandom));
        writeVec2(out, scaleAnmMaxFrame);

        out.write(reinterpret_cast<const char*>(&rotateAngle), sizeof(rotateAngle));
        out.write(reinterpret_cast<const char*>(&rotateSpeed), sizeof(rotateSpeed));
        out.write(reinterpret_cast<const char*>(&rotateAngleRandom), sizeof(rotateAngleRandom));
        out.write(reinterpret_cast<const char*>(&rotateSpeedRandom), sizeof(rotateSpeedRandom));
        out.write(reinterpret_cast<const char*>(&rotateDirection), sizeof(rotateDirection));

        return JPCError::NONE;
    }


    JPCError ETX1::read(std::istream& jpc) {
        if (!jpc.read(magic, 4)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        jpc.seekg(4, std::ios::cur);

        if (std::strncmp(magic, "ETX1", 4) != 0) LOG_ERR_AND_RETURN(JPCError::UNKNOWN_CHUNK);

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        if (!jpc.read(reinterpret_cast<char*>(&flags), sizeof(flags))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        Utility::Endian::toPlatform_inplace(eType::Big, flags);

        if (!jpc.read(reinterpret_cast<char*>(&p00), sizeof(p00))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&p01), sizeof(p01))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&p02), sizeof(p02))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&p10), sizeof(p10))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&p11), sizeof(p11))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&p12), sizeof(p12))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&scale), sizeof(scale))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        Utility::Endian::toPlatform_inplace(eType::Big, p00);
        Utility::Endian::toPlatform_inplace(eType::Big, p01);
        Utility::Endian::toPlatform_inplace(eType::Big, p02);
        Utility::Endian::toPlatform_inplace(eType::Big, p10);
        Utility::Endian::toPlatform_inplace(eType::Big, p11);
        Utility::Endian::toPlatform_inplace(eType::Big, p12);
        Utility::Endian::toPlatform_inplace(eType::Big, scale);

        indTextureMode = static_cast<IndTextureMode>(flags & 0x03);
        if (!jpc.read(reinterpret_cast<char*>(&indTextureID), sizeof(indTextureID))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&subTextureID), sizeof(subTextureID))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&secondTextureIndex), sizeof(secondTextureIndex))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        useSecondTex = (flags & 0x00000100) != 0;

        return JPCError::NONE;
    }

    JPCError ETX1::save_changes(std::ostream& out) {
        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        out.write(magic, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
        Utility::seek(out, 4, std::ios::cur);

        flags = (flags & ~0x00000003) | (static_cast<uint8_t>(indTextureMode) & 0x00000003);
        flags = (flags & ~0x00000100) | ((useSecondTex & 0x1) << 8);

        Utility::Endian::toPlatform_inplace(eType::Big, flags);

        out.write(reinterpret_cast<const char*>(&flags), sizeof(flags));

        Utility::Endian::toPlatform_inplace(eType::Big, p00);
        Utility::Endian::toPlatform_inplace(eType::Big, p01);
        Utility::Endian::toPlatform_inplace(eType::Big, p02);
        Utility::Endian::toPlatform_inplace(eType::Big, p10);
        Utility::Endian::toPlatform_inplace(eType::Big, p11);
        Utility::Endian::toPlatform_inplace(eType::Big, p12);
        Utility::Endian::toPlatform_inplace(eType::Big, scale);

        out.write(reinterpret_cast<const char*>(&p00), sizeof(p00));
        out.write(reinterpret_cast<const char*>(&p01), sizeof(p01));
        out.write(reinterpret_cast<const char*>(&p02), sizeof(p02));
        out.write(reinterpret_cast<const char*>(&p10), sizeof(p10));
        out.write(reinterpret_cast<const char*>(&p11), sizeof(p11));
        out.write(reinterpret_cast<const char*>(&p12), sizeof(p12));
        out.write(reinterpret_cast<const char*>(&scale), sizeof(scale));

        out.write(reinterpret_cast<const char*>(&indTextureID), sizeof(indTextureID));
        out.write(reinterpret_cast<const char*>(&subTextureID), sizeof(subTextureID));

        out.write(reinterpret_cast<const char*>(&secondTextureIndex), sizeof(secondTextureIndex));

        return JPCError::NONE;
    }


    JPCError SSP1::read(std::istream& jpc) {
        if (!jpc.read(magic, 4)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        jpc.seekg(4, std::ios::cur);

        if (std::strncmp(magic, "SSP1", 4) != 0) LOG_ERR_AND_RETURN(JPCError::UNKNOWN_CHUNK);

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        if (!jpc.read(reinterpret_cast<char*>(&flags), sizeof(flags))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        Utility::Endian::toPlatform_inplace(eType::Big, flags);

        shapeType = static_cast<ShapeType>(flags & 0xF);
        dirType = static_cast<DirType>((flags >> 0x4) & 0x7);
        rotType = static_cast<RotType>((flags >> 0x7) & 0x7);
        planeType = static_cast<PlaneType>((flags >> 0xA) & 0x1);
        if (shapeType == ShapeType::DIRECTION_CROSS || shapeType == ShapeType::ROTATION_CROSS) {
            planeType = PlaneType::X;
        }

        isDrawParent = (flags & 0x00080000) != 0;

        if (!jpc.read(reinterpret_cast<char*>(&posRndm), sizeof(posRndm))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&baseVel), sizeof(baseVel))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&baseVelRndm), sizeof(baseVelRndm))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&velInfRate), sizeof(velInfRate))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&gravity), sizeof(gravity))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&timing), sizeof(timing))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&life), sizeof(life))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&rate), sizeof(rate))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&step), sizeof(step))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        if (!readVec2(jpc, jpc.tellg(), globalScale2D)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        if (!jpc.read(reinterpret_cast<char*>(&rotateSpeed), sizeof(rotateSpeed))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        isEnableRotate = (flags & 0x01000000) != 0;
        isEnableAlphaOut = (flags & 0x00800000) != 0;
        isEnableScaleOut = (flags & 0x00400000) != 0;
        isEnableField = (flags & 0x00200000) != 0;
        isInheritedRGB = (flags & 0x00040000) != 0;
        isInheritedAlpha = (flags & 0x00020000) != 0;
        isInheritedScale = (flags & 0x00010000) != 0;

        if (!jpc.read(reinterpret_cast<char*>(&inheritScale), sizeof(inheritScale))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&inheritAlpha), sizeof(inheritAlpha))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&inheritRGB), sizeof(inheritRGB))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!readRGBA(jpc, jpc.tellg(), colorPrm)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!readRGBA(jpc, jpc.tellg(), colorEnv)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&texIdx), sizeof(texIdx))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        Utility::Endian::toPlatform_inplace(eType::Big, posRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, baseVel);
        Utility::Endian::toPlatform_inplace(eType::Big, baseVelRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, velInfRate);
        Utility::Endian::toPlatform_inplace(eType::Big, gravity);
        Utility::Endian::toPlatform_inplace(eType::Big, timing);
        Utility::Endian::toPlatform_inplace(eType::Big, life);
        Utility::Endian::toPlatform_inplace(eType::Big, rate);
        Utility::Endian::toPlatform_inplace(eType::Big, step);
        Utility::Endian::toPlatform_inplace(eType::Big, rotateSpeed);
        Utility::Endian::toPlatform_inplace(eType::Big, inheritRGB);
        Utility::Endian::toPlatform_inplace(eType::Big, inheritAlpha);
        Utility::Endian::toPlatform_inplace(eType::Big, inheritRGB);

        return JPCError::NONE;
    }

    JPCError SSP1::save_changes(std::ostream& out) {
        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        out.write(magic, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
        Utility::seek(out, 4, std::ios::cur);

        flags = (flags & ~0x01000000) | ((isEnableRotate & 0x1) << 0x18);
        flags = (flags & ~0x00800000) | ((isEnableAlphaOut & 0x1) << 0x17);
        flags = (flags & ~0x00400000) | ((isEnableScaleOut & 0x1) << 0x16);
        flags = (flags & ~0x00200000) | ((isEnableField & 0x1) << 0x15);
        flags = (flags & ~0x00080000) | ((isDrawParent & 0x1) << 0x13);
        flags = (flags & ~0x00040000) | ((isInheritedRGB & 0x1) << 0x12);
        flags = (flags & ~0x00020000) | ((isInheritedAlpha & 0x1) << 0x11);
        flags = (flags & ~0x00010000) | ((isInheritedScale & 0x1) << 0x10);

        flags = (flags & ~0x0000000F) | ((static_cast<uint8_t>(shapeType) & 0xF) << 0x0);
        flags = (flags & ~0x00000070) | ((static_cast<uint8_t>(dirType) & 0x7) << 0x4);
        flags = (flags & ~0x00000380) | ((static_cast<uint8_t>(rotType) & 0x7) << 0x7);
        flags = (flags & ~0x00000400) | ((static_cast<uint8_t>(planeType) & 0x1) << 0xA);


        Utility::Endian::toPlatform_inplace(eType::Big, flags);

        out.write(reinterpret_cast<const char*>(&flags), sizeof(flags));

        Utility::Endian::toPlatform_inplace(eType::Big, posRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, baseVel);
        Utility::Endian::toPlatform_inplace(eType::Big, baseVelRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, velInfRate);
        Utility::Endian::toPlatform_inplace(eType::Big, gravity);
        Utility::Endian::toPlatform_inplace(eType::Big, timing);
        Utility::Endian::toPlatform_inplace(eType::Big, life);
        Utility::Endian::toPlatform_inplace(eType::Big, rate);
        Utility::Endian::toPlatform_inplace(eType::Big, step);
        Utility::Endian::toPlatform_inplace(eType::Big, rotateSpeed);
        Utility::Endian::toPlatform_inplace(eType::Big, inheritRGB);
        Utility::Endian::toPlatform_inplace(eType::Big, inheritAlpha);
        Utility::Endian::toPlatform_inplace(eType::Big, inheritRGB);

        out.write(reinterpret_cast<const char*>(&posRndm), sizeof(posRndm));
        out.write(reinterpret_cast<const char*>(&baseVel), sizeof(baseVel));
        out.write(reinterpret_cast<const char*>(&baseVelRndm), sizeof(baseVelRndm));
        out.write(reinterpret_cast<const char*>(&velInfRate), sizeof(velInfRate));
        out.write(reinterpret_cast<const char*>(&gravity), sizeof(gravity));
        out.write(reinterpret_cast<const char*>(&timing), sizeof(timing));
        out.write(reinterpret_cast<const char*>(&life), sizeof(life));
        out.write(reinterpret_cast<const char*>(&rate), sizeof(rate));
        out.write(reinterpret_cast<const char*>(&step), sizeof(step));

        writeVec2(out, globalScale2D);

        out.write(reinterpret_cast<const char*>(&rotateSpeed), sizeof(rotateSpeed));

        out.write(reinterpret_cast<const char*>(&inheritScale), sizeof(inheritScale));
        out.write(reinterpret_cast<const char*>(&inheritAlpha), sizeof(inheritAlpha));
        out.write(reinterpret_cast<const char*>(&inheritRGB), sizeof(inheritRGB));
        writeRGBA(out, colorPrm);
        writeRGBA(out, colorEnv);
        out.write(reinterpret_cast<const char*>(&texIdx), sizeof(texIdx));

        return JPCError::NONE;
    }


    JPCError FLD1::read(std::istream& jpc) {
        if (!jpc.read(magic, 4)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        jpc.seekg(4, std::ios::cur);

        if (std::strncmp(magic, "FLD1", 4) != 0) LOG_ERR_AND_RETURN(JPCError::UNKNOWN_CHUNK);

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        if (!jpc.read(reinterpret_cast<char*>(&flags), sizeof(flags))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        Utility::Endian::toPlatform_inplace(eType::Big, flags);

        statusFlag = static_cast<FieldStatusFlag>(flags >> 0x10);
        type = static_cast<FieldType>(flags & 0xF);
        addType = static_cast<FieldAddType>((flags >> 8) & 0x03);

        if (!jpc.read(reinterpret_cast<char*>(&mag), sizeof(mag))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&magRndm), sizeof(magRndm))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&maxDist), sizeof(maxDist))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        if (!readVec3(jpc, jpc.tellg(), pos)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        if (!readVec3(jpc, jpc.tellg(), dir)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        if (!jpc.read(reinterpret_cast<char*>(&param1), sizeof(param1))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&param2), sizeof(param2))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&param3), sizeof(param3))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&fadeIn), sizeof(fadeIn))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&fadeOut), sizeof(fadeOut))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&enTime), sizeof(enTime))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&disTime), sizeof(disTime))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&cycle), sizeof(cycle))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&unk1), sizeof(unk1))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        Utility::Endian::toPlatform_inplace(eType::Big, mag);
        Utility::Endian::toPlatform_inplace(eType::Big, magRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, maxDist);
        Utility::Endian::toPlatform_inplace(eType::Big, param1);
        Utility::Endian::toPlatform_inplace(eType::Big, param2);
        Utility::Endian::toPlatform_inplace(eType::Big, param3);
        Utility::Endian::toPlatform_inplace(eType::Big, fadeIn);
        Utility::Endian::toPlatform_inplace(eType::Big, fadeOut);
        Utility::Endian::toPlatform_inplace(eType::Big, enTime);
        Utility::Endian::toPlatform_inplace(eType::Big, disTime);

        //refDistance = -1.0f;
        //innerSpeed = -1.0f;
        //outerSpeed = -1.0f;

        //if (type == FieldType::NEWTON) {
        //    refDistance = pow(param1, 2);
        //}
        //else if (type == FieldType::VORTEX) {
        //    innerSpeed = mag;
        //    outerSpeed = magRndm;
        //}
        //else if (type == FieldType::AIR) {
        //    refDistance = magRndm;
        //}
        //else if (type == FieldType::CONVECTION) {
        //    refDistance = param2;
        //}
        //else if (type == FieldType::SPIN) {
        //    innerSpeed = mag;
        //}

        return JPCError::NONE;
    }

    JPCError FLD1::save_changes(std::ostream& out) {
        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        out.write(magic, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
        Utility::seek(out, 4, std::ios::cur);

        flags = (flags & ~0x00FF0000) | (static_cast<uint8_t>(statusFlag) << 0x10);
        flags = (flags & ~0x00000300) | ((static_cast<uint8_t>(addType) & 0x3) << 0x8);
        flags = (flags & ~0x0000000F) | (static_cast<uint8_t>(type) & 0xF);

        Utility::Endian::toPlatform_inplace(eType::Big, flags);

        out.write(reinterpret_cast<const char*>(&flags), sizeof(flags));

        Utility::Endian::toPlatform_inplace(eType::Big, mag);
        Utility::Endian::toPlatform_inplace(eType::Big, magRndm);
        Utility::Endian::toPlatform_inplace(eType::Big, maxDist);
        Utility::Endian::toPlatform_inplace(eType::Big, param1);
        Utility::Endian::toPlatform_inplace(eType::Big, param2);
        Utility::Endian::toPlatform_inplace(eType::Big, param3);
        Utility::Endian::toPlatform_inplace(eType::Big, fadeIn);
        Utility::Endian::toPlatform_inplace(eType::Big, fadeOut);
        Utility::Endian::toPlatform_inplace(eType::Big, enTime);
        Utility::Endian::toPlatform_inplace(eType::Big, disTime);

        out.write(reinterpret_cast<const char*>(&mag), sizeof(mag));
        out.write(reinterpret_cast<const char*>(&magRndm), sizeof(magRndm));
        out.write(reinterpret_cast<const char*>(&maxDist), sizeof(maxDist));

        writeVec3(out, pos);
        writeVec3(out, dir);

        out.write(reinterpret_cast<const char*>(&param1), sizeof(param1));
        out.write(reinterpret_cast<const char*>(&param2), sizeof(param2));
        out.write(reinterpret_cast<const char*>(&param3), sizeof(param3));
        out.write(reinterpret_cast<const char*>(&fadeIn), sizeof(fadeIn));
        out.write(reinterpret_cast<const char*>(&fadeOut), sizeof(fadeOut));
        out.write(reinterpret_cast<const char*>(&enTime), sizeof(enTime));
        out.write(reinterpret_cast<const char*>(&disTime), sizeof(disTime));
        out.write(reinterpret_cast<const char*>(&cycle), sizeof(cycle));
        out.write(reinterpret_cast<const char*>(&unk1), sizeof(unk1));

        return JPCError::NONE;
    }


    JPCError KFA1::read(std::istream& jpc) {
        if (!jpc.read(magic, 4)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        jpc.seekg(4, std::ios::cur);

        if (std::strncmp(magic, "KFA1", 4) != 0) LOG_ERR_AND_RETURN(JPCError::UNKNOWN_CHUNK);

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        if (!jpc.read(reinterpret_cast<char*>(&keyType), sizeof(keyType))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        jpc.seekg(3, std::ios::cur);
        uint8_t keyCount;
        if (!jpc.read(reinterpret_cast<char*>(&keyCount), sizeof(keyCount))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        if (!jpc.read(reinterpret_cast<char*>(&isLoopEnable), 1)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        isLoopEnable = isLoopEnable != 0;

        if (!jpc.read(reinterpret_cast<char*>(&unk1), sizeof(unk1))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

        jpc.seekg(0xD, std::ios::cur);
        for (uint8_t i = 0; i < keyCount; i++) {
            CurveKeyframe& keyframe = keys.emplace_back();
            if (!jpc.read(reinterpret_cast<char*>(&keyframe.time), sizeof(keyframe.time))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
            if (!jpc.read(reinterpret_cast<char*>(&keyframe.value), sizeof(keyframe.value))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
            if (!jpc.read(reinterpret_cast<char*>(&keyframe.tanIn), sizeof(keyframe.tanIn))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
            if (!jpc.read(reinterpret_cast<char*>(&keyframe.tanOut), sizeof(keyframe.tanOut))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);

            Utility::Endian::toPlatform_inplace(eType::Big, keyframe.time);
            Utility::Endian::toPlatform_inplace(eType::Big, keyframe.value);
            Utility::Endian::toPlatform_inplace(eType::Big, keyframe.tanIn);
            Utility::Endian::toPlatform_inplace(eType::Big, keyframe.tanOut);
        }

        return JPCError::NONE;
    }

    JPCError KFA1::save_changes(std::ostream& out) {
        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        out.write(magic, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
        Utility::seek(out, 4, std::ios::cur);

        out.write(reinterpret_cast<const char*>(&keyType), sizeof(keyType));
        Utility::seek(out, 3, std::ios::cur);
        uint8_t keyCount = keys.size();
        out.write(reinterpret_cast<const char*>(&keyCount), sizeof(keyCount));

        out.write(reinterpret_cast<const char*>(&isLoopEnable), 1);
        out.write(reinterpret_cast<const char*>(&unk1), sizeof(unk1));

        Utility::seek(out, 0xD, std::ios::cur);
        for (CurveKeyframe& keyframe : keys) {
            Utility::Endian::toPlatform_inplace(eType::Big, keyframe.time);
            Utility::Endian::toPlatform_inplace(eType::Big, keyframe.value);
            Utility::Endian::toPlatform_inplace(eType::Big, keyframe.tanIn);
            Utility::Endian::toPlatform_inplace(eType::Big, keyframe.tanOut);

            out.write(reinterpret_cast<const char*>(&keyframe.time), sizeof(keyframe.time));
            out.write(reinterpret_cast<const char*>(&keyframe.value), sizeof(keyframe.value));
            out.write(reinterpret_cast<const char*>(&keyframe.tanIn), sizeof(keyframe.tanIn));
            out.write(reinterpret_cast<const char*>(&keyframe.tanOut), sizeof(keyframe.tanOut));
        }

        return JPCError::NONE;
    }


    JPCError TDB1::read(std::istream& jpc, const uint8_t texCount) {
        if (!jpc.read(magic, 4)) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        jpc.seekg(4, std::ios::cur);

        if (std::strncmp(magic, "TDB1", 4) != 0) LOG_ERR_AND_RETURN(JPCError::UNKNOWN_CHUNK);

        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        for (uint8_t i = 0; i < texCount; i++) {
            uint16_t& index = texIDs.emplace_back();
            if (!jpc.read(reinterpret_cast<char*>(&index), sizeof(index))) LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
            Utility::Endian::toPlatform_inplace(eType::Big, index);
        }

        return JPCError::NONE;
    }

    JPCError TDB1::populateFilenames(const std::vector<std::string>& texList) {
        for (const uint16_t& id : texIDs) {
            if (id > texList.size() - 1) LOG_ERR_AND_RETURN(JPCError::MISSING_TEXTURE);

            texFilenames.push_back(texList[id]);
        }

        return JPCError::NONE;
    }

    JPCError TDB1::save_changes(std::ostream& out, const std::unordered_map<std::string, size_t>& textures) {
        Utility::Endian::toPlatform_inplace(eType::Big, sectionSize);

        out.write(magic, 4);
        out.write(reinterpret_cast<const char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
        Utility::seek(out, 4, std::ios::cur);

        texIDs.clear();
        texIDs.reserve(texFilenames.size());
        for (const std::string& filename : texFilenames) {
            if (!textures.contains(filename)) LOG_ERR_AND_RETURN(JPCError::MISSING_TEXTURE);

            texIDs.push_back(textures.at(filename));
        }

        for (uint16_t& id : texIDs) {
            Utility::Endian::toPlatform_inplace(eType::Big, id);

            out.write(reinterpret_cast<const char*>(&id), sizeof(id));
        }

        return JPCError::NONE;
    }
}

JPCError Particle::read(std::istream& jpc) {
    if (!jpc.read(magicJEFF, 8)) {
        LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
    }
    if (std::strncmp(magicJEFF, "JEFFjpa1", 8) != 0) {
        LOG_ERR_AND_RETURN(JPCError::UNEXPECTED_VALUE);
    }
    if (!jpc.read(reinterpret_cast<char*>(&unknown_1), sizeof(unknown_1))) {
        LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
    }
    if (!jpc.read(reinterpret_cast<char*>(&num_chunks), sizeof(num_chunks))) {
        LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
    }
    if (!jpc.read(reinterpret_cast<char*>(&size), sizeof(size))) {
        LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
    }
    if (!jpc.read(reinterpret_cast<char*>(&num_kfa1_chunks), sizeof(num_kfa1_chunks))) {
        LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
    }
    if (!jpc.read(reinterpret_cast<char*>(&num_fld1_chunks), sizeof(num_fld1_chunks))) {
        LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
    }
    if (!jpc.read(reinterpret_cast<char*>(&num_textures), sizeof(num_textures))) {
        LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
    }
    if (!jpc.read(reinterpret_cast<char*>(&unknown_5), sizeof(unknown_5))) {
        LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
    }
    if (!jpc.read(reinterpret_cast<char*>(&particle_id), sizeof(particle_id))) {
        LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
    }
    if (!jpc.read(reinterpret_cast<char*>(&unknown_6), sizeof(unknown_6))) {
        LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
    }

    Utility::Endian::toPlatform_inplace(eType::Big, unknown_1);
    Utility::Endian::toPlatform_inplace(eType::Big, num_chunks);
    Utility::Endian::toPlatform_inplace(eType::Big, size);
    Utility::Endian::toPlatform_inplace(eType::Big, particle_id);

    for (uint32_t i = 0; i < num_chunks; i++) {
	    char magic[4];
	    unsigned int numPaddingBytes = 0x20 - (jpc.tellg() % 0x20);
        if (numPaddingBytes == 0x20) {
            numPaddingBytes = 0;
        }
        jpc.seekg(numPaddingBytes, std::ios::cur);

        if (!jpc.read(magic, 4)) {
            LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
        }
        jpc.seekg(-4, std::ios::cur);

        if (std::strncmp(magic, "BEM1", 4) == 0) {
            if (emitter.has_value()) LOG_ERR_AND_RETURN(JPCError::DUPLICATE_CHUNK);
            LOG_AND_RETURN_IF_ERR(emitter.emplace().read(jpc));
        }
        else if (std::strncmp(magic, "BSP1", 4) == 0) {
            if (baseShape.has_value()) LOG_ERR_AND_RETURN(JPCError::DUPLICATE_CHUNK);
            LOG_AND_RETURN_IF_ERR(baseShape.emplace().read(jpc));
        }
        else if (std::strncmp(magic, "ESP1", 4) == 0) {
            if (extraShape.has_value()) LOG_ERR_AND_RETURN(JPCError::DUPLICATE_CHUNK);
            LOG_AND_RETURN_IF_ERR(extraShape.emplace().read(jpc));
        }
        else if (std::strncmp(magic, "ETX1", 4) == 0) {
            if (extraTex.has_value()) LOG_ERR_AND_RETURN(JPCError::DUPLICATE_CHUNK);
            LOG_AND_RETURN_IF_ERR(extraTex.emplace().read(jpc))
        }
        else if (std::strncmp(magic, "SSP1", 4) == 0) {
            if (childShape.has_value()) LOG_ERR_AND_RETURN(JPCError::DUPLICATE_CHUNK);
            LOG_AND_RETURN_IF_ERR(childShape.emplace().read(jpc));
        }
        else if (std::strncmp(magic, "FLD1", 4) == 0) {
            LOG_AND_RETURN_IF_ERR(fields.emplace_back().read(jpc));
        }
        else if (std::strncmp(magic, "KFA1", 4) == 0) {
            LOG_AND_RETURN_IF_ERR(curves.emplace_back().read(jpc));
        }
        else if (std::strncmp(magic, "TDB1", 4) == 0) {
            if (texDatabase.has_value()) LOG_ERR_AND_RETURN(JPCError::DUPLICATE_CHUNK);
            LOG_AND_RETURN_IF_ERR(texDatabase.emplace().read(jpc, num_textures));
        }
        else {
            LOG_ERR_AND_RETURN(JPCError::UNKNOWN_CHUNK);
        }
    }

    if (!emitter.has_value()) LOG_ERR_AND_RETURN(JPCError::MISSING_CHUNK);
    if (!baseShape.has_value()) LOG_ERR_AND_RETURN(JPCError::MISSING_CHUNK);
    if (!texDatabase.has_value()) LOG_ERR_AND_RETURN(JPCError::MISSING_CHUNK);
    if (fields.size() > num_fld1_chunks) LOG_ERR_AND_RETURN(JPCError::DUPLICATE_CHUNK);
    if (fields.size() < num_fld1_chunks) LOG_ERR_AND_RETURN(JPCError::MISSING_CHUNK);
    if (curves.size() > num_kfa1_chunks) LOG_ERR_AND_RETURN(JPCError::DUPLICATE_CHUNK);
    if (curves.size() < num_kfa1_chunks) LOG_ERR_AND_RETURN(JPCError::MISSING_CHUNK);

	return JPCError::NONE;
}

JPCError Particle::save_changes(std::ostream& out, const std::unordered_map<std::string, size_t>& textures) {
	out.write(magicJEFF, 8);

	num_chunks = 3;
	if (!emitter.has_value()) LOG_ERR_AND_RETURN(JPCError::MISSING_CHUNK);
	if (!baseShape.has_value()) LOG_ERR_AND_RETURN(JPCError::MISSING_CHUNK);
	if (extraShape.has_value()) num_chunks += 1;
	if (extraTex.has_value()) num_chunks += 1;
	if (childShape.has_value()) num_chunks += 1;
	num_chunks += fields.size();
	num_chunks += curves.size();
	if (!texDatabase.has_value()) LOG_ERR_AND_RETURN(JPCError::MISSING_CHUNK);

	num_kfa1_chunks = curves.size();
	num_fld1_chunks = fields.size();

	num_textures = texDatabase.value().texFilenames.size();

	Utility::Endian::toPlatform_inplace(eType::Big, unknown_1);
	Utility::Endian::toPlatform_inplace(eType::Big, num_chunks);
	Utility::Endian::toPlatform_inplace(eType::Big, size); //not always accurate, shouldnt need to update?
	Utility::Endian::toPlatform_inplace(eType::Big, particle_id);

	out.write(reinterpret_cast<const char*>(&unknown_1), sizeof(unknown_1));
	out.write(reinterpret_cast<const char*>(&num_chunks), sizeof(num_chunks));
	out.write(reinterpret_cast<const char*>(&size), sizeof(size));
	out.write(reinterpret_cast<const char*>(&num_kfa1_chunks), sizeof(num_kfa1_chunks));
	out.write(reinterpret_cast<const char*>(&num_fld1_chunks), sizeof(num_fld1_chunks));
	out.write(reinterpret_cast<const char*>(&num_textures), sizeof(num_textures));
	out.write(reinterpret_cast<const char*>(&unknown_5), sizeof(unknown_5));
	out.write(reinterpret_cast<const char*>(&particle_id), sizeof(particle_id));
	out.write(reinterpret_cast<const char*>(&unknown_6), sizeof(unknown_6));

	//bem -> fld -> kfa -> bsp -> esp -> ssp -> etx -> tdb
	if (emitter.has_value()) {
		LOG_AND_RETURN_IF_ERR(emitter.value().save_changes(out));
		padToLen(out, 0x20);
	}

	for (JParticle::FLD1& field : fields) {
		LOG_AND_RETURN_IF_ERR(field.save_changes(out));
		padToLen(out, 0x20);
	}

	for (JParticle::KFA1& curve : curves) {
		LOG_AND_RETURN_IF_ERR(curve.save_changes(out));
		padToLen(out, 0x20);
	}

	if (baseShape.has_value()) {
		LOG_AND_RETURN_IF_ERR(baseShape.value().save_changes(out));
		padToLen(out, 0x20);
	}
	if (extraShape.has_value()) {
		LOG_AND_RETURN_IF_ERR(extraShape.value().save_changes(out));
		padToLen(out, 0x20);
	}
	if (childShape.has_value()) {
		LOG_AND_RETURN_IF_ERR(childShape.value().save_changes(out));
		padToLen(out, 0x20);
	}
	if (extraTex.has_value()) {
		LOG_AND_RETURN_IF_ERR(extraTex.value().save_changes(out));
		padToLen(out, 0x20);
	}
	if (texDatabase.has_value()) {
		LOG_AND_RETURN_IF_ERR(texDatabase.value().save_changes(out, textures));
		padToLen(out, 0x20);
	}

	return JPCError::NONE;
}

namespace FileTypes {

	const char* JPCErrorGetName(JPCError err) {
		switch (err) {
		case JPCError::NONE:
			return "NONE";
		case JPCError::REACHED_EOF:
			return "REACHED_EOF";
		case JPCError::COULD_NOT_OPEN:
			return "COULD_NOT_OPEN";
		case JPCError::NOT_JPC:
			return "NOT_JPC";
		case JPCError::UNKNOWN_CHUNK:
			return "UNKNOWN_CHUNK";
		case JPCError::DUPLICATE_CHUNK:
			return "DUPLICATE_CHUNK";
		case JPCError::MISSING_CHUNK:
			return "MISSING_CHUNK";
		case JPCError::UNEXPECTED_VALUE:
			return "UNEXPECTED_VALUE";
		case JPCError::DUPLICATE_PARTICLE_ID:
			return "DUPLICATE_PARTICLE_ID";
		case JPCError::DUPLICATE_FILENAME:
			return "DUPLICATE_FILENAME";
		case JPCError::MISSING_PARTICLE:
			return "MISSING_PARTICLE";
		case JPCError::MISSING_TEXTURE:
			return "MISSING_TEXTURE";
		default:
			return "UNKNOWN";
		}
	}

	void JPC::initNew() {
		memcpy(magicJPAC, "JPAC1-00", 8);
		num_particles = 0;
		num_textures = 0;

		particle_index_by_id = {};
		particles = {};

		textures = {};
	}

	JPC JPC::createNew() {
		JPC newJPC{};
		newJPC.initNew();
		return newJPC;
	}

	JPCError JPC::loadFromBinary(std::istream& jpc) {
		if (!jpc.read(magicJPAC, 8)) {
			LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
		}
		if (std::strncmp(magicJPAC, "JPAC1-00", 8) != 0) {
			LOG_ERR_AND_RETURN(JPCError::NOT_JPC);
		}
		if (!jpc.read(reinterpret_cast<char*>(&num_particles), sizeof(num_particles))) {
			LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
		}
		if (!jpc.read(reinterpret_cast<char*>(&num_textures), sizeof(num_textures))) {
			LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
		}
		Utility::Endian::toPlatform_inplace(eType::Big, num_particles);
		Utility::Endian::toPlatform_inplace(eType::Big, num_textures);

		jpc.seekg(0x20, std::ios::beg);

		for (uint16_t i = 0; i < num_particles; i++) {
			Particle& particle = particles.emplace_back();
			LOG_AND_RETURN_IF_ERR(particle.read(jpc));

			particle_index_by_id[particle.particle_id] = i;

			LOG_AND_RETURN_IF_ERR(readPadding<JPCError>(jpc, 0x20));
		}

		std::vector<std::string> texFilenames;
		for (uint16_t i = 0; i < num_textures; i++) {
			char magic[4];
			if (!jpc.read(magic, 4)) {
				LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
			}
			if (std::strncmp(magic, "TEX1", 4) != 0) {
				LOG_ERR_AND_RETURN(JPCError::UNKNOWN_CHUNK);
			}

			jpc.seekg(8, std::ios::cur); //ignore section size, its not accurate anyway, also skip 4 unused bytes
			std::string filename(0x14, '\0');
			if (!jpc.read(&filename[0], 0x14)) {
				LOG_ERR_AND_RETURN(JPCError::REACHED_EOF);
			}

			texFilenames.push_back(filename);
			textures[filename] = i;
		}

		for (Particle& particle : particles) {
			if(!particle.texDatabase.has_value()) LOG_ERR_AND_RETURN(JPCError::MISSING_CHUNK);

			LOG_AND_RETURN_IF_ERR(particle.texDatabase.value().populateFilenames(texFilenames));
		}

		return JPCError::NONE;
	}

	JPCError JPC::loadFromFile(const std::string& filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERR_AND_RETURN(JPCError::COULD_NOT_OPEN);
		}
		return loadFromBinary(file);
	}

	JPCError JPC::addParticle(const Particle& particle) {
		if (particle_index_by_id.contains(particle.particle_id)) LOG_ERR_AND_RETURN(JPCError::DUPLICATE_PARTICLE_ID);

		particle_index_by_id[particle.particle_id] = particles.size();
		particles.push_back(particle);

		return JPCError::NONE;
	}

	JPCError JPC::replaceParticle(const Particle& particle) {
		if (!particle_index_by_id.contains(particle.particle_id)) LOG_ERR_AND_RETURN(JPCError::MISSING_PARTICLE);

		particles[particle_index_by_id[particle.particle_id]] = particle;

		return JPCError::NONE;
	}

	JPCError JPC::addTexture(const std::string& filename) {
		std::string filenamePad = filename;
		filenamePad.resize(0x14, '\0');
		if (textures.contains(filenamePad)) LOG_ERR_AND_RETURN(JPCError::DUPLICATE_FILENAME);

		const size_t nextIndex = textures.size();
		textures[filenamePad] = nextIndex;
		return JPCError::NONE;
	}

	JPCError JPC::writeToStream(std::ostream& out) {
		out.write(magicJPAC, 8);

		num_particles = particles.size();
		num_textures = textures.size();

		Utility::Endian::toPlatform_inplace(eType::Big, num_particles);
		Utility::Endian::toPlatform_inplace(eType::Big, num_textures);

		out.write(reinterpret_cast<const char*>(&num_particles), sizeof(num_particles));
		out.write(reinterpret_cast<const char*>(&num_textures), sizeof(num_textures));

		Utility::seek(out, 0x20, std::ios::beg);

		for (Particle& particle : particles) {
			LOG_AND_RETURN_IF_ERR(particle.save_changes(out, textures));
			padToLen(out, 0x20);
		}

		std::streamoff texStart = out.tellp(); //some stuff to preserve original order
		for (const auto& [filename, index] : textures) {
			Utility::seek(out, texStart + (index * 0x20), std::ios::beg); //some stuff to preserve original order
			out.write("TEX1", 4);

			out.write("\x00\x00\x00\x20\x00\x00\x00\x00", 8);
			out.write(&filename[0], 0x14);
		}

		return JPCError::NONE;
	}

	JPCError JPC::writeToFile(const std::string& outFilePath) {
		std::ofstream outFile(outFilePath, std::ios::binary);
		if (!outFile.is_open()) {
			LOG_ERR_AND_RETURN(JPCError::COULD_NOT_OPEN);
		}
		return writeToStream(outFile);
	}
}
