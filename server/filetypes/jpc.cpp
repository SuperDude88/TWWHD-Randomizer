#include "jpc.hpp"

#include <cstring>
#include <algorithm>

#include "../utility/byteswap.hpp"



std::vector<JParticle::ColorAnimationKeyframe> readColorTable(std::istream& in, unsigned int offset, uint8_t dataCount) {
	in.seekg(offset, std::ios::beg);

	std::vector<JParticle::ColorAnimationKeyframe> table;
	table.reserve(dataCount);
	for (uint8_t i = 0; i < dataCount; i++) {
		JParticle::ColorAnimationKeyframe& keyframe = table.emplace_back();

		if (!in.read(reinterpret_cast<char*>(&keyframe.time), sizeof(keyframe.time))) {
			table.clear(); //return empty vector to signal error
			return table;
		};
		Utility::byteswap_inplace(keyframe.time);

		if (!readRGBA8(in, in.tellg(), keyframe.color)) {
			table.clear(); //return empty vector to signal error
			return table;
		};
	}

	return table;
}

void writeColorTable(std::ostream& out, const std::vector<JParticle::ColorAnimationKeyframe>& table) {
	for (const JParticle::ColorAnimationKeyframe& keyframe : table) {
		uint16_t time = Utility::byteswap(keyframe.time);

		out.write(reinterpret_cast<char*>(&time), sizeof(time));
		writeRGBA8(out, keyframe.color);
	}

	return;
}



namespace JParticle {
	namespace chunks {
		JPCError BEM1::read(std::istream& jpc, const unsigned int offset) {
			this->offset = offset;
			jpc.seekg(offset, std::ios::beg);
			
			if (!jpc.read(magic, 4)) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) return JPCError::REACHED_EOF;
			jpc.seekg(4, std::ios::cur);

			if (std::strncmp(magic, "BEM1", 4) != 0) return JPCError::UNKNOWN_CHUNK;

			Utility::byteswap_inplace(sectionSize);

			if (!jpc.read(reinterpret_cast<char*>(&flags), sizeof(flags))) return JPCError::REACHED_EOF;
			Utility::byteswap_inplace(flags);

			emitFlags = static_cast<EmitFlags>(flags & 0xFF);
			volumeType = static_cast<VolumeType>((flags >> 8) & 0x07);

			if (!jpc.read(reinterpret_cast<char*>(&volumeSweep), sizeof(volumeSweep))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&volumeMinRad), sizeof(volumeMinRad))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&volumeSize), sizeof(volumeSize))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&divNumber), sizeof(divNumber))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&rate), sizeof(rate))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&rateRndm), sizeof(rateRndm))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&rateStep), sizeof(rateStep))) return JPCError::REACHED_EOF;
			jpc.seekg(1, std::ios::cur);

			if (!jpc.read(reinterpret_cast<char*>(&maxFrame), sizeof(maxFrame))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&startFrame), sizeof(startFrame))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&lifeTime), sizeof(lifeTime))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&lifeTimeRndm), sizeof(lifeTimeRndm))) return JPCError::REACHED_EOF;

			if (!jpc.read(reinterpret_cast<char*>(&initialVelOmni), sizeof(initialVelOmni))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&initialVelAxis), sizeof(initialVelAxis))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&initialVelRndm), sizeof(initialVelRndm))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&initialVelDir), sizeof(initialVelDir))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&initialVelRatio), sizeof(initialVelRatio))) return JPCError::REACHED_EOF;

			if (!jpc.read(reinterpret_cast<char*>(&spread), sizeof(spread))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&airResist), sizeof(airResist))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&airResistRndm), sizeof(airResistRndm))) return JPCError::REACHED_EOF;

			if (!jpc.read(reinterpret_cast<char*>(&moment), sizeof(moment))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&momentRndm), sizeof(momentRndm))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&accel), sizeof(accel))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&accelRndm), sizeof(accelRndm))) return JPCError::REACHED_EOF;

			Utility::byteswap_inplace(volumeSweep);
			Utility::byteswap_inplace(volumeMinRad);
			Utility::byteswap_inplace(volumeSize);
			Utility::byteswap_inplace(divNumber);
			Utility::byteswap_inplace(rate);
			Utility::byteswap_inplace(rateRndm);
			Utility::byteswap_inplace(maxFrame);
			Utility::byteswap_inplace(startFrame);
			Utility::byteswap_inplace(lifeTime);
			Utility::byteswap_inplace(lifeTime);
			Utility::byteswap_inplace(initialVelOmni);
			Utility::byteswap_inplace(initialVelAxis);
			Utility::byteswap_inplace(initialVelRndm);
			Utility::byteswap_inplace(initialVelDir);
			Utility::byteswap_inplace(initialVelRatio);
			Utility::byteswap_inplace(spread);
			Utility::byteswap_inplace(airResist);
			Utility::byteswap_inplace(airResistRndm);
			Utility::byteswap_inplace(moment);
			Utility::byteswap_inplace(momentRndm);
			Utility::byteswap_inplace(accel);
			Utility::byteswap_inplace(accelRndm);

			if (!readVec3(jpc, jpc.tellg(), emitterScale)) return JPCError::REACHED_EOF;
			if (!readVec3(jpc, jpc.tellg(), emitterTrans)) return JPCError::REACHED_EOF;
			if (!readVec3(jpc, jpc.tellg(), emitterDir)) return JPCError::REACHED_EOF;
			if (!readVec3(jpc, jpc.tellg(), emitterRot)) return JPCError::REACHED_EOF;

			return JPCError::NONE;
		}

		JPCError BEM1::save_changes(std::ostream& out) {
			this->offset = out.tellp();

			Utility::byteswap_inplace(sectionSize);

			out.write(magic, 4);
			out.write(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldn't need to update?
			out.seekp(4, std::ios::cur);

			flags = (flags & 0xFFFFFF00) | (static_cast<uint8_t>(emitFlags) & 0x000000FF);
			flags = (flags & 0xFFFF00FF) | ((static_cast<uint8_t>(volumeType) & 0x000000FF) << 8);

			Utility::byteswap_inplace(flags);
			out.write(reinterpret_cast<char*>(&flags), sizeof(flags));

			Utility::byteswap_inplace(volumeSweep);
			Utility::byteswap_inplace(volumeMinRad);
			Utility::byteswap_inplace(volumeSize);
			Utility::byteswap_inplace(divNumber);
			Utility::byteswap_inplace(rate);
			Utility::byteswap_inplace(rateRndm);
			Utility::byteswap_inplace(maxFrame);
			Utility::byteswap_inplace(startFrame);
			Utility::byteswap_inplace(lifeTime);
			Utility::byteswap_inplace(lifeTime);
			Utility::byteswap_inplace(initialVelOmni);
			Utility::byteswap_inplace(initialVelAxis);
			Utility::byteswap_inplace(initialVelRndm);
			Utility::byteswap_inplace(initialVelDir);
			Utility::byteswap_inplace(initialVelRatio);
			Utility::byteswap_inplace(spread);
			Utility::byteswap_inplace(airResist);
			Utility::byteswap_inplace(airResistRndm);
			Utility::byteswap_inplace(moment);
			Utility::byteswap_inplace(momentRndm);
			Utility::byteswap_inplace(accel);
			Utility::byteswap_inplace(accelRndm);

			out.write(reinterpret_cast<char*>(&volumeSweep), sizeof(volumeSweep));
			out.write(reinterpret_cast<char*>(&volumeMinRad), sizeof(volumeMinRad));
			out.write(reinterpret_cast<char*>(&volumeSize), sizeof(volumeSize));
			out.write(reinterpret_cast<char*>(&divNumber), sizeof(divNumber));
			out.write(reinterpret_cast<char*>(&rate), sizeof(rate));
			out.write(reinterpret_cast<char*>(&rateRndm), sizeof(rateRndm));
			out.write(reinterpret_cast<char*>(&rateStep), sizeof(rateStep));
			out.seekp(1, std::ios::cur); //what is this?

			out.write(reinterpret_cast<char*>(&maxFrame), sizeof(maxFrame));
			out.write(reinterpret_cast<char*>(&startFrame), sizeof(startFrame));
			out.write(reinterpret_cast<char*>(&lifeTime), sizeof(lifeTime));
			out.write(reinterpret_cast<char*>(&lifeTimeRndm), sizeof(lifeTimeRndm));

			out.write(reinterpret_cast<char*>(&initialVelOmni), sizeof(initialVelOmni));
			out.write(reinterpret_cast<char*>(&initialVelAxis), sizeof(initialVelAxis));
			out.write(reinterpret_cast<char*>(&initialVelRndm), sizeof(initialVelRndm));
			out.write(reinterpret_cast<char*>(&initialVelDir), sizeof(initialVelDir));
			out.write(reinterpret_cast<char*>(&initialVelRatio), sizeof(initialVelRatio));

			out.write(reinterpret_cast<char*>(&spread), sizeof(spread));
			out.write(reinterpret_cast<char*>(&airResist), sizeof(airResist));
			out.write(reinterpret_cast<char*>(&airResistRndm), sizeof(airResistRndm));

			out.write(reinterpret_cast<char*>(&moment), sizeof(moment));
			out.write(reinterpret_cast<char*>(&momentRndm), sizeof(momentRndm));
			out.write(reinterpret_cast<char*>(&accel), sizeof(accel));
			out.write(reinterpret_cast<char*>(&accelRndm), sizeof(accelRndm));

			writeVec3(out, emitterScale);
			writeVec3(out, emitterTrans);
			writeVec3(out, emitterDir);
			writeVec3(out, emitterRot);

			return JPCError::NONE;
		}
		

		JPCError BSP1::read(std::istream& jpc, const unsigned int offset) {
			this->offset = offset;
			jpc.seekg(offset, std::ios::beg);

			if (!jpc.read(magic, 4)) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) return JPCError::REACHED_EOF;
			jpc.seekg(4, std::ios::cur);

			if (std::strncmp(magic, "BSP1", 4) != 0) return JPCError::UNKNOWN_CHUNK;

			Utility::byteswap_inplace(sectionSize);

			if (!jpc.read(reinterpret_cast<char*>(&flags), sizeof(flags))) return JPCError::REACHED_EOF;
			Utility::byteswap_inplace(flags);

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
			if (!jpc.read(reinterpret_cast<char*>(&colorPrmAnimDataOff), sizeof(colorPrmAnimDataOff))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&colorEnvAnimDataOff), sizeof(colorEnvAnimDataOff))) return JPCError::REACHED_EOF;
			Utility::byteswap_inplace(colorPrmAnimDataOff);
			Utility::byteswap_inplace(colorEnvAnimDataOff);

			if (!readVec2(jpc, jpc.tellg(), baseSize)) return JPCError::REACHED_EOF;
			
			if (!jpc.read(reinterpret_cast<char*>(&anmRndm), sizeof(anmRndm))) return JPCError::REACHED_EOF;

			Utility::byteswap_inplace(anmRndm);

			colorLoopOfstMask = -((flags >> 0x0B) & 0x01);
			texIdxLoopOfstMask = -((flags >> 0x0D) & 0x01);

			uint8_t texIdxAnimCount;
			uint8_t colorPrmAnimDataCount;
			uint8_t colorEnvAnimDataCount;
			if (!jpc.read(reinterpret_cast<char*>(&blendModeFlags), sizeof(blendModeFlags))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&alphaCompareFlags), sizeof(alphaCompareFlags))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&alphaRef0), sizeof(alphaRef0))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&alphaRef1), sizeof(alphaRef1))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&zModeFlags), sizeof(zModeFlags))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&texFlags), sizeof(texFlags))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&texIdxAnimCount), sizeof(texIdxAnimCount))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&texIdx), sizeof(texIdx))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&colorFlags), sizeof(colorFlags))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&colorPrmAnimDataCount), sizeof(colorPrmAnimDataCount))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&colorEnvAnimDataCount), sizeof(colorEnvAnimDataCount))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&colorAnimMaxFrm), sizeof(colorAnimMaxFrm))) return JPCError::REACHED_EOF;

			Utility::byteswap_inplace(blendModeFlags);
			Utility::byteswap_inplace(colorAnimMaxFrm);

			if (!readRGBA8(jpc, jpc.tellg(), colorPrm)) return JPCError::REACHED_EOF;
			if (!readRGBA8(jpc, jpc.tellg(), colorEnv)) return JPCError::REACHED_EOF;

			colorCalcIdxType = static_cast<CalcIdxType>((colorFlags >> 4) & 0x07);

			if (!jpc.read(reinterpret_cast<char*>(&tilingS), sizeof(tilingS))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&tilingT), sizeof(tilingT))) return JPCError::REACHED_EOF;

			Utility::byteswap_inplace(tilingS);
			Utility::byteswap_inplace(tilingT);

			texCalcIdxType = static_cast<CalcIdxType>((texFlags >> 2) & 0x07);

			if (!readVec2(jpc, jpc.tellg(), texInitTrans)) return JPCError::REACHED_EOF;
			if (!readVec2(jpc, jpc.tellg(), texInitScale)) return JPCError::REACHED_EOF;
			if (!readVec2(jpc, jpc.tellg(), texIncTrans)) return JPCError::REACHED_EOF;
			if (!readVec2(jpc, jpc.tellg(), texIncScale)) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&texIncRot), sizeof(texIncRot))) return JPCError::REACHED_EOF;

			Utility::byteswap_inplace(texIncRot);

			if ((texFlags & 0x00000001) != 0) {
				for (uint8_t i = 0; i < texIdxAnimCount; i++) {
					uint8_t& val = texIdxAnimData.emplace_back();
					if (!jpc.read(reinterpret_cast<char*>(&val), sizeof(val))) return JPCError::REACHED_EOF;
				}
			}

			isEnableTexture = (texFlags & 0x00000002) != 0;

			if ((colorFlags & 0x02) != 0) {
				if (colorPrmAnimData = readColorTable(jpc, this->offset + colorPrmAnimDataOff, colorPrmAnimDataCount); colorPrmAnimData.empty()) return JPCError::REACHED_EOF;
			}

			if ((colorFlags & 0x08) != 0) {
				if (colorEnvAnimData = readColorTable(jpc, this->offset + colorEnvAnimDataOff, colorEnvAnimDataCount); colorEnvAnimData.empty()) return JPCError::REACHED_EOF;
			}

			return JPCError::NONE;
		}

		JPCError BSP1::save_changes(std::ostream& out) {
			this->offset = out.tellp();

			Utility::byteswap_inplace(sectionSize);

			out.write(magic, 4);
			out.write(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
			out.seekp(4, std::ios::cur);

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

			Utility::byteswap_inplace(flags);

			out.write(reinterpret_cast<char*>(&flags), sizeof(flags));

			uint16_t colorPrmAnimDataOff = 0;
			uint16_t colorEnvAnimDataOff = 0;

			colorPrmAnimDataOff = 0x60 + texIdxAnimData.size();
			unsigned int numPaddingBytes = 4 - (colorPrmAnimDataOff % 4);
			if (numPaddingBytes == 4) {
				numPaddingBytes = 0;
			}
			colorPrmAnimDataOff += numPaddingBytes;

			if (colorEnvAnimData.size() > 0) colorEnvAnimDataOff = colorPrmAnimDataOff + (colorPrmAnimData.size() * 0x6);
			numPaddingBytes = 4 - (colorEnvAnimDataOff % 4);
			if (numPaddingBytes == 4) {
				numPaddingBytes = 0;
			}
			colorEnvAnimDataOff += numPaddingBytes;

			if (colorPrmAnimData.size() == 0) colorPrmAnimDataOff = 0;
			if (colorEnvAnimData.size() == 0) colorEnvAnimDataOff = 0;

			Utility::byteswap_inplace(colorPrmAnimDataOff);
			Utility::byteswap_inplace(colorEnvAnimDataOff);

			out.write(reinterpret_cast<char*>(&colorPrmAnimDataOff), sizeof(colorPrmAnimDataOff));
			out.write(reinterpret_cast<char*>(&colorEnvAnimDataOff), sizeof(colorEnvAnimDataOff));

			writeVec2(out, baseSize);

			Utility::byteswap_inplace(anmRndm);

			out.write(reinterpret_cast<char*>(&anmRndm), sizeof(anmRndm));


			uint8_t texIdxAnimCount = texIdxAnimData.size();
			uint8_t colorPrmAnimDataCount = colorPrmAnimData.size();
			uint8_t colorEnvAnimDataCount = colorEnvAnimData.size();

			Utility::byteswap_inplace(blendModeFlags);
			Utility::byteswap_inplace(colorAnimMaxFrm);

			colorFlags = (colorFlags & ~0x70) | ((static_cast<uint8_t>(colorCalcIdxType) & 0x07) << 4);
			colorFlags = (colorFlags & ~0x02) | ((0x0 << 1) & 0x2);
			if (colorPrmAnimData.size() > 0) colorFlags = (colorFlags & ~0x02) | ((0x1 << 1) & 0x2);
			colorFlags = (colorFlags & ~0x08) | ((0x0 << 3) & 0x8);
			if (colorEnvAnimData.size() > 0) colorFlags = (colorFlags & ~0x08) | ((0x1 << 3) & 0x8);

			texFlags = (texFlags & ~0x28) | ((static_cast<uint8_t>(texCalcIdxType) & 0x07) << 2);
			texFlags = (texFlags & ~0x01) | (0x0 & 0x1);
			if (texIdxAnimData.size() > 0) texFlags = (texFlags & ~0x01) | (0x1);

			texFlags = (texFlags & ~0x02) | ((isEnableTexture & 1) << 1);

			out.write(reinterpret_cast<char*>(&blendModeFlags), sizeof(blendModeFlags));
			out.write(reinterpret_cast<char*>(&alphaCompareFlags), sizeof(alphaCompareFlags));
			out.write(reinterpret_cast<char*>(&alphaRef0), sizeof(alphaRef0));
			out.write(reinterpret_cast<char*>(&alphaRef1), sizeof(alphaRef1));
			out.write(reinterpret_cast<char*>(&zModeFlags), sizeof(zModeFlags));
			out.write(reinterpret_cast<char*>(&texFlags), sizeof(texFlags));
			out.write(reinterpret_cast<char*>(&texIdxAnimCount), sizeof(texIdxAnimCount));
			out.write(reinterpret_cast<char*>(&texIdx), sizeof(texIdx));
			out.write(reinterpret_cast<char*>(&colorFlags), sizeof(colorFlags));
			out.write(reinterpret_cast<char*>(&colorPrmAnimDataCount), sizeof(colorPrmAnimDataCount));
			out.write(reinterpret_cast<char*>(&colorEnvAnimDataCount), sizeof(colorEnvAnimDataCount));
			out.write(reinterpret_cast<char*>(&colorAnimMaxFrm), sizeof(colorAnimMaxFrm));

			writeRGBA8(out, colorPrm);
			writeRGBA8(out, colorEnv);

			Utility::byteswap_inplace(tilingS);
			Utility::byteswap_inplace(tilingT);

			out.write(reinterpret_cast<char*>(&tilingS), sizeof(tilingS));
			out.write(reinterpret_cast<char*>(&tilingT), sizeof(tilingT));

			writeVec2(out, texInitTrans);
			writeVec2(out, texInitScale);
			writeVec2(out, texIncTrans);
			writeVec2(out, texIncScale);
			
			Utility::byteswap_inplace(texIncRot);

			out.write(reinterpret_cast<char*>(&texIncRot), sizeof(texIncRot));

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


		JPCError ESP1::read(std::istream& jpc, const unsigned int offset) {
			this->offset = offset;
			jpc.seekg(offset, std::ios::beg);

			if (!jpc.read(magic, 4)) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) return JPCError::REACHED_EOF;
			jpc.seekg(4, std::ios::cur);

			if (std::strncmp(magic, "ESP1", 4) != 0) return JPCError::UNKNOWN_CHUNK;

			Utility::byteswap_inplace(sectionSize);

			if (!jpc.read(reinterpret_cast<char*>(&flags), sizeof(flags))) return JPCError::REACHED_EOF;
			Utility::byteswap_inplace(flags);

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

			if (!jpc.read(reinterpret_cast<char*>(&alphaInTiming), sizeof(alphaInTiming))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&alphaOutTiming), sizeof(alphaOutTiming))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&alphaInValue), sizeof(alphaInValue))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&alphaBaseValue), sizeof(alphaBaseValue))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&alphaOutValue), sizeof(alphaOutValue))) return JPCError::REACHED_EOF;

			if (!jpc.read(reinterpret_cast<char*>(&alphaWaveParam1), sizeof(alphaWaveParam1))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&alphaWaveParam2), sizeof(alphaWaveParam2))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&alphaWaveParam3), sizeof(alphaWaveParam3))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&alphaWaveRandom), sizeof(alphaWaveRandom))) return JPCError::REACHED_EOF;

			if (!jpc.read(reinterpret_cast<char*>(&scaleInTiming), sizeof(scaleInTiming))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&scaleOutTiming), sizeof(scaleOutTiming))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&scaleInValueX), sizeof(scaleInValueX))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&scaleOutValueX), sizeof(scaleOutValueX))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&scaleInValueY), sizeof(scaleInValueY))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&scaleOutValueY), sizeof(scaleOutValueY))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&scaleOutRandom), sizeof(scaleOutRandom))) return JPCError::REACHED_EOF;
			if (!readVec2(jpc, jpc.tellg(), scaleAnmMaxFrame)) return JPCError::REACHED_EOF;

			if (!jpc.read(reinterpret_cast<char*>(&rotateAngle), sizeof(rotateAngle))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&rotateSpeed), sizeof(rotateSpeed))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&rotateAngleRandom), sizeof(rotateAngleRandom))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&rotateSpeedRandom), sizeof(rotateSpeedRandom))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&rotateDirection), sizeof(rotateDirection))) return JPCError::REACHED_EOF;

			Utility::byteswap_inplace(alphaInTiming);
			Utility::byteswap_inplace(alphaOutTiming);
			Utility::byteswap_inplace(alphaInValue);
			Utility::byteswap_inplace(alphaBaseValue);
			Utility::byteswap_inplace(alphaOutValue);

			Utility::byteswap_inplace(alphaWaveParam1);
			Utility::byteswap_inplace(alphaWaveParam2);
			Utility::byteswap_inplace(alphaWaveParam3);
			Utility::byteswap_inplace(alphaWaveRandom);

			Utility::byteswap_inplace(scaleInTiming);
			Utility::byteswap_inplace(scaleOutTiming);
			Utility::byteswap_inplace(scaleInValueX);
			Utility::byteswap_inplace(scaleOutValueX);
			Utility::byteswap_inplace(scaleInValueY);
			Utility::byteswap_inplace(scaleOutValueY);
			Utility::byteswap_inplace(scaleOutRandom);

			Utility::byteswap_inplace(rotateAngle);
			Utility::byteswap_inplace(rotateSpeed);
			Utility::byteswap_inplace(rotateAngleRandom);
			Utility::byteswap_inplace(rotateSpeedRandom);
			Utility::byteswap_inplace(rotateDirection);

			return JPCError::NONE;
		}
		
		JPCError ESP1::save_changes(std::ostream& out) {
			this->offset = out.tellp();

			Utility::byteswap_inplace(sectionSize);

			out.write(magic, 4);
			out.write(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
			out.seekp(4, std::ios::cur);

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

			Utility::byteswap_inplace(flags);

			out.write(reinterpret_cast<char*>(&flags), sizeof(flags));

			out.seekp(4, std::ios::cur);

			Utility::byteswap_inplace(alphaInTiming);
			Utility::byteswap_inplace(alphaOutTiming);
			Utility::byteswap_inplace(alphaInValue);
			Utility::byteswap_inplace(alphaBaseValue);
			Utility::byteswap_inplace(alphaOutValue);

			Utility::byteswap_inplace(alphaWaveParam1);
			Utility::byteswap_inplace(alphaWaveParam2);
			Utility::byteswap_inplace(alphaWaveParam3);
			Utility::byteswap_inplace(alphaWaveRandom);

			Utility::byteswap_inplace(scaleInTiming);
			Utility::byteswap_inplace(scaleOutTiming);
			Utility::byteswap_inplace(scaleInValueX);
			Utility::byteswap_inplace(scaleOutValueX);
			Utility::byteswap_inplace(scaleInValueY);
			Utility::byteswap_inplace(scaleOutValueY);
			Utility::byteswap_inplace(scaleOutRandom);

			Utility::byteswap_inplace(rotateAngle);
			Utility::byteswap_inplace(rotateSpeed);
			Utility::byteswap_inplace(rotateAngleRandom);
			Utility::byteswap_inplace(rotateSpeedRandom);
			Utility::byteswap_inplace(rotateDirection);

			out.write(reinterpret_cast<char*>(&alphaInTiming), sizeof(alphaInTiming));
			out.write(reinterpret_cast<char*>(&alphaOutTiming), sizeof(alphaOutTiming));
			out.write(reinterpret_cast<char*>(&alphaInValue), sizeof(alphaInValue));
			out.write(reinterpret_cast<char*>(&alphaBaseValue), sizeof(alphaBaseValue));
			out.write(reinterpret_cast<char*>(&alphaOutValue), sizeof(alphaOutValue));

			out.write(reinterpret_cast<char*>(&alphaWaveParam1), sizeof(alphaWaveParam1));
			out.write(reinterpret_cast<char*>(&alphaWaveParam2), sizeof(alphaWaveParam2));
			out.write(reinterpret_cast<char*>(&alphaWaveParam3), sizeof(alphaWaveParam3));
			out.write(reinterpret_cast<char*>(&alphaWaveRandom), sizeof(alphaWaveRandom));

			out.write(reinterpret_cast<char*>(&scaleInTiming), sizeof(scaleInTiming));
			out.write(reinterpret_cast<char*>(&scaleOutTiming), sizeof(scaleOutTiming));
			out.write(reinterpret_cast<char*>(&scaleInValueX), sizeof(scaleInValueX));
			out.write(reinterpret_cast<char*>(&scaleOutValueX), sizeof(scaleOutValueX));
			out.write(reinterpret_cast<char*>(&scaleInValueY), sizeof(scaleInValueY));
			out.write(reinterpret_cast<char*>(&scaleOutValueY), sizeof(scaleOutValueY));
			out.write(reinterpret_cast<char*>(&scaleOutRandom), sizeof(scaleOutRandom));
			writeVec2(out, scaleAnmMaxFrame);

			out.write(reinterpret_cast<char*>(&rotateAngle), sizeof(rotateAngle));
			out.write(reinterpret_cast<char*>(&rotateSpeed), sizeof(rotateSpeed));
			out.write(reinterpret_cast<char*>(&rotateAngleRandom), sizeof(rotateAngleRandom));
			out.write(reinterpret_cast<char*>(&rotateSpeedRandom), sizeof(rotateSpeedRandom));
			out.write(reinterpret_cast<char*>(&rotateDirection), sizeof(rotateDirection));

			return JPCError::NONE;
		}


		JPCError ETX1::read(std::istream& jpc, const unsigned int offset) {
			this->offset = offset;
			jpc.seekg(offset, std::ios::beg);

			if (!jpc.read(magic, 4)) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) return JPCError::REACHED_EOF;
			jpc.seekg(4, std::ios::cur);

			if (std::strncmp(magic, "ETX1", 4) != 0) return JPCError::UNKNOWN_CHUNK;

			Utility::byteswap_inplace(sectionSize);

			if (!jpc.read(reinterpret_cast<char*>(&flags), sizeof(flags))) return JPCError::REACHED_EOF;
			Utility::byteswap_inplace(flags);

			if (!jpc.read(reinterpret_cast<char*>(&p00), sizeof(p00))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&p01), sizeof(p01))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&p02), sizeof(p02))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&p10), sizeof(p10))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&p11), sizeof(p11))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&p12), sizeof(p12))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&scale), sizeof(scale))) return JPCError::REACHED_EOF;

			Utility::byteswap_inplace(p00);
			Utility::byteswap_inplace(p01);
			Utility::byteswap_inplace(p02);
			Utility::byteswap_inplace(p10);
			Utility::byteswap_inplace(p11);
			Utility::byteswap_inplace(p12);
			Utility::byteswap_inplace(scale);

			indTextureMode = static_cast<IndTextureMode>(flags & 0x03);
			if (!jpc.read(reinterpret_cast<char*>(&indTextureID), sizeof(indTextureID))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&subTextureID), sizeof(subTextureID))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&secondTextureIndex), sizeof(secondTextureIndex))) return JPCError::REACHED_EOF;
			
			useSecondTex = (flags & 0x00000100) != 0;

			return JPCError::NONE;
		}

		JPCError ETX1::save_changes(std::ostream& out) {
			this->offset = out.tellp();

			Utility::byteswap_inplace(sectionSize);

			out.write(magic, 4);
			out.write(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
			out.seekp(4, std::ios::cur);

			flags = (flags & ~0x00000003) | (static_cast<uint8_t>(indTextureMode) & 0x00000003);
			flags = (flags & ~0x00000100) | ((useSecondTex & 0x1) << 8);

			Utility::byteswap_inplace(flags);

			out.write(reinterpret_cast<char*>(&flags), sizeof(flags));

			Utility::byteswap_inplace(p00);
			Utility::byteswap_inplace(p01);
			Utility::byteswap_inplace(p02);
			Utility::byteswap_inplace(p10);
			Utility::byteswap_inplace(p11);
			Utility::byteswap_inplace(p12);
			Utility::byteswap_inplace(scale);

			out.write(reinterpret_cast<char*>(&p00), sizeof(p00));
			out.write(reinterpret_cast<char*>(&p01), sizeof(p01));
			out.write(reinterpret_cast<char*>(&p02), sizeof(p02));
			out.write(reinterpret_cast<char*>(&p10), sizeof(p10));
			out.write(reinterpret_cast<char*>(&p11), sizeof(p11));
			out.write(reinterpret_cast<char*>(&p12), sizeof(p12));
			out.write(reinterpret_cast<char*>(&scale), sizeof(scale));

			out.write(reinterpret_cast<char*>(&indTextureID), sizeof(indTextureID));
			out.write(reinterpret_cast<char*>(&subTextureID), sizeof(subTextureID));
			
			out.write(reinterpret_cast<char*>(&secondTextureIndex), sizeof(secondTextureIndex));

			return JPCError::NONE;
		}


		JPCError SSP1::read(std::istream& jpc, const unsigned int offset) {
			this->offset = offset;
			jpc.seekg(offset, std::ios::beg);

			if (!jpc.read(magic, 4)) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) return JPCError::REACHED_EOF;
			jpc.seekg(4, std::ios::cur);

			if (std::strncmp(magic, "SSP1", 4) != 0) return JPCError::UNKNOWN_CHUNK;

			Utility::byteswap_inplace(sectionSize);

			if (!jpc.read(reinterpret_cast<char*>(&flags), sizeof(flags))) return JPCError::REACHED_EOF;
			Utility::byteswap_inplace(flags);

			shapeType = static_cast<ShapeType>(flags & 0xF);
			dirType = static_cast<DirType>((flags >> 0x4) & 0x7);
			rotType = static_cast<RotType>((flags >> 0x7) & 0x7);
			planeType = static_cast<PlaneType>((flags >> 0xA) & 0x1);
			if (shapeType == ShapeType::DIRECTION_CROSS || shapeType == ShapeType::ROTATION_CROSS) {
				planeType = PlaneType::X;
			}

			isDrawParent = (flags & 0x00080000) != 0;

			if (!jpc.read(reinterpret_cast<char*>(&posRndm), sizeof(posRndm))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&baseVel), sizeof(baseVel))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&baseVelRndm), sizeof(baseVelRndm))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&velInfRate), sizeof(velInfRate))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&gravity), sizeof(gravity))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&timing), sizeof(timing))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&life), sizeof(life))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&rate), sizeof(rate))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&step), sizeof(step))) return JPCError::REACHED_EOF;

			if (!readVec2(jpc, jpc.tellg(), globalScale2D)) return JPCError::REACHED_EOF;

			if (!jpc.read(reinterpret_cast<char*>(&rotateSpeed), sizeof(rotateSpeed))) return JPCError::REACHED_EOF;

			isEnableRotate = (flags & 0x01000000) != 0;
			isEnableAlphaOut = (flags & 0x00800000) != 0;
			isEnableScaleOut = (flags & 0x00400000) != 0;
			isEnableField = (flags & 0x00200000) != 0;
			isInheritedRGB = (flags & 0x00040000) != 0;
			isInheritedAlpha = (flags & 0x00020000) != 0;
			isInheritedScale = (flags & 0x00010000) != 0;

			if (!jpc.read(reinterpret_cast<char*>(&inheritScale), sizeof(inheritScale))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&inheritAlpha), sizeof(inheritAlpha))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&inheritRGB), sizeof(inheritRGB))) return JPCError::REACHED_EOF;
			if (!readRGBA8(jpc, jpc.tellg(), colorPrm)) return JPCError::REACHED_EOF;
			if (!readRGBA8(jpc, jpc.tellg(), colorEnv)) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&texIdx), sizeof(texIdx))) return JPCError::REACHED_EOF;

			Utility::byteswap_inplace(posRndm);
			Utility::byteswap_inplace(baseVel);
			Utility::byteswap_inplace(baseVelRndm);
			Utility::byteswap_inplace(velInfRate);
			Utility::byteswap_inplace(gravity);
			Utility::byteswap_inplace(timing);
			Utility::byteswap_inplace(life);
			Utility::byteswap_inplace(rate);
			Utility::byteswap_inplace(step);
			Utility::byteswap_inplace(rotateSpeed);
			Utility::byteswap_inplace(inheritRGB);
			Utility::byteswap_inplace(inheritAlpha);
			Utility::byteswap_inplace(inheritRGB);

			return JPCError::NONE;
		}

		JPCError SSP1::save_changes(std::ostream& out) {
			this->offset = out.tellp();

			Utility::byteswap_inplace(sectionSize);

			out.write(magic, 4);
			out.write(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
			out.seekp(4, std::ios::cur);

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


			Utility::byteswap_inplace(flags);

			out.write(reinterpret_cast<char*>(&flags), sizeof(flags));

			Utility::byteswap_inplace(posRndm);
			Utility::byteswap_inplace(baseVel);
			Utility::byteswap_inplace(baseVelRndm);
			Utility::byteswap_inplace(velInfRate);
			Utility::byteswap_inplace(gravity);
			Utility::byteswap_inplace(timing);
			Utility::byteswap_inplace(life);
			Utility::byteswap_inplace(rate);
			Utility::byteswap_inplace(step);
			Utility::byteswap_inplace(rotateSpeed);
			Utility::byteswap_inplace(inheritRGB);
			Utility::byteswap_inplace(inheritAlpha);
			Utility::byteswap_inplace(inheritRGB);

			out.write(reinterpret_cast<char*>(&posRndm), sizeof(posRndm));
			out.write(reinterpret_cast<char*>(&baseVel), sizeof(baseVel));
			out.write(reinterpret_cast<char*>(&baseVelRndm), sizeof(baseVelRndm));
			out.write(reinterpret_cast<char*>(&velInfRate), sizeof(velInfRate));
			out.write(reinterpret_cast<char*>(&gravity), sizeof(gravity));
			out.write(reinterpret_cast<char*>(&timing), sizeof(timing));
			out.write(reinterpret_cast<char*>(&life), sizeof(life));
			out.write(reinterpret_cast<char*>(&rate), sizeof(rate));
			out.write(reinterpret_cast<char*>(&step), sizeof(step));

			writeVec2(out, globalScale2D);

			out.write(reinterpret_cast<char*>(&rotateSpeed), sizeof(rotateSpeed));

			out.write(reinterpret_cast<char*>(&inheritScale), sizeof(inheritScale));
			out.write(reinterpret_cast<char*>(&inheritAlpha), sizeof(inheritAlpha));
			out.write(reinterpret_cast<char*>(&inheritRGB), sizeof(inheritRGB));
			writeRGBA8(out, colorPrm);
			writeRGBA8(out, colorEnv);
			out.write(reinterpret_cast<char*>(&texIdx), sizeof(texIdx));

			return JPCError::NONE;
		}


		JPCError FLD1::read(std::istream& jpc, const unsigned int offset) {
			this->offset = offset;
			jpc.seekg(offset, std::ios::beg);

			if (!jpc.read(magic, 4)) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) return JPCError::REACHED_EOF;
			jpc.seekg(4, std::ios::cur);

			if (std::strncmp(magic, "FLD1", 4) != 0) return JPCError::UNKNOWN_CHUNK;

			Utility::byteswap_inplace(sectionSize);

			if (!jpc.read(reinterpret_cast<char*>(&flags), sizeof(flags))) return JPCError::REACHED_EOF;
			Utility::byteswap_inplace(flags);

			statusFlag = static_cast<FieldStatusFlag>(flags >> 0x10);
			type = static_cast<FieldType>(flags & 0xF);
			addType = static_cast<FieldAddType>((flags >> 8) & 0x03);

			if (!jpc.read(reinterpret_cast<char*>(&mag), sizeof(mag))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&magRndm), sizeof(magRndm))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&maxDist), sizeof(maxDist))) return JPCError::REACHED_EOF;

			if (!readVec3(jpc, jpc.tellg(), pos)) return JPCError::REACHED_EOF;

			if (!readVec3(jpc, jpc.tellg(), dir)) return JPCError::REACHED_EOF;

			if (!jpc.read(reinterpret_cast<char*>(&param1), sizeof(param1))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&param2), sizeof(param2))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&param3), sizeof(param3))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&fadeIn), sizeof(fadeIn))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&fadeOut), sizeof(fadeOut))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&enTime), sizeof(enTime))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&disTime), sizeof(disTime))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&cycle), sizeof(cycle))) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&unk1), sizeof(unk1))) return JPCError::REACHED_EOF;

			Utility::byteswap_inplace(mag);
			Utility::byteswap_inplace(magRndm);
			Utility::byteswap_inplace(maxDist);
			Utility::byteswap_inplace(param1);
			Utility::byteswap_inplace(param2);
			Utility::byteswap_inplace(param3);
			Utility::byteswap_inplace(fadeIn);
			Utility::byteswap_inplace(fadeOut);
			Utility::byteswap_inplace(enTime);
			Utility::byteswap_inplace(disTime);

			//refDistance = -1.0f;
			//innerSpeed = -1.0f;
			//outerSpeed = -1.0f;

			//if (type == FieldType::NEWTON) {
			//	refDistance = pow(param1, 2);
			//}
			//else if (type == FieldType::VORTEX) {
			//	innerSpeed = mag;
			//	outerSpeed = magRndm;
			//}
			//else if (type == FieldType::AIR) {
			//	refDistance = magRndm;
			//}
			//else if (type == FieldType::CONVECTION) {
			//	refDistance = param2;
			//}
			//else if (type == FieldType::SPIN) {
			//	innerSpeed = mag;
			//}

			return JPCError::NONE;
		}

		JPCError FLD1::save_changes(std::ostream& out) {
			this->offset = out.tellp();

			Utility::byteswap_inplace(sectionSize);

			out.write(magic, 4);
			out.write(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
			out.seekp(4, std::ios::cur);

			flags = (flags & ~0x00FF0000) | (static_cast<uint8_t>(statusFlag) << 0x10);
			flags = (flags & ~0x00000300) | ((static_cast<uint8_t>(addType) & 0x3) << 0x8);
			flags = (flags & ~0x0000000F) | (static_cast<uint8_t>(type) & 0xF);

			Utility::byteswap_inplace(flags);

			out.write(reinterpret_cast<char*>(&flags), sizeof(flags));

			Utility::byteswap_inplace(mag);
			Utility::byteswap_inplace(magRndm);
			Utility::byteswap_inplace(maxDist);
			Utility::byteswap_inplace(param1);
			Utility::byteswap_inplace(param2);
			Utility::byteswap_inplace(param3);
			Utility::byteswap_inplace(fadeIn);
			Utility::byteswap_inplace(fadeOut);
			Utility::byteswap_inplace(enTime);
			Utility::byteswap_inplace(disTime);

			out.write(reinterpret_cast<char*>(&mag), sizeof(mag));
			out.write(reinterpret_cast<char*>(&magRndm), sizeof(magRndm));
			out.write(reinterpret_cast<char*>(&maxDist), sizeof(maxDist));

			writeVec3(out, pos);
			writeVec3(out, dir);

			out.write(reinterpret_cast<char*>(&param1), sizeof(param1));
			out.write(reinterpret_cast<char*>(&param2), sizeof(param2));
			out.write(reinterpret_cast<char*>(&param3), sizeof(param3));
			out.write(reinterpret_cast<char*>(&fadeIn), sizeof(fadeIn));
			out.write(reinterpret_cast<char*>(&fadeOut), sizeof(fadeOut));
			out.write(reinterpret_cast<char*>(&enTime), sizeof(enTime));
			out.write(reinterpret_cast<char*>(&disTime), sizeof(disTime));
			out.write(reinterpret_cast<char*>(&cycle), sizeof(cycle));
			out.write(reinterpret_cast<char*>(&unk1), sizeof(unk1));

			return JPCError::NONE;
		}


		JPCError KFA1::read(std::istream& jpc, const unsigned int offset) {
			this->offset = offset;
			jpc.seekg(offset, std::ios::beg);

			if (!jpc.read(magic, 4)) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) return JPCError::REACHED_EOF;
			jpc.seekg(4, std::ios::cur);

			if (std::strncmp(magic, "KFA1", 4) != 0) return JPCError::UNKNOWN_CHUNK;

			Utility::byteswap_inplace(sectionSize);

			if (!jpc.read(reinterpret_cast<char*>(&keyType), sizeof(keyType))) return JPCError::REACHED_EOF;
			jpc.seekg(3, std::ios::cur);
			uint8_t keyCount;
			if (!jpc.read(reinterpret_cast<char*>(&keyCount), sizeof(keyCount))) return JPCError::REACHED_EOF;

			if (!jpc.read(reinterpret_cast<char*>(&isLoopEnable), 1)) return JPCError::REACHED_EOF;
			isLoopEnable = isLoopEnable != 0;

			if (!jpc.read(reinterpret_cast<char*>(&unk1), sizeof(unk1))) return JPCError::REACHED_EOF;

			jpc.seekg(0xD, std::ios::cur);
			for (uint8_t i = 0; i < keyCount; i++) {
				CurveKeyframe& keyframe = keys.emplace_back();
				if (!jpc.read(reinterpret_cast<char*>(&keyframe.time), sizeof(keyframe.time))) return JPCError::REACHED_EOF;
				if (!jpc.read(reinterpret_cast<char*>(&keyframe.value), sizeof(keyframe.value))) return JPCError::REACHED_EOF;
				if (!jpc.read(reinterpret_cast<char*>(&keyframe.tanIn), sizeof(keyframe.tanIn))) return JPCError::REACHED_EOF;
				if (!jpc.read(reinterpret_cast<char*>(&keyframe.tanOut), sizeof(keyframe.tanOut))) return JPCError::REACHED_EOF;
				
				Utility::byteswap_inplace(keyframe.time);
				Utility::byteswap_inplace(keyframe.value);
				Utility::byteswap_inplace(keyframe.tanIn);
				Utility::byteswap_inplace(keyframe.tanOut);
			}

			return JPCError::NONE;
		}

		JPCError KFA1::save_changes(std::ostream& out) {
			this->offset = out.tellp();

			Utility::byteswap_inplace(sectionSize);

			out.write(magic, 4);
			out.write(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
			out.seekp(4, std::ios::cur);

			out.write(reinterpret_cast<char*>(&keyType), sizeof(keyType));
			out.seekp(3, std::ios::cur);
			uint8_t keyCount = keys.size();
			out.write(reinterpret_cast<char*>(&keyCount), sizeof(keyCount));

			out.write(reinterpret_cast<char*>(&isLoopEnable), 1);
			out.write(reinterpret_cast<char*>(&unk1), sizeof(unk1));

			out.seekp(0xD, std::ios::cur);
			for (CurveKeyframe& keyframe : keys) {
				Utility::byteswap_inplace(keyframe.time);
				Utility::byteswap_inplace(keyframe.value);
				Utility::byteswap_inplace(keyframe.tanIn);
				Utility::byteswap_inplace(keyframe.tanOut);

				out.write(reinterpret_cast<char*>(&keyframe.time), sizeof(keyframe.time));
				out.write(reinterpret_cast<char*>(&keyframe.value), sizeof(keyframe.value));
				out.write(reinterpret_cast<char*>(&keyframe.tanIn), sizeof(keyframe.tanIn));
				out.write(reinterpret_cast<char*>(&keyframe.tanOut), sizeof(keyframe.tanOut));
			}

			return JPCError::NONE;
		}


		JPCError TDB1::read(std::istream& jpc, const unsigned int offset, const uint8_t texCount) {
			this->offset = offset;
			jpc.seekg(offset, std::ios::beg);

			if (!jpc.read(magic, 4)) return JPCError::REACHED_EOF;
			if (!jpc.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) return JPCError::REACHED_EOF;
			jpc.seekg(4, std::ios::cur);

			if (std::strncmp(magic, "TDB1", 4) != 0) return JPCError::UNKNOWN_CHUNK;

			Utility::byteswap_inplace(sectionSize);

			for (uint8_t i = 0; i < texCount; i++) {
				uint16_t& index = texIDs.emplace_back();
				if (!jpc.read(reinterpret_cast<char*>(&index), sizeof(index))) return JPCError::REACHED_EOF;
				Utility::byteswap_inplace(index);
			}

			return JPCError::NONE;
		}

		JPCError TDB1::populateFilenames(const std::vector<std::string>& texList) {
			for (const uint16_t& id : texIDs) {
				if (id > texList.size() - 1) return JPCError::MISSING_TEXTURE;

				texFilenames.push_back(texList[id]);
			}

			return JPCError::NONE;
		}

		JPCError TDB1::save_changes(std::ostream& out, const std::unordered_map<std::string, size_t>& textures) {
			this->offset = out.tellp();

			Utility::byteswap_inplace(sectionSize);

			out.write(magic, 4);
			out.write(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize)); //isnt used, shouldnt need to update?
			out.seekp(4, std::ios::cur);

			texIDs.clear();
			texIDs.reserve(texFilenames.size());
			for (const std::string& filename : texFilenames) {
				if (textures.count(filename) == 0) return JPCError::MISSING_TEXTURE;

				texIDs.push_back(textures.at(filename));
			}

			for (uint16_t& id : texIDs) {
				Utility::byteswap_inplace(id);

				out.write(reinterpret_cast<char*>(&id), sizeof(id));
			}

			return JPCError::NONE;
		}
	}
}

JPCError Particle::read(std::istream& jpc, unsigned int particle_offset) {
	jpc.seekg(particle_offset, std::ios::beg);
	if (!jpc.read(magicJEFF, 8)) {
		return JPCError::REACHED_EOF;
	}
	if (std::strncmp(magicJEFF, "JEFFjpa1", 8) != 0) {
		return JPCError::UNEXPECTED_VALUE;
	}
	if (!jpc.read((char*)&unknown_1, sizeof(unknown_1))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&num_chunks, sizeof(num_chunks))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&size, sizeof(size))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&num_kfa1_chunks, sizeof(num_kfa1_chunks))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&num_fld1_chunks, sizeof(num_fld1_chunks))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&num_textures, sizeof(num_textures))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&unknown_5, sizeof(unknown_5))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&particle_id, sizeof(particle_id))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&unknown_6, sizeof(unknown_6))) {
		return JPCError::REACHED_EOF;
	}

	Utility::byteswap_inplace(unknown_1);
	Utility::byteswap_inplace(num_chunks);
	Utility::byteswap_inplace(size);
	Utility::byteswap_inplace(particle_id);

	char magic[4];
	for (uint32_t i = 0; i < num_chunks; i++) {
		unsigned int numPaddingBytes = 0x20 - (jpc.tellg() % 0x20);
		if (numPaddingBytes == 0x20) {
			numPaddingBytes = 0;
		}
		jpc.seekg(numPaddingBytes, std::ios::cur);

		if (!jpc.read(magic, 4)) {
			return JPCError::REACHED_EOF;
		}
		jpc.seekg(-4, std::ios::cur);

		if (std::strncmp(magic, "BEM1", 4) == 0) {
			if (emitter.has_value()) return JPCError::DUPLICATE_CHUNK;
			if (JPCError err = emitter.emplace().read(jpc, jpc.tellg()); err != JPCError::NONE) return err;
		}
		else if (std::strncmp(magic, "BSP1", 4) == 0) {
			if (baseShape.has_value()) return JPCError::DUPLICATE_CHUNK;
			if (JPCError err = baseShape.emplace().read(jpc, jpc.tellg()); err != JPCError::NONE) return err;
		}
		else if (std::strncmp(magic, "ESP1", 4) == 0) {
			if (extraShape.has_value()) return JPCError::DUPLICATE_CHUNK;
			if (JPCError err = extraShape.emplace().read(jpc, jpc.tellg()); err != JPCError::NONE) return err;
		}
		else if (std::strncmp(magic, "ETX1", 4) == 0) {
			if (extraTex.has_value()) return JPCError::DUPLICATE_CHUNK;
			if (JPCError err = extraTex.emplace().read(jpc, jpc.tellg()); err != JPCError::NONE) return err;
		}
		else if (std::strncmp(magic, "SSP1", 4) == 0) {
			if (childShape.has_value()) return JPCError::DUPLICATE_CHUNK;
			if (JPCError err = childShape.emplace().read(jpc, jpc.tellg()); err != JPCError::NONE) return err;
		}
		else if (std::strncmp(magic, "FLD1", 4) == 0) {
			if (JPCError err = fields.emplace_back().read(jpc, jpc.tellg()); err != JPCError::NONE) return err;
		}
		else if (std::strncmp(magic, "KFA1", 4) == 0) {
			if (JPCError err = curves.emplace_back().read(jpc, jpc.tellg()); err != JPCError::NONE) return err;
		}
		else if (std::strncmp(magic, "TDB1", 4) == 0) {
			if (texDatabase.has_value()) return JPCError::DUPLICATE_CHUNK;
			if (JPCError err = texDatabase.emplace().read(jpc, jpc.tellg(), num_textures); err != JPCError::NONE) return err;
		}
		else {
			return JPCError::UNKNOWN_CHUNK;
		}
	}

	if (!emitter.has_value()) return JPCError::MISSING_CHUNK;
	if (!baseShape.has_value()) return JPCError::MISSING_CHUNK;
	if (!texDatabase.has_value()) return JPCError::MISSING_CHUNK;
	if (fields.size() > num_fld1_chunks) return JPCError::DUPLICATE_CHUNK;
	if (fields.size() < num_fld1_chunks) return JPCError::MISSING_CHUNK;
	if (curves.size() > num_kfa1_chunks) return JPCError::DUPLICATE_CHUNK;
	if (curves.size() < num_kfa1_chunks) return JPCError::MISSING_CHUNK;

	return JPCError::NONE;
}

JPCError Particle::save_changes(std::ostream& out, const std::unordered_map<std::string, size_t>& textures) {
	out.write(magicJEFF, 8);

	num_chunks = 3;
	if (!emitter.has_value()) return JPCError::MISSING_CHUNK;
	if (!baseShape.has_value()) return JPCError::MISSING_CHUNK;
	if (extraShape.has_value()) num_chunks += 1;
	if (extraTex.has_value()) num_chunks += 1;
	if (childShape.has_value()) num_chunks += 1;
	num_chunks += fields.size();
	num_chunks += curves.size();
	if (!texDatabase.has_value()) return JPCError::MISSING_CHUNK;

	num_kfa1_chunks = curves.size();
	num_fld1_chunks = fields.size();

	num_textures = texDatabase.value().texFilenames.size();

	Utility::byteswap_inplace(unknown_1);
	Utility::byteswap_inplace(num_chunks);
	Utility::byteswap_inplace(size); //not always accurate, shouldnt need to update?
	Utility::byteswap_inplace(particle_id);

	out.write(reinterpret_cast<char*>(&unknown_1), sizeof(unknown_1));
	out.write(reinterpret_cast<char*>(&num_chunks), sizeof(num_chunks));
	out.write(reinterpret_cast<char*>(&size), sizeof(size));
	out.write(reinterpret_cast<char*>(&num_kfa1_chunks), sizeof(num_kfa1_chunks));
	out.write(reinterpret_cast<char*>(&num_fld1_chunks), sizeof(num_fld1_chunks));
	out.write(reinterpret_cast<char*>(&num_textures), sizeof(num_textures));
	out.write(reinterpret_cast<char*>(&unknown_5), sizeof(unknown_5));
	out.write(reinterpret_cast<char*>(&particle_id), sizeof(particle_id));
	out.write(reinterpret_cast<char*>(&unknown_6), sizeof(unknown_6));

	//bem -> fld -> kfa -> bsp -> esp -> ssp -> etx -> tdb
	if (emitter.has_value()) {
		if (JPCError err = emitter.value().save_changes(out); err != JPCError::NONE) return err;
		padToLen(out, 0x20);
	}

	for (JParticle::chunks::FLD1& field : fields) {
		if (JPCError err = field.save_changes(out); err != JPCError::NONE) return err;
		padToLen(out, 0x20);
	}

	for (JParticle::chunks::KFA1& curve : curves) {
		if (JPCError err = curve.save_changes(out); err != JPCError::NONE) return err;
		padToLen(out, 0x20);
	}

	if (baseShape.has_value()) {
		if (JPCError err = baseShape.value().save_changes(out); err != JPCError::NONE) return err;
		padToLen(out, 0x20);
	}
	if (extraShape.has_value()) {
		if (JPCError err = extraShape.value().save_changes(out); err != JPCError::NONE) return err;
		padToLen(out, 0x20);
	}
	if (childShape.has_value()) {
		if (JPCError err = childShape.value().save_changes(out); err != JPCError::NONE) return err;
		padToLen(out, 0x20);
	}
	if (extraTex.has_value()) {
		if (JPCError err = extraTex.value().save_changes(out); err != JPCError::NONE) return err;
		padToLen(out, 0x20);
	}
	if (texDatabase.has_value()) {
		if (JPCError err = texDatabase.value().save_changes(out, textures); err != JPCError::NONE) return err;
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

	JPC::JPC() {

	}

	void JPC::initNew() {
		memcpy(magicJPAC, "JPAC1-00", 8);
		num_particles = 0;
		num_textures = 0;

		particle_index_by_id = {};
		particles = {};

		textures = {};
	}

	JPC JPC::createNew(const std::string& filename) {
		JPC newJPC{};
		newJPC.initNew();
		return newJPC;
	}

	JPCError JPC::loadFromBinary(std::istream& jpc) {
		JPCError err = JPCError::NONE;
		if (!jpc.read(magicJPAC, 8)) {
			return JPCError::REACHED_EOF;
		}
		if (std::strncmp(magicJPAC, "JPAC1-00", 8) != 0) {
			return JPCError::NOT_JPC;
		}
		if (!jpc.read((char*)&num_particles, sizeof(num_particles))) {
			return JPCError::REACHED_EOF;
		}
		if (!jpc.read((char*)&num_textures, sizeof(num_textures))) {
			return JPCError::REACHED_EOF;
		}
		Utility::byteswap_inplace(num_particles);
		Utility::byteswap_inplace(num_textures);

		jpc.seekg(0x20, std::ios::beg);

		for (uint16_t i = 0; i < num_particles; i++) {
			Particle& particle = particles.emplace_back();
			if (err = particle.read(jpc, jpc.tellg()); err != JPCError::NONE) 
				return err;

			particle_index_by_id[particle.particle_id] = i;

			unsigned int numPaddingBytes = 0x20 - (jpc.tellg() % 0x20);
			if (numPaddingBytes == 0x20) {
				numPaddingBytes = 0;
			}
			jpc.seekg(numPaddingBytes, std::ios::cur);
		}

		char magic[4];
		std::vector<std::string> texFilenames;
		for (uint16_t i = 0; i < num_textures; i++) {
			if (!jpc.read(magic, 4)) {
				return JPCError::REACHED_EOF;
			}
			if (std::strncmp(magic, "TEX1", 4) != 0) {
				return JPCError::UNKNOWN_CHUNK;
			}

			jpc.seekg(8, std::ios::cur); //ignore section size, its not accurate anyway, also skip 4 unused bytes
			std::string filename(0x14, '\0');
			if (!jpc.read(&filename[0], 0x14)) {
				return JPCError::REACHED_EOF;
			}

			texFilenames.push_back(filename);
			textures[filename] = i;
		}

		for (Particle& particle : particles) {
			if(!particle.texDatabase.has_value()) return JPCError::MISSING_CHUNK;

			if (err = particle.texDatabase.value().populateFilenames(texFilenames); err != JPCError::NONE) return err;
		}

		return JPCError::NONE;
	}

	JPCError JPC::loadFromFile(const std::string& filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			return JPCError::COULD_NOT_OPEN;
		}
		return loadFromBinary(file);
	}

	JPCError JPC::addParticle(const Particle& particle) {
		if (particle_index_by_id.count(particle.particle_id) > 0) return JPCError::DUPLICATE_PARTICLE_ID;

		particle_index_by_id[particle.particle_id] = particles.size();
		particles.push_back(particle);

		return JPCError::NONE;
	}

	JPCError JPC::replaceParticle(const Particle& particle) {
		if (particle_index_by_id.count(particle.particle_id) == 0) return JPCError::MISSING_PARTICLE;

		particles[particle_index_by_id[particle.particle_id]] = particle;

		return JPCError::NONE;
	}

	JPCError JPC::addTexture(const std::string& filename) {
		std::string filenamePad = filename;
		filenamePad.resize(0x18, '\0');
		if (textures.count(filenamePad) > 0) return JPCError::DUPLICATE_FILENAME;

		size_t nextIndex = textures.size();
		textures[filenamePad] = nextIndex;
		return JPCError::NONE;
	}

	JPCError JPC::writeToStream(std::ostream& out) {
		JPCError err = JPCError::NONE;
		out.write(magicJPAC, 8);

		num_particles = particles.size();
		num_textures = textures.size();

		Utility::byteswap_inplace(num_particles);
		Utility::byteswap_inplace(num_textures);

		out.write(reinterpret_cast<char*>(&num_particles), sizeof(num_particles));
		out.write(reinterpret_cast<char*>(&num_textures), sizeof(num_textures));

		out.seekp(0x20, std::ios::beg);

		for (Particle& particle : particles) {
			if (err = particle.save_changes(out, textures); err != JPCError::NONE) return err;
			padToLen(out, 0x20);
		}

		std::streamoff texStart = out.tellp(); //some stuff to preserve original order
		for (const auto& [filename, index] : textures) {
			out.seekp(texStart + (index * 0x20), std::ios::beg); //some stuff to preserve original order
			out.write("TEX1", 4);

			out.write("\x00\x00\x00\x20\x00\x00\x00\x00", 8);
			out.write(&filename[0], 0x14);
		}

		return JPCError::NONE;
	}

	JPCError JPC::writeToFile(const std::string& outFilePath) {
		std::ofstream outFile(outFilePath, std::ios::binary);
		if (!outFile.is_open()) {
			return JPCError::COULD_NOT_OPEN;
		}
		return writeToStream(outFile);
	}
}
