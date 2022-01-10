#include "bflyt.hpp"

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

std::string readNullTerminatedStr(std::istream& in, const unsigned int offset) {
	in.seekg(offset, std::ios::beg);

	std::string ret;
	char character = '\x00';
	do {
		if (!in.read(&character, 1)) {
			ret.clear();
			return ret;
		}
		ret += character;
	} while (character != '\x00');

	return ret;
}

std::u16string readNullTerminatedU16str(std::istream& in, const unsigned int offset) {
	in.seekg(offset, std::ios::beg);

	std::u16string ret;
	char16_t character = '\x0000';
	do {
		if (!in.read(reinterpret_cast<char*>(&character), 2)) {
			ret.clear();
			return ret;
		}
		ret += character;
	} while (character != '\x0000');

	return ret;
}

std::optional<RGBA> readRGBA(std::istream& in, const unsigned int offset) {
	in.seekg(offset, std::ios::beg);

	RGBA ret;
	if (!in.read(reinterpret_cast<char*>(&ret.R), sizeof(ret.R))) return std::nullopt;
	if (!in.read(reinterpret_cast<char*>(&ret.G), sizeof(ret.G))) return std::nullopt;
	if (!in.read(reinterpret_cast<char*>(&ret.B), sizeof(ret.B))) return std::nullopt;
	if (!in.read(reinterpret_cast<char*>(&ret.A), sizeof(ret.A))) return std::nullopt;

	return ret;
}

FLYTError readMaterial(std::istream& bflyt, const unsigned int offset, material& out) {
	bflyt.seekg(offset, std::ios::beg);

	out.name.resize(0x1C);
	if (!bflyt.read(&out.name[0], 0x1C)) return FLYTError::REACHED_EOF;

	{
		std::optional<RGBA> color = readRGBA(bflyt, bflyt.tellg());
		if (!color.has_value()) {
			return FLYTError::REACHED_EOF; //returns nullopt if read fails
		}
		out.blackColor = color.value();

		color = readRGBA(bflyt, bflyt.tellg());
		if (!color.has_value()) {
			return FLYTError::REACHED_EOF; //returns nullopt if read fails
		}
		out.whiteColor = color.value();
	}

	uint32_t flags = 0;
	if (!bflyt.read(reinterpret_cast<char*>(&flags), sizeof(flags))) return FLYTError::REACHED_EOF;

	Utility::byteswap_inplace(flags);

	out.texCount = flags & 3;
	out.mtxCount = (flags >> 2) & 3;
	out.texCoordGenCount = (flags >> 4) & 3;
	out.tevStageCount = (flags >> 6) & 7;
	out.enableAlphaCompare = (flags >> 9) & 1;
	out.enableBlend = (flags >> 10) & 1;
	out.textureOnly = (flags >> 11) & 1;
	out.blendLogic = (flags >> 12) & 1;
	out.indParams = (flags >> 14) & 1;
	out.projMapCount = (flags >> 15) & 3;
	out.fontShadowParams = (flags >> 17) & 1;
	out.alphaInterpolation = (flags >> 18) & 1;

	out.texRefs.reserve(out.texCount);
	for (unsigned int i = 0; i < out.texCount; i++) {
		texRef ref;
		if (!bflyt.read(reinterpret_cast<char*>(&ref.nameIndex), sizeof(ref.nameIndex))) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&ref.wrapModeU), 1)) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&ref.wrapModeV), 1)) return FLYTError::REACHED_EOF;

		Utility::byteswap_inplace(ref.nameIndex);

		out.texRefs.push_back(ref);
	}

	out.texTransforms.reserve(out.mtxCount);
	for (unsigned int i = 0; i < out.mtxCount; i++) {
		texTransform transform;
		if (!bflyt.read(reinterpret_cast<char*>(&transform.translation[0]), sizeof(transform.translation[0]))) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&transform.translation[1]), sizeof(transform.translation[1]))) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&transform.rotation), sizeof(transform.rotation))) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&transform.scale[0]), sizeof(transform.scale[0]))) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&transform.scale[1]), sizeof(transform.scale[1]))) return FLYTError::REACHED_EOF;

		Utility::byteswap_inplace(transform.translation[0]);
		Utility::byteswap_inplace(transform.translation[1]);
		Utility::byteswap_inplace(transform.rotation);
		Utility::byteswap_inplace(transform.scale[0]);
		Utility::byteswap_inplace(transform.scale[1]);

		out.texTransforms.push_back(transform);
	}

	out.texCoordGens.reserve(out.texCoordGenCount);
	for (unsigned int i = 0; i < out.texCoordGenCount; i++) {
		texCoordGen coordGen;
		if (!bflyt.read(reinterpret_cast<char*>(&coordGen.matrix), 1)) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&coordGen.source), 1)) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&coordGen.unk), sizeof(coordGen.unk))) return FLYTError::REACHED_EOF;

		out.texCoordGens.push_back(coordGen);
	}

	out.tevStages.reserve(out.tevStageCount);
	for (unsigned int i = 0; i < out.tevStageCount; i++) {
		tevStage stage;
		if (!bflyt.read(reinterpret_cast<char*>(&stage.colorMode), 1)) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&stage.alphaMode), 1)) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&stage.padding_0x00), sizeof(stage.padding_0x00))) return FLYTError::REACHED_EOF;

		out.tevStages.push_back(stage);
	}

	if (out.enableAlphaCompare) {
		alphaCompare compare;
		if (!bflyt.read(reinterpret_cast<char*>(&compare.compareMode), 1)) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&compare.unk), 3)) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&compare.value), sizeof(compare.value))) return FLYTError::REACHED_EOF;

		Utility::byteswap_inplace(compare.value);

		out.alphaComparison = compare;
	}

	if (out.enableBlend) {
		blendMode blend;
		if (!bflyt.read(reinterpret_cast<char*>(&blend.blendOp), 1)) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&blend.sourceFactor), 1)) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&blend.destFactor), 1)) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&blend.logicOp), 1)) return FLYTError::REACHED_EOF;

		out.blendingMode = blend;
	}

	if (out.blendLogic) {
		blendMode blend;
		if (!bflyt.read(reinterpret_cast<char*>(&blend.blendOp), 1)) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&blend.sourceFactor), 1)) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&blend.destFactor), 1)) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&blend.logicOp), 1)) return FLYTError::REACHED_EOF;

		out.blendModeLogic = blend;
	}

	if (out.indParams) {
		indirectParam param;
		if (!bflyt.read(reinterpret_cast<char*>(&param.rotation), sizeof(param.rotation))) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&param.scaleX), sizeof(param.scaleX))) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&param.scaleY), sizeof(param.scaleY))) return FLYTError::REACHED_EOF;

		Utility::byteswap_inplace(param.rotation);
		Utility::byteswap_inplace(param.scaleX);
		Utility::byteswap_inplace(param.scaleY);

		out.indParameter = param;
	}

	out.projectionMaps.reserve(out.projMapCount);
	for (unsigned int i = 0; i < out.projMapCount; i++) {
		projectionMap map;
		if (!bflyt.read(reinterpret_cast<char*>(&map.posX), sizeof(map.posX))) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&map.posY), sizeof(map.posY))) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&map.scaleX), sizeof(map.scaleX))) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&map.scaleY), sizeof(map.scaleY))) return FLYTError::REACHED_EOF;
		if (!bflyt.read(reinterpret_cast<char*>(&map.flags), sizeof(map.flags))) return FLYTError::REACHED_EOF;

		Utility::byteswap_inplace(map.posX);
		Utility::byteswap_inplace(map.posY);
		Utility::byteswap_inplace(map.scaleX);
		Utility::byteswap_inplace(map.scaleY);
		Utility::byteswap_inplace(map.flags);

		out.projectionMaps.push_back(map);
	}

	if (out.fontShadowParams) {
		fontShadowParameter param;

		std::optional<RGBA> color = readRGBA(bflyt, bflyt.tellg());
		if (!color.has_value()) {
			return FLYTError::REACHED_EOF; //returns nullopt if read fails
		}
		param.blackColor = color.value();

		color = readRGBA(bflyt, bflyt.tellg());
		if (!color.has_value()) {
			return FLYTError::REACHED_EOF; //returns nullopt if read fails
		}
		param.whiteColor = color.value();

		out.fontShadowParam = param;
	}

	return FLYTError::NONE;
}



FLYTError cnt1::read(std::istream& bflyt, const unsigned int offset) {
	bflyt.seekg(offset, std::ios::beg);
	this->offset = offset;

	if (!bflyt.read(magic, 4)) return FLYTError::REACHED_EOF;
	if (strncmp(magic, "cnt1", 4) != 0) {
		return FLYTError::UNKNOWN_SECTION;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&paneNamesOffset), sizeof(paneNamesOffset))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&paneCount), sizeof(paneCount))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&animCount), sizeof(animCount))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(sectionSize);
	Utility::byteswap_inplace(paneNamesOffset);
	Utility::byteswap_inplace(paneCount);
	Utility::byteswap_inplace(animCount);

	if (std::string& name = readNullTerminatedStr(bflyt, bflyt.tellg()); name.empty()) {
		return FLYTError::REACHED_EOF; //empty string means it could not read a character from file
	}
	else {
		this->name = name;
	}

	bflyt.seekg(this->offset + paneNamesOffset, std::ios::beg);
	for (unsigned int i = 0; i < paneCount; i++) {
		std::string paneName;
		paneName.resize(0x18);
		if (!bflyt.read(reinterpret_cast<char*>(&paneName[0]), 0x18)) {
			return FLYTError::REACHED_EOF;
		}

		paneNames.push_back(paneName);
	}

	unsigned int numPaddingBytes = 4 - (bflyt.tellg() % 4);
	if (numPaddingBytes == 4) {
		numPaddingBytes = 0;
	}
	bflyt.seekg(numPaddingBytes, std::ios::cur);
	animNameTableOffset = (unsigned int)bflyt.tellg() - this->offset;

	for (unsigned int i = 0; i < animCount; i++) {
		uint32_t nameOffset = 0;
		if (!bflyt.read(reinterpret_cast<char*>(&nameOffset), sizeof(nameOffset))) {
			return FLYTError::REACHED_EOF;
		}

		Utility::byteswap_inplace(nameOffset);
		animNameOffsets.push_back(nameOffset);
	}

	for (const uint32_t nameOffset : animNameOffsets) {
		if (std::string& name = readNullTerminatedStr(bflyt, this->offset + animNameTableOffset + nameOffset); name.empty()) {
			return FLYTError::REACHED_EOF; //empty string means it could not read a character from file
		}
		else {
			animNames.push_back(name);
		}
	}

	bflyt.seekg(this->offset + sectionSize, std::ios::beg);

	return FLYTError::NONE;
}

FLYTError cnt1::save_changes(std::ostream& out) {
	return FLYTError::NONE;
}


FLYTError usd1::read(std::istream& bflyt, const unsigned int offset) {
	bflyt.seekg(offset, std::ios::beg);
	this->offset = offset;

	if (!bflyt.read(magic, 4)) return FLYTError::REACHED_EOF;
	if (strncmp(magic, "usd1", 4) != 0) {
		return FLYTError::UNKNOWN_SECTION;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&numEntries), sizeof(numEntries))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(sectionSize);
	Utility::byteswap_inplace(numEntries);

	for (unsigned int i = 0; i < numEntries; i++) {
		userDataEntry entry;
		if (!bflyt.read(reinterpret_cast<char*>(&entry.nameOffset), sizeof(entry.nameOffset))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&entry.dataOffset), sizeof(entry.dataOffset))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&entry.dataLen), sizeof(entry.dataLen))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&entry.dataType), 1)) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&entry.unk), sizeof(entry.unk))) {
			return FLYTError::REACHED_EOF;
		}

		Utility::byteswap_inplace(entry.nameOffset);
		Utility::byteswap_inplace(entry.dataOffset);
		Utility::byteswap_inplace(entry.dataLen);

		if (entry.nameOffset != 0) {
			bflyt.seekg(this->offset + entry.nameOffset, std::ios::beg);
			if (std::string& name = readNullTerminatedStr(bflyt, this->offset + entry.nameOffset); name.empty()) {
				return FLYTError::REACHED_EOF; //empty string means it could not read a character from file
			}
			else {
				entry.name = name;
			}
		}

		if (entry.dataOffset != 0) {
			bflyt.seekg(this->offset + entry.dataOffset, std::ios::beg);
			switch (entry.dataType) {
				case UserDataType::STRING:
					if (entry.dataLen != 0) {
						std::string data;
						data.resize(entry.dataLen);
						if (!bflyt.read(reinterpret_cast<char*>(&data[0]), entry.dataLen)) {
							return FLYTError::REACHED_EOF;
						}
						entry.data = data;
					}
					else {
						if (std::string& data = readNullTerminatedStr(bflyt, this->offset + entry.dataOffset); data.empty()) {
							return FLYTError::REACHED_EOF; //empty string means it could not read a character from file
						}
						else {
							entry.data = data;
						}
					}
					break;
				case UserDataType::INT:
					entry.data.emplace<std::vector<int32_t>>();
					std::get<std::vector<int32_t>>(entry.data).reserve(entry.dataLen);
					for (unsigned int i = 0; i < entry.dataLen; i++) {
						int32_t value;
						if (!bflyt.read(reinterpret_cast<char*>(&value), sizeof(value))) {
							return FLYTError::REACHED_EOF;
						}

						Utility::byteswap_inplace(value);

						std::get<std::vector<int32_t>>(entry.data).push_back(value);
					}
					break;
				case UserDataType::FLOAT:
					entry.data.emplace<std::vector<float>>();
					std::get<std::vector<float>>(entry.data).reserve(entry.dataLen);
					for (unsigned int i = 0; i < entry.dataLen; i++) {
						float value;
						if (!bflyt.read(reinterpret_cast<char*>(&value), sizeof(value))) {
							return FLYTError::REACHED_EOF;
						}

						Utility::byteswap_inplace(value);

						std::get<std::vector<float>>(entry.data).push_back(value);
					}
					break;
				case UserDataType::STRUCT:
					break; //type not implemented, should not appear in Wii U files
			}
		}
	}

	bflyt.seekg(this->offset + sectionSize, std::ios::beg);

	return FLYTError::NONE;
}

FLYTError usd1::save_changes(std::ostream& out) {
	return FLYTError::NONE;
}


PaneBase::~PaneBase() {

}


pan1::~pan1() {
	
}

FLYTError pan1::read(std::istream& bflyt, const unsigned int offset) {
	bflyt.seekg(offset, std::ios::beg);
	this->offset = offset;

	if (!bflyt.read(magic, 4)) return FLYTError::REACHED_EOF;
	if (strncmp(magic, "pan1", 4) != 0) {
		return FLYTError::UNKNOWN_SECTION;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.bitFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.originFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.alpha), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.paneMagFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	baseData.name.resize(0x18);
	baseData.userInfo.resize(0x8);
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.name[0]), 0x18)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.userInfo[0]), 0x8)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transX), sizeof(baseData.transX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transY), sizeof(baseData.transY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transZ), sizeof(baseData.transZ))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotX), sizeof(baseData.rotX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotY), sizeof(baseData.rotY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotZ), sizeof(baseData.rotZ))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.scaleX), sizeof(baseData.scaleX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.scaleY), sizeof(baseData.scaleY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.width), sizeof(baseData.width))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.height), sizeof(baseData.height))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(sectionSize);
	Utility::byteswap_inplace(baseData.transX);
	Utility::byteswap_inplace(baseData.transY);
	Utility::byteswap_inplace(baseData.transZ);
	Utility::byteswap_inplace(baseData.rotX);
	Utility::byteswap_inplace(baseData.rotY);
	Utility::byteswap_inplace(baseData.rotZ);
	Utility::byteswap_inplace(baseData.scaleX);
	Utility::byteswap_inplace(baseData.scaleY);
	Utility::byteswap_inplace(baseData.width);
	Utility::byteswap_inplace(baseData.height);

	bflyt.seekg(this->offset + sectionSize, std::ios::beg);

	return FLYTError::NONE;
}

FLYTError pan1::save_changes(std::ostream& out) {
	return FLYTError::NONE;
}


bnd1::~bnd1() {
	
}

FLYTError bnd1::read(std::istream& bflyt, const unsigned int offset) {
	bflyt.seekg(offset, std::ios::beg);
	this->offset = offset;

	if (!bflyt.read(magic, 4)) return FLYTError::REACHED_EOF;
	if (strncmp(magic, "bnd1", 4) != 0) {
		return FLYTError::UNKNOWN_SECTION;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.bitFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.originFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.alpha), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.paneMagFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	baseData.name.resize(0x18);
	baseData.userInfo.resize(0x8);
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.name[0]), 0x18)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.userInfo[0]), 0x8)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transX), sizeof(baseData.transX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transY), sizeof(baseData.transY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transZ), sizeof(baseData.transZ))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotX), sizeof(baseData.rotX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotY), sizeof(baseData.rotY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotZ), sizeof(baseData.rotZ))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.scaleX), sizeof(baseData.scaleX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.scaleY), sizeof(baseData.scaleY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.width), sizeof(baseData.width))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.height), sizeof(baseData.height))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(sectionSize);
	Utility::byteswap_inplace(baseData.transX);
	Utility::byteswap_inplace(baseData.transY);
	Utility::byteswap_inplace(baseData.transZ);
	Utility::byteswap_inplace(baseData.rotX);
	Utility::byteswap_inplace(baseData.rotY);
	Utility::byteswap_inplace(baseData.rotZ);
	Utility::byteswap_inplace(baseData.scaleX);
	Utility::byteswap_inplace(baseData.scaleY);
	Utility::byteswap_inplace(baseData.width);
	Utility::byteswap_inplace(baseData.height);

	bflyt.seekg(this->offset + sectionSize, std::ios::beg);

	return FLYTError::NONE;
}

FLYTError bnd1::save_changes(std::ostream& out) {
	return FLYTError::NONE;
}


wnd1::~wnd1() {
	
}

FLYTError wnd1::read(std::istream& bflyt, const unsigned int offset) {
	bflyt.seekg(offset, std::ios::beg);
	this->offset = offset;

	if (!bflyt.read(magic, 4)) return FLYTError::REACHED_EOF;
	if (strncmp(magic, "wnd1", 4) != 0) {
		return FLYTError::UNKNOWN_SECTION;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.bitFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.originFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.alpha), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.paneMagFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	baseData.name.resize(0x18);
	baseData.userInfo.resize(0x8);
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.name[0]), 0x18)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.userInfo[0]), 0x8)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transX), sizeof(baseData.transX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transY), sizeof(baseData.transY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transZ), sizeof(baseData.transZ))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotX), sizeof(baseData.rotX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotY), sizeof(baseData.rotY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotZ), sizeof(baseData.rotZ))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.scaleX), sizeof(baseData.scaleX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.scaleY), sizeof(baseData.scaleY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.width), sizeof(baseData.width))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.height), sizeof(baseData.height))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(sectionSize);
	Utility::byteswap_inplace(baseData.transX);
	Utility::byteswap_inplace(baseData.transY);
	Utility::byteswap_inplace(baseData.transZ);
	Utility::byteswap_inplace(baseData.rotX);
	Utility::byteswap_inplace(baseData.rotY);
	Utility::byteswap_inplace(baseData.rotZ);
	Utility::byteswap_inplace(baseData.scaleX);
	Utility::byteswap_inplace(baseData.scaleY);
	Utility::byteswap_inplace(baseData.width);
	Utility::byteswap_inplace(baseData.height);

	if (!bflyt.read(reinterpret_cast<char*>(&leftStretch), sizeof(leftStretch))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&rightStretch), sizeof(rightStretch))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&topStretch), sizeof(topStretch))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&bottomStretch), sizeof(bottomStretch))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&frameSizeLeft), sizeof(frameSizeLeft))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&frameSizeRight), sizeof(frameSizeRight))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&frameSizeTop), sizeof(frameSizeTop))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&frameSizeBottom), sizeof(frameSizeBottom))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&frameNum), sizeof(frameNum))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&bitFlags), sizeof(bitFlags))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&contentOffset), sizeof(contentOffset))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&frameTableOffset), sizeof(frameTableOffset))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(leftStretch);
	Utility::byteswap_inplace(rightStretch);
	Utility::byteswap_inplace(topStretch);
	Utility::byteswap_inplace(bottomStretch);
	Utility::byteswap_inplace(frameSizeLeft);
	Utility::byteswap_inplace(frameSizeRight);
	Utility::byteswap_inplace(frameSizeTop);
	Utility::byteswap_inplace(frameSizeBottom);
	Utility::byteswap_inplace(contentOffset);
	Utility::byteswap_inplace(frameTableOffset);

	bflyt.seekg(this->offset + contentOffset, std::ios::beg);
	std::optional<RGBA> color = readRGBA(bflyt, bflyt.tellg());
	if (!color.has_value()) {
		return FLYTError::REACHED_EOF; //returns nullopt if read fails
	}
	content.vertexColTL = color.value();

	color = readRGBA(bflyt, bflyt.tellg());
	if (!color.has_value()) {
		return FLYTError::REACHED_EOF; //returns nullopt if read fails
	}
	content.vertexColTR = color.value();

	color = readRGBA(bflyt, bflyt.tellg());
	if (!color.has_value()) {
		return FLYTError::REACHED_EOF; //returns nullopt if read fails
	}
	content.vertexColBL = color.value();

	color = readRGBA(bflyt, bflyt.tellg());
	if (!color.has_value()) {
		return FLYTError::REACHED_EOF; //returns nullopt if read fails
	}
	content.vertexColBR = color.value();

	if (!bflyt.read(reinterpret_cast<char*>(&content.matIndex), sizeof(content.matIndex))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&content.numCoords), sizeof(content.numCoords))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&content.padding_0x00), sizeof(content.padding_0x00))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(content.matIndex);

	content.coords.reserve(content.numCoords);
	for (uint8_t i = 0; i < content.numCoords; i++) {
		UVCoords coord;
		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordTL[0]), sizeof(coord.coordTL[0]))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordTL[1]), sizeof(coord.coordTL[1]))) {
			return FLYTError::REACHED_EOF;
		}

		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordTR[0]), sizeof(coord.coordTR[0]))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordTR[1]), sizeof(coord.coordTR[1]))) {
			return FLYTError::REACHED_EOF;
		}

		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordBL[0]), sizeof(coord.coordBL[0]))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordBL[1]), sizeof(coord.coordBL[1]))) {
			return FLYTError::REACHED_EOF;
		}

		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordBR[0]), sizeof(coord.coordBR[0]))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordBR[1]), sizeof(coord.coordBR[1]))) {
			return FLYTError::REACHED_EOF;
		}

		Utility::byteswap_inplace(coord.coordTL[0]);
		Utility::byteswap_inplace(coord.coordTL[1]);
		Utility::byteswap_inplace(coord.coordTR[0]);
		Utility::byteswap_inplace(coord.coordTR[1]);
		Utility::byteswap_inplace(coord.coordBL[0]);
		Utility::byteswap_inplace(coord.coordBL[1]);
		Utility::byteswap_inplace(coord.coordBR[0]);
		Utility::byteswap_inplace(coord.coordBR[1]);

		content.coords.push_back(coord);
	}


	bflyt.seekg(this->offset + frameTableOffset);
	for (unsigned int i = 0; i < frameNum; i++) {
		uint32_t frameOffset = 0;
		if (!bflyt.read(reinterpret_cast<char*>(&frameOffset), sizeof(frameOffset))) {
			return FLYTError::REACHED_EOF;
		}

		Utility::byteswap_inplace(frameOffset);
		frameTable.push_back(frameOffset);
	}

	for (const uint32_t frameOffset : frameTable) {
		bflyt.seekg(this->offset + frameOffset, std::ios::beg);

		windowFrame frame;
		if (!bflyt.read(reinterpret_cast<char*>(&frame.matIndex), sizeof(frame.matIndex))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&frame.texFlip), sizeof(frame.texFlip))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&frame.padding_0x00), sizeof(frame.padding_0x00))) {
			return FLYTError::REACHED_EOF;
		}

		Utility::byteswap_inplace(frame.matIndex);

		frames.push_back(frame);
	}

	return FLYTError::NONE;
}

FLYTError wnd1::save_changes(std::ostream& out) {
	return FLYTError::NONE;
}


txt1::~txt1() {
	
}

FLYTError txt1::read(std::istream& bflyt, const unsigned int offset) {
	bflyt.seekg(offset, std::ios::beg);
	this->offset = offset;

	if (!bflyt.read(magic, 4)) return FLYTError::REACHED_EOF;
	if (strncmp(magic, "txt1", 4) != 0) {
		return FLYTError::UNKNOWN_SECTION;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.bitFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.originFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.alpha), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.paneMagFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	baseData.name.resize(0x18);
	baseData.userInfo.resize(0x8);
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.name[0]), 0x18)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.userInfo[0]), 0x8)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transX), sizeof(baseData.transX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transY), sizeof(baseData.transY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transZ), sizeof(baseData.transZ))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotX), sizeof(baseData.rotX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotY), sizeof(baseData.rotY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotZ), sizeof(baseData.rotZ))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.scaleX), sizeof(baseData.scaleX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.scaleY), sizeof(baseData.scaleY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.width), sizeof(baseData.width))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.height), sizeof(baseData.height))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(sectionSize);
	Utility::byteswap_inplace(baseData.transX);
	Utility::byteswap_inplace(baseData.transY);
	Utility::byteswap_inplace(baseData.transZ);
	Utility::byteswap_inplace(baseData.rotX);
	Utility::byteswap_inplace(baseData.rotY);
	Utility::byteswap_inplace(baseData.rotZ);
	Utility::byteswap_inplace(baseData.scaleX);
	Utility::byteswap_inplace(baseData.scaleY);
	Utility::byteswap_inplace(baseData.width);
	Utility::byteswap_inplace(baseData.height);

	if (!bflyt.read(reinterpret_cast<char*>(&texLen), sizeof(texLen))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&restrictedLen), sizeof(restrictedLen))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&matIndex), sizeof(matIndex))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&fontIndex), sizeof(fontIndex))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&textAlignment), sizeof(textAlignment))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&lineAlignment), sizeof(lineAlignment))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&bitflags), sizeof(bitflags))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&italicTilt), sizeof(italicTilt))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&textOffset), sizeof(textOffset))) {
		return FLYTError::REACHED_EOF;
	}

	std::optional<RGBA> color = readRGBA(bflyt, bflyt.tellg());
	if (!color.has_value()) {
		return FLYTError::REACHED_EOF; //returns nullopt if read fails
	}
	fontColorTop = color.value();
	color = readRGBA(bflyt, bflyt.tellg());
	if (!color.has_value()) {
		return FLYTError::REACHED_EOF; //returns nullopt if read fails
	}
	fontColorBottom = color.value();

	if (!bflyt.read(reinterpret_cast<char*>(&fontSizeX), sizeof(fontSizeX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&fontSizeY), sizeof(fontSizeY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&charSpace), sizeof(charSpace))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&lineSpace), sizeof(lineSpace))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&nameOffset), sizeof(nameOffset))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&shadowPosX), sizeof(shadowPosX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&shadowPosY), sizeof(shadowPosY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&shadowSizeX), sizeof(shadowSizeX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&shadowSizeY), sizeof(shadowSizeY))) {
		return FLYTError::REACHED_EOF;
	}

	color = readRGBA(bflyt, bflyt.tellg());
	if (!color.has_value()) {
		return FLYTError::REACHED_EOF; //returns nullopt if read fails
	}
	shadowColorTop = color.value();
	color = readRGBA(bflyt, bflyt.tellg());
	if (!color.has_value()) {
		return FLYTError::REACHED_EOF; //returns nullopt if read fails
	}
	shadowColorBottom = color.value();

	if (!bflyt.read(reinterpret_cast<char*>(&shadowItalicTilt), sizeof(shadowItalicTilt))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(texLen);
	Utility::byteswap_inplace(restrictedLen);
	Utility::byteswap_inplace(matIndex);
	Utility::byteswap_inplace(fontIndex);
	Utility::byteswap_inplace(italicTilt);
	Utility::byteswap_inplace(textOffset);
	Utility::byteswap_inplace(fontSizeX);
	Utility::byteswap_inplace(fontSizeY);
	Utility::byteswap_inplace(charSpace);
	Utility::byteswap_inplace(lineSpace);
	Utility::byteswap_inplace(nameOffset);
	Utility::byteswap_inplace(shadowPosX);
	Utility::byteswap_inplace(shadowPosY);
	Utility::byteswap_inplace(shadowSizeX);
	Utility::byteswap_inplace(shadowSizeY);
	Utility::byteswap_inplace(shadowItalicTilt);

	if (textOffset != 0) {
		if (std::u16string& name = readNullTerminatedU16str(bflyt, this->offset + textOffset); name.empty()) {
			return FLYTError::REACHED_EOF; //empty string means it could not read a character from file
		}
		else {
			text = name;
		}
	}

	if (nameOffset != 0) {
		if (std::string& name = readNullTerminatedStr(bflyt, this->offset + nameOffset); name.empty()) {
			return FLYTError::REACHED_EOF; //empty string means it could not read a character from file
		}
		else {
			textBoxName = name;
		}
	}

	if ((bitflags >> 4) & 0x01) { //per char transform flag
		uint32_t transformOffset = 0;
		if (!bflyt.read(reinterpret_cast<char*>(&transformOffset), sizeof(transformOffset))) {
			return FLYTError::REACHED_EOF;
		}

		Utility::byteswap_inplace(transformOffset);
		charTransformOffset = transformOffset;

		bflyt.seekg(this->offset + transformOffset);
		perCharTransform transform;
		if (!bflyt.read(reinterpret_cast<char*>(&transform.curveTimeOffset), sizeof(transform.curveTimeOffset))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&transform.animCurveWidth), sizeof(transform.animCurveWidth))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&transform.loopType), sizeof(transform.loopType))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&transform.verticalOrigin), sizeof(transform.verticalOrigin))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&transform.hasAnimInfo), 1)) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&transform.padding_0x00), sizeof(transform.padding_0x00))) {
			return FLYTError::REACHED_EOF;
		}

		Utility::byteswap_inplace(transform.curveTimeOffset);
		Utility::byteswap_inplace(transform.animCurveWidth);

		charTransform = transform;
	}
	else {
		charTransformOffset = std::nullopt;
		charTransform = std::nullopt;
	}

	bflyt.seekg(this->offset + sectionSize, std::ios::beg);

	return FLYTError::NONE;
}

FLYTError txt1::save_changes(std::ostream& out) {
	return FLYTError::NONE;
}


pic1::~pic1() {

}

FLYTError pic1::read(std::istream& bflyt, const unsigned int offset) {
	bflyt.seekg(offset, std::ios::beg);
	this->offset = offset;

	if (!bflyt.read(magic, 4)) return FLYTError::REACHED_EOF;
	if (strncmp(magic, "pic1", 4) != 0) {
		return FLYTError::UNKNOWN_SECTION;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.bitFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.originFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.alpha), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.paneMagFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	baseData.name.resize(0x18);
	baseData.userInfo.resize(0x8);
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.name[0]), 0x18)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.userInfo[0]), 0x8)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transX), sizeof(baseData.transX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transY), sizeof(baseData.transY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transZ), sizeof(baseData.transZ))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotX), sizeof(baseData.rotX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotY), sizeof(baseData.rotY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotZ), sizeof(baseData.rotZ))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.scaleX), sizeof(baseData.scaleX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.scaleY), sizeof(baseData.scaleY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.width), sizeof(baseData.width))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.height), sizeof(baseData.height))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(sectionSize);
	Utility::byteswap_inplace(baseData.transX);
	Utility::byteswap_inplace(baseData.transY);
	Utility::byteswap_inplace(baseData.transZ);
	Utility::byteswap_inplace(baseData.rotX);
	Utility::byteswap_inplace(baseData.rotY);
	Utility::byteswap_inplace(baseData.rotZ);
	Utility::byteswap_inplace(baseData.scaleX);
	Utility::byteswap_inplace(baseData.scaleY);
	Utility::byteswap_inplace(baseData.width);
	Utility::byteswap_inplace(baseData.height);

	std::optional<RGBA> color = readRGBA(bflyt, bflyt.tellg());
	if (!color.has_value()) {
		return FLYTError::REACHED_EOF; //returns nullopt if read fails
	}
	vertexColorTL = color.value();
	color = readRGBA(bflyt, bflyt.tellg());
	if (!color.has_value()) {
		return FLYTError::REACHED_EOF; //returns nullopt if read fails
	}
	vertexColorTR = color.value();
	color = readRGBA(bflyt, bflyt.tellg());
	if (!color.has_value()) {
		return FLYTError::REACHED_EOF; //returns nullopt if read fails
	}
	vertexColorBL = color.value();
	color = readRGBA(bflyt, bflyt.tellg());
	if (!color.has_value()) {
		return FLYTError::REACHED_EOF; //returns nullopt if read fails
	}
	vertexColorBR = color.value();

	if (!bflyt.read(reinterpret_cast<char*>(&matIndex), sizeof(matIndex))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&numCoords), sizeof(numCoords))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(matIndex);

	coords.reserve(numCoords);
	for (uint8_t i = 0; i < numCoords; i++) {
		UVCoords coord;
		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordTL[0]), sizeof(coord.coordTL[0]))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordTL[1]), sizeof(coord.coordTL[1]))) {
			return FLYTError::REACHED_EOF;
		}

		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordTR[0]), sizeof(coord.coordTR[0]))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordTR[1]), sizeof(coord.coordTR[1]))) {
			return FLYTError::REACHED_EOF;
		}

		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordBL[0]), sizeof(coord.coordBL[0]))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordBL[1]), sizeof(coord.coordBL[1]))) {
			return FLYTError::REACHED_EOF;
		}

		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordBR[0]), sizeof(coord.coordBR[0]))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&coord.coordBR[1]), sizeof(coord.coordBR[1]))) {
			return FLYTError::REACHED_EOF;
		}

		Utility::byteswap_inplace(coord.coordTL[0]);
		Utility::byteswap_inplace(coord.coordTL[1]);
		Utility::byteswap_inplace(coord.coordTR[0]);
		Utility::byteswap_inplace(coord.coordTR[1]);
		Utility::byteswap_inplace(coord.coordBL[0]);
		Utility::byteswap_inplace(coord.coordBL[1]);
		Utility::byteswap_inplace(coord.coordBR[0]);
		Utility::byteswap_inplace(coord.coordBR[1]);

		coords.push_back(coord);
	}

	bflyt.seekg(this->offset + sectionSize, std::ios::beg);

	return FLYTError::NONE;
}

FLYTError pic1::save_changes(std::ostream& out) {
	return FLYTError::NONE;
}


prt1::~prt1() {

}

FLYTError prt1::read(std::istream& bflyt, const unsigned int offset) {
	bflyt.seekg(offset, std::ios::beg);
	this->offset = offset;

	if (!bflyt.read(magic, 4)) return FLYTError::REACHED_EOF;
	if (strncmp(magic, "prt1", 4) != 0) {
		return FLYTError::UNKNOWN_SECTION;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.bitFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.originFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.alpha), 1)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.paneMagFlags), 1)) {
		return FLYTError::REACHED_EOF;
	}
	baseData.name.resize(0x18);
	baseData.userInfo.resize(0x8);
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.name[0]), 0x18)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.userInfo[0]), 0x8)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transX), sizeof(baseData.transX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transY), sizeof(baseData.transY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.transZ), sizeof(baseData.transZ))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotX), sizeof(baseData.rotX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotY), sizeof(baseData.rotY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.rotZ), sizeof(baseData.rotZ))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.scaleX), sizeof(baseData.scaleX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.scaleY), sizeof(baseData.scaleY))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.width), sizeof(baseData.width))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&baseData.height), sizeof(baseData.height))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(sectionSize);
	Utility::byteswap_inplace(baseData.transX);
	Utility::byteswap_inplace(baseData.transY);
	Utility::byteswap_inplace(baseData.transZ);
	Utility::byteswap_inplace(baseData.rotX);
	Utility::byteswap_inplace(baseData.rotY);
	Utility::byteswap_inplace(baseData.rotZ);
	Utility::byteswap_inplace(baseData.scaleX);
	Utility::byteswap_inplace(baseData.scaleY);
	Utility::byteswap_inplace(baseData.width);
	Utility::byteswap_inplace(baseData.height);

	if (!bflyt.read(reinterpret_cast<char*>(&propCount), sizeof(propCount))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&magnifyX), sizeof(magnifyX))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&magnifyY), sizeof(magnifyY))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(propCount);
	Utility::byteswap_inplace(magnifyX);
	Utility::byteswap_inplace(magnifyY);

	properties.reserve(propCount);
	for (unsigned int i = 0; i < propCount; i++) {
		partProperty& prop = properties.emplace_back();
		if (!bflyt.read(reinterpret_cast<char*>(&prop.propName), 0x18)) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&prop.usageFlag), 1)) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&prop.basicUsageFlag), 1)) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&prop.matUsageFlag), 1)) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&prop.padding_0x00), 1)) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&prop.propOffset), sizeof(prop.propOffset))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&prop.userDataOffset), sizeof(prop.userDataOffset))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&prop.panelInfoOffset), sizeof(prop.panelInfoOffset))) {
			return FLYTError::REACHED_EOF;
		}

		Utility::byteswap_inplace(prop.propOffset);
		Utility::byteswap_inplace(prop.userDataOffset);
		Utility::byteswap_inplace(prop.panelInfoOffset);

		if (prop.propOffset != 0) {
			bflyt.seekg(this->offset + prop.propOffset, std::ios::beg);

			char propMagic[4];
			if (!bflyt.read(reinterpret_cast<char*>(&propMagic), sizeof(propMagic))) return FLYTError::REACHED_EOF;
			bflyt.seekg(-4, std::ios::cur); //seek back to start of the section

			FLYTError err = FLYTError::NONE;
			if (strncmp(propMagic, "pic1", 4) == 0) {
				prop.prop = std::make_unique<pic1>();
				err = dynamic_cast<pic1*>(prop.prop.value().get())->read(bflyt, this->offset + prop.propOffset);
				if (err != FLYTError::NONE) {
					return err;
				}
			}
			else if (strncmp(propMagic, "txt1", 4) == 0) {
				prop.prop = std::make_unique<txt1>();
				err = dynamic_cast<txt1*>(prop.prop.value().get())->read(bflyt, this->offset + prop.propOffset);
				if (err != FLYTError::NONE) {
					return err;
				}
			}
			else if (strncmp(propMagic, "wnd1", 4) == 0) {
				prop.prop = std::make_unique<wnd1>();
				err = dynamic_cast<wnd1*>(prop.prop.value().get())->read(bflyt, this->offset + prop.propOffset);
				if (err != FLYTError::NONE) {
					return err;
				}
			}
			else if (strncmp(propMagic, "bnd1", 4) == 0) {
				prop.prop = std::make_unique<bnd1>();
				err = dynamic_cast<bnd1*>(prop.prop.value().get())->read(bflyt, this->offset + prop.propOffset);
				if (err != FLYTError::NONE) {
					return err;
				}
			}
			else if (strncmp(propMagic, "prt1", 4) == 0) {
				prop.prop = std::make_unique<prt1>();
				err = dynamic_cast<prt1*>(prop.prop.value().get())->read(bflyt, this->offset + prop.propOffset);
				if (err != FLYTError::NONE) {
					return err;
				}
			}
		}
		else {
			prop.prop = std::nullopt;
		}

		if (prop.userDataOffset != 0) {
			usd1 data;
			data.read(bflyt, this->offset + prop.userDataOffset);
			prop.userData = data;
		}
		else {
			prop.prop = std::nullopt;
		}

		if (prop.panelInfoOffset != 0) {
			std::string data;
			data.resize(52);
			if (!bflyt.read(reinterpret_cast<char*>(&data[0]), 52)) {
				return FLYTError::REACHED_EOF;
			}
			prop.paneInfo = data;
		}
		else {
			prop.paneInfo = std::nullopt;
		}
	}

	if (std::string& name = readNullTerminatedStr(bflyt, bflyt.tellg()); name.empty()) {
		return FLYTError::REACHED_EOF; //empty string means it could not read a character from file
	}
	else {
		lytFilename = name;
	}

	bflyt.seekg(this->offset + sectionSize, std::ios::beg);

	return FLYTError::NONE;
}

FLYTError prt1::save_changes(std::ostream& out) {
	return FLYTError::NONE;
}


FLYTError grp1::read(std::istream& bflyt, const unsigned int offset) {
	bflyt.seekg(offset, std::ios::beg);
	this->offset = offset;

	if (!bflyt.read(magic, 4)) return FLYTError::REACHED_EOF;
	if (strncmp(magic, "grp1", 4) != 0) {
		return FLYTError::UNKNOWN_SECTION;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&sectionSize), sizeof(sectionSize))) {
		return FLYTError::REACHED_EOF;
	}
	groupName.resize(0x18);
	if (!bflyt.read(reinterpret_cast<char*>(&groupName[0]), 0x18)) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&numPanes), sizeof(numPanes))) {
		return FLYTError::REACHED_EOF;
	}
	if (!bflyt.read(reinterpret_cast<char*>(&padding_0x00), sizeof(padding_0x00))) {
		return FLYTError::REACHED_EOF;
	}

	Utility::byteswap_inplace(sectionSize);
	Utility::byteswap_inplace(numPanes);

	for (unsigned int i = 0; i < numPanes; i++) {
		std::string paneName;
		paneName.resize(0x18);
		if (!bflyt.read(reinterpret_cast<char*>(&paneName[0]), 0x18)) {
			return FLYTError::REACHED_EOF;
		}

		paneNames.push_back(paneName);
	}

	char nextSection[4];
	if (!bflyt.read(reinterpret_cast<char*>(&nextSection), sizeof(nextSection))) {
		return FLYTError::REACHED_EOF;
	}
	if (strncmp(nextSection, "grs1", 4) == 0) {
		uint32_t grs1Size;
		if (!bflyt.read(reinterpret_cast<char*>(&grs1Size), sizeof(grs1Size))) {
			return FLYTError::REACHED_EOF;
		}
		Utility::byteswap_inplace(grs1Size);
		if (grs1Size != 0x8) {
			return FLYTError::UNEXPECTED_VALUE;
		}

		if (!bflyt.read(reinterpret_cast<char*>(&nextSection), sizeof(nextSection))) {
			return FLYTError::REACHED_EOF;
		}

		while (strncmp(nextSection, "gre1", 4) != 0) {
			bflyt.seekg(-4, std::ios::cur);

			if (strncmp(nextSection, "grp1", 4) != 0) return FLYTError::UNKNOWN_SECTION; //should never happen

			grp1 child;
			FLYTError err = FLYTError::NONE;

			err = child.read(bflyt, bflyt.tellg());
			if (err != FLYTError::NONE) {
				return err;
			}
			children.push_back(child);

			if (!bflyt.read(reinterpret_cast<char*>(&nextSection), sizeof(nextSection))) {
				return FLYTError::REACHED_EOF;
			}
		}

		uint32_t gre1Size;
		if (!bflyt.read(reinterpret_cast<char*>(&gre1Size), sizeof(gre1Size))) {
			return FLYTError::REACHED_EOF;
		}
		Utility::byteswap_inplace(gre1Size);
		if (gre1Size != 0x8) {
			return FLYTError::UNEXPECTED_VALUE;
		}
	}
	else {
		bflyt.seekg(-4, std::ios::cur);
	}

	return FLYTError::NONE;
}

FLYTError grp1::save_changes(std::ostream& out) {
	return FLYTError::NONE;
}


Pane::Pane() {
	
}

FLYTError Pane::read(std::istream& bflyt, const unsigned int offset) {
	bflyt.seekg(offset, std::ios::beg);
	this->offset = offset;
	userData = std::nullopt;

	if (!bflyt.read(magic, 4)) return FLYTError::REACHED_EOF;
	
	FLYTError err = FLYTError::NONE;
	if (strncmp(magic, "pan1", 4) == 0) {
		pane = std::make_unique<pan1>();
		err = dynamic_cast<pan1*>(pane.get())->read(bflyt, this->offset);
		if (err != FLYTError::NONE) {
			return err;
		}
	}
	else if (strncmp(magic, "bnd1", 4) == 0) {
		pane = std::make_unique<bnd1>();
		err = dynamic_cast<bnd1*>(pane.get())->read(bflyt, this->offset);
		if (err != FLYTError::NONE) {
			return err;
		}
	}
	else if (strncmp(magic, "wnd1", 4) == 0) {
		pane = std::make_unique<wnd1>();
		err = dynamic_cast<wnd1*>(pane.get())->read(bflyt, this->offset);
		if (err != FLYTError::NONE) {
			return err;
		}
	}
	else if (strncmp(magic, "txt1", 4) == 0) {
		pane = std::make_unique<txt1>();
		err = dynamic_cast<txt1*>(pane.get())->read(bflyt, this->offset);
		if (err != FLYTError::NONE) {
			return err;
		}
	}
	else if (strncmp(magic, "pic1", 4) == 0) {
		pane = std::make_unique<pic1>();
		err = dynamic_cast<pic1*>(pane.get())->read(bflyt, this->offset);
		if (err != FLYTError::NONE) {
			return err;
		}
	}
	else if (strncmp(magic, "prt1", 4) == 0) {
		pane = std::make_unique<prt1>();
		err = dynamic_cast<prt1*>(pane.get())->read(bflyt, this->offset);
		if (err != FLYTError::NONE) {
			return err;
		}
	}
	else {
		return FLYTError::UNKNOWN_SECTION;
	}

	char nextSection[4];
	if (!bflyt.read(reinterpret_cast<char*>(&nextSection), sizeof(nextSection))) {
		return FLYTError::REACHED_EOF;
	}
	if (strncmp(nextSection, "pas1", 4) == 0) {
		uint32_t pas1Size;
		if (!bflyt.read(reinterpret_cast<char*>(&pas1Size), sizeof(pas1Size))) {
			return FLYTError::REACHED_EOF;
		}
		Utility::byteswap_inplace(pas1Size);
		if (pas1Size != 0x8) {
			return FLYTError::UNEXPECTED_VALUE;
		}

		if (!bflyt.read(reinterpret_cast<char*>(&nextSection), sizeof(nextSection))) {
			return FLYTError::REACHED_EOF;
		}

		while (strncmp(nextSection, "pae1", 4) != 0) {
			if (strncmp(nextSection, "usd1", 4) == 0) {
				bflyt.seekg(-4, std::ios::cur);

				usd1& data = userData.emplace();
				err = data.read(bflyt, bflyt.tellg());
				if (err != FLYTError::NONE) {
					return err;
				}
			}
			else {
				bflyt.seekg(-4, std::ios::cur);

				Pane& child = children.emplace_back(); //do this because unique_ptr can't be copied
				FLYTError err = FLYTError::NONE;

				err = child.read(bflyt, bflyt.tellg());
				if (err != FLYTError::NONE) {
					return err;
				}
			}
			
			if (!bflyt.read(reinterpret_cast<char*>(&nextSection), sizeof(nextSection))) {
				return FLYTError::REACHED_EOF;
			}
		}

		uint32_t pae1Size;
		if (!bflyt.read(reinterpret_cast<char*>(&pae1Size), sizeof(pae1Size))) {
			return FLYTError::REACHED_EOF;
		}
		Utility::byteswap_inplace(pae1Size);
		if (pae1Size != 0x8) {
			return FLYTError::UNEXPECTED_VALUE;
		}
	}
	else {
		bflyt.seekg(-4, std::ios::cur);
	}

	return FLYTError::NONE;
}

FLYTError Pane::save_changes(std::ostream& out) {
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

	FLYTFile::FLYTFile() {

	}

	void FLYTFile::initNew() {
		memcpy(header.magicFLYT, "FLYT", 4);
		header.byteOrderMarker = 0xFEFF;
		header.headerSize_0x14 = 0x14;
		header.version_0x0202 = 0x0202;
		header.fileSize = 0;
		header.numSections = 0;

		memcpy(LYT1.magicLYT1, "LYT1", 4);
		LYT1.sectionSize = 0;
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

		//init rootpane and rootgroup
		return;
	}

	FLYTFile FLYTFile::createNew(const std::string& filename) {
		FLYTFile newFLYT{};
		newFLYT.initNew();
		return newFLYT;
	}

	FLYTError FLYTFile::readLYT1(std::istream& bflyt) {
		if (!bflyt.read(LYT1.magicLYT1, 4)) return FLYTError::REACHED_EOF;
		if (strncmp(LYT1.magicLYT1, "lyt1", 4) != 0) {
			return FLYTError::UNKNOWN_SECTION;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&LYT1.sectionSize), sizeof(LYT1.sectionSize))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&LYT1.drawCentered), sizeof(LYT1.drawCentered))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&LYT1.padding_0x00), sizeof(LYT1.padding_0x00))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&LYT1.width), sizeof(LYT1.width))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&LYT1.height), sizeof(LYT1.height))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&LYT1.maxPartWidth), sizeof(LYT1.maxPartWidth))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&LYT1.maxPartHeight), sizeof(LYT1.maxPartHeight))) {
			return FLYTError::REACHED_EOF;
		}
		
		if (std::string& name = readNullTerminatedStr(bflyt, bflyt.tellg()); name.empty()) {
			return FLYTError::REACHED_EOF; //empty string means it could not read a character from file
		}
		else {
			LYT1.layoutName = name; //read the name properly
		}

		Utility::byteswap_inplace(LYT1.sectionSize);
		Utility::byteswap_inplace(LYT1.width);
		Utility::byteswap_inplace(LYT1.height);
		Utility::byteswap_inplace(LYT1.maxPartWidth);
		Utility::byteswap_inplace(LYT1.maxPartHeight);

		bflyt.seekg(0x14 + LYT1.sectionSize, std::ios::beg);

		return FLYTError::NONE;
	}

	FLYTError FLYTFile::readTextures(std::istream& bflyt) {
		txl1& textureList = textures.emplace();

		textureList.offset = bflyt.tellg();

		if (!bflyt.read(textureList.magicTXL1, 4)) return FLYTError::REACHED_EOF;
		if (strncmp(textureList.magicTXL1, "txl1", 4) != 0) {
			return FLYTError::UNKNOWN_SECTION;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&textureList.sectionSize), sizeof(textureList.sectionSize))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&textureList.numTextures), sizeof(textureList.numTextures))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&textureList.padding_0x00), sizeof(textureList.padding_0x00))) {
			return FLYTError::REACHED_EOF;
		}

		Utility::byteswap_inplace(textureList.sectionSize);
		Utility::byteswap_inplace(textureList.numTextures);

		for (unsigned int i = 0; i < textureList.numTextures; i++) {
			uint32_t nameOffset = 0;
			if (!bflyt.read(reinterpret_cast<char*>(&nameOffset), sizeof(nameOffset))) {
				return FLYTError::REACHED_EOF;
			}

			Utility::byteswap_inplace(nameOffset);
			textureList.texStrOffsets.push_back(nameOffset);
		}

		for (uint32_t offset : textureList.texStrOffsets) {
			bflyt.seekg(textureList.offset + 0xC + offset);
			if (std::string& name = readNullTerminatedStr(bflyt, bflyt.tellg()); name.empty()) {
				return FLYTError::REACHED_EOF; //empty string means it could not read a character from file
			}
			else {
				textureList.texNames.push_back(name);
			}
		}

		bflyt.seekg(textureList.offset + textureList.sectionSize, std::ios::beg);

		return FLYTError::NONE;
	}

	FLYTError FLYTFile::readFonts(std::istream& bflyt) {
		fnl1& fontList = fonts.emplace();

		fontList.offset = bflyt.tellg();

		if (!bflyt.read(fontList.magicFNL1, 4)) return FLYTError::REACHED_EOF;
		if (strncmp(fontList.magicFNL1, "fnl1", 4) != 0) {
			return FLYTError::UNKNOWN_SECTION;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&fontList.sectionSize), sizeof(fontList.sectionSize))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&fontList.numFonts), sizeof(fontList.numFonts))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&fontList.padding_0x00), sizeof(fontList.padding_0x00))) {
			return FLYTError::REACHED_EOF;
		}

		Utility::byteswap_inplace(fontList.sectionSize);
		Utility::byteswap_inplace(fontList.numFonts);

		for (unsigned int i = 0; i < fontList.numFonts; i++) {
			uint32_t nameOffset = 0;
			if (!bflyt.read(reinterpret_cast<char*>(&nameOffset), sizeof(nameOffset))) {
				return FLYTError::REACHED_EOF;
			}

			Utility::byteswap_inplace(nameOffset);
			fontList.fontStrOffsets.push_back(nameOffset);
		}

		for (uint32_t offset : fontList.fontStrOffsets) {
			bflyt.seekg(fontList.offset + 0xC + offset);
			if (std::string& name = readNullTerminatedStr(bflyt, bflyt.tellg()); name.empty()) {
				return FLYTError::REACHED_EOF; //empty string means it could not read a character from file
			}
			else {
				fontList.fontNames.push_back(name);
			}
		}

		bflyt.seekg(fontList.offset + fontList.sectionSize, std::ios::beg);

		return FLYTError::NONE;
	}

	FLYTError FLYTFile::readMaterials(std::istream& bflyt) {
		mat1& matList = materials.emplace();

		matList.offset = bflyt.tellg();

		if (!bflyt.read(matList.magicMAT1, 4)) return FLYTError::REACHED_EOF;
		if (strncmp(matList.magicMAT1, "mat1", 4) != 0) {
			return FLYTError::UNKNOWN_SECTION;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&matList.sectionSize), sizeof(matList.sectionSize))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&matList.numMats), sizeof(matList.numMats))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&matList.padding_0x00), sizeof(matList.padding_0x00))) {
			return FLYTError::REACHED_EOF;
		}

		Utility::byteswap_inplace(matList.sectionSize);
		Utility::byteswap_inplace(matList.numMats);

		for (unsigned int i = 0; i < matList.numMats; i++) {
			uint32_t matOffset = 0;
			if (!bflyt.read(reinterpret_cast<char*>(&matOffset), sizeof(matOffset))) {
				return FLYTError::REACHED_EOF;
			}

			Utility::byteswap_inplace(matOffset);
			matList.matOffsets.push_back(matOffset);
		}

		for (uint32_t offset : matList.matOffsets) {
			material mat;
			FLYTError err = FLYTError::NONE;
			err = readMaterial(bflyt, matList.offset + offset, mat);
			if (err != FLYTError::NONE) {
				return err;
			}
			matList.materials.push_back(mat);
		}

		bflyt.seekg(matList.offset + matList.sectionSize, std::ios::beg);

		return FLYTError::NONE;
	}

	FLYTError FLYTFile::loadFromBinary(std::istream& bflyt) {
		if (!bflyt.read(header.magicFLYT, 4)) return FLYTError::REACHED_EOF;
		if (strncmp(header.magicFLYT, "FLYT", 4) != 0) {
			return FLYTError::NOT_FLYT;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&header.byteOrderMarker), sizeof(header.byteOrderMarker))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&header.headerSize_0x14), sizeof(header.headerSize_0x14))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&header.version_0x0202), sizeof(header.version_0x0202))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&header.fileSize), sizeof(header.fileSize))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&header.numSections), sizeof(header.numSections))) {
			return FLYTError::REACHED_EOF;
		}
		if (!bflyt.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00))) {
			return FLYTError::REACHED_EOF;
		}

		Utility::byteswap_inplace(header.byteOrderMarker);
		Utility::byteswap_inplace(header.headerSize_0x14);
		Utility::byteswap_inplace(header.version_0x0202);
		Utility::byteswap_inplace(header.fileSize);
		Utility::byteswap_inplace(header.numSections);

		if (header.byteOrderMarker != 0xFEFF) return FLYTError::UNEXPECTED_VALUE;
		if (header.headerSize_0x14 != 0x14) return FLYTError::UNEXPECTED_VALUE;
		if (header.version_0x0202 != 0x02020000) return FLYTError::UNKNOWN_VERSION;
		if (header.padding_0x00[0] != 0x00 || header.padding_0x00[1] != 0x00) return FLYTError::UNEXPECTED_VALUE;

		FLYTError err = FLYTError::NONE;
		err = readLYT1(bflyt);
		if (err != FLYTError::NONE) {
			return err;
		}

		std::string magic(4, '\0');
		if (!bflyt.read(reinterpret_cast<char*>(&magic[0]), 4)) return FLYTError::REACHED_EOF;
		bflyt.seekg(-4, std::ios::cur); //seek back to start of the section

		if (magic == "txl1") {
			err = readTextures(bflyt);
			if (err != FLYTError::NONE) {
				return err;
			}

			if (!bflyt.read(reinterpret_cast<char*>(&magic[0]), 4)) return FLYTError::REACHED_EOF;
			bflyt.seekg(-4, std::ios::cur); //seek back to start of the section
		}

		if (magic == "fnl1") {
			err = readFonts(bflyt);
			if (err != FLYTError::NONE) {
				return err;
			}

			if (!bflyt.read(reinterpret_cast<char*>(&magic[0]), 4)) return FLYTError::REACHED_EOF;
			bflyt.seekg(-4, std::ios::cur); //seek back to start of the section
		}

		if (magic == "mat1") {
			err = readMaterials(bflyt);
			if (err != FLYTError::NONE) {
				return err;
			}

			if (!bflyt.read(reinterpret_cast<char*>(&magic[0]), 4)) return FLYTError::REACHED_EOF;
			bflyt.seekg(-4, std::ios::cur); //seek back to start of the section
		}

		while (sections.count(magic) > 0) {
			if (magic == "usd1") {
				userData.emplace();
				err = userData.value().read(bflyt, bflyt.tellg());
				if (err != FLYTError::NONE) {
					return err;
				}
			}
			else if (magic == "cnt1") {
				container.emplace();
				err = container.value().read(bflyt, bflyt.tellg());
				if (err != FLYTError::NONE) {
					return err;
				}
			}
			else if (magic == "grp1") {
				err = rootGroup.read(bflyt, bflyt.tellg());
				if (err != FLYTError::NONE) {
					return err;
				}
			}
			else {
				err = rootPane.read(bflyt, bflyt.tellg());
				if (err != FLYTError::NONE) {
					return err;
				}
			}

			if (!bflyt.read(reinterpret_cast<char*>(&magic[0]), 4)) {
				break; //tried to read past the last section
			}
			bflyt.seekg(-4, std::ios::cur); //seek back to start of the section
		}

		return FLYTError::NONE;
	}

	FLYTError FLYTFile::loadFromFile(const std::string& filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			return FLYTError::COULD_NOT_OPEN;
		}
		return loadFromBinary(file);
	}

	FLYTError FLYTFile::writeToStream(std::ostream& out) {
		return FLYTError::NONE;
	}

	FLYTError FLYTFile::writeToFile(const std::string& outFilePath) {
		std::ofstream outFile(outFilePath, std::ios::binary);
		if (!outFile.is_open()) {
			return FLYTError::COULD_NOT_OPEN;
		}
		return writeToStream(outFile);
	}
}
