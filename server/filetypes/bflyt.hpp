#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>

#include "../utility/byteswap.hpp"
#include "../utility/macros.hpp"



struct RGBA { //might want to move into a generic type macros hpp
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t A;
};

struct UVCoords {
	float coordTL[2]; //0 is X, 1 is Y
	float coordTR[2]; //0 is X, 1 is Y
	float coordBL[2]; //0 is X, 1 is Y
	float coordBR[2]; //0 is X, 1 is Y
};



enum struct FLYTError {
	NONE = 0,
	COULD_NOT_OPEN,
	NOT_FLYT,
	UNKNOWN_VERSION,
	UNEXPECTED_VALUE,
	UNKNOWN_SECTION,
	REACHED_EOF,
	UNKNOWN
};

struct FLYTHeader {
	char magicFLYT[4];
	uint16_t byteOrderMarker;
	uint16_t headerSize_0x14;
	uint32_t version_0x0202;
	uint32_t fileSize;
	uint16_t numSections;
	uint8_t padding_0x00[2];
};

struct lyt1 {
	char magicLYT1[4];
	uint32_t sectionSize;
	uint8_t drawCentered; //drawn from center
	uint8_t padding_0x00[3];
	float width;
	float height;
	float maxPartWidth;
	float maxPartHeight;
	std::string layoutName;
};

struct txl1 {
	char magicTXL1[4];
	uint32_t sectionSize;
	uint16_t numTextures;
	uint8_t padding_0x00[2];
	std::vector<uint32_t> texStrOffsets;
	std::vector<std::string> texNames;
};

struct fnl1 {
	char magicFNL1[4];
	uint32_t sectionSize;
	uint16_t fontNum;
	uint8_t padding_0x00[2];
	std::vector<uint32_t> fontStrOffsets;
	std::vector<std::string> fontNames;
};

struct mat1 {
	char magicMAT1[4];
	uint32_t sectionSize;
	uint16_t numMat;
	std::vector<uint32_t> offsets;
	std::vector<std::string> matData; //fields don't really need editing, just need to read data (theyre also quite complex)
};

struct paneData {
	uint8_t bitFlags;
	uint8_t originFlags;
	uint8_t alpha;
	uint8_t scale;
	std::string name; //always 32 bytes, padded with null (hence no char array)
	float transX;
	float transY;
	float transZ;
	float rotX;
	float rotY;
	float rotZ;
	float scaleX;
	float scaleY;
	float width;
	float height;
};

class PaneBase {
public:
	int offset;

	char magic[4];
	uint32_t sectionSize;
	paneData baseData;

	virtual FLYTError read(std::istream& bflyt, int offset) = 0;
	virtual FLYTError save_changes(std::ostream& out) = 0;
};

class pan1 : public PaneBase {
	magic = 'pan1';

	FLYTError read(std::istream& bflyt, int offset);
	FLYTError save_changes(std::ostream& out);
};

class bnd1 : public PaneBase {
	magic = 'bnd1';

	FLYTError read(std::istream& bflyt, int offset);
	FLYTError save_changes(std::ostream& out);
};

struct windowContent {
	RGBA vertexColTL;
	RGBA vertexColTR;
	RGBA vertexColBL;
	RGBA vertexColBR;
	uint16_t matIndex;
	uint8_t numCoords;
	uint8_t padding;
	std::vector<UVCoords> coords;
};

struct windowFrame {
	uint16_t matIndex;
	uint8_t texFlip;
	uint8_t padding;
};

class wnd1 : public PaneBase {
	magic = 'wnd1';
	int16_t leftStretch;
	int16_t rightStretch;
	int16_t topStretch;
	int16_t bottomStretch;
	int16_t frameSizeLeft;
	int16_t frameSizeRight;
	int16_t frameSizeTop;
	int16_t frameSizeBottom;
	uint8_t frameNum;
	uint8_t bitFlags;
	uint8_t padding[2];
	uint32_t contentOffset;
	uint32_t frameTableOffset;

	windowContent content;
	std::vector<uint32_t> frameTable; //table of frame offsets
	windowFrame frames;

	FLYTError read(std::istream& bflyt, int offset);
	FLYTError save_changes(std::ostream& out);
};

struct perCharTransform {
	float curveTimeOffset;
	float animCurveWidth;
	uint8_t loopType;
	uint8_t verticalOrigin;
	bool hasAnimInfo;
	uint8_t padding;
};

class txt1 : public PaneBase {
	magic = 'txt1';

	uint16_t texLen;
	uint16_t restrictedLen;
	uint16_t matIndex;
	uint16_t fontIndex;
	uint8_t textAlignment;
	uint8_t lineAlignment;
	uint8_t bitflags;
	uint8_t padding;
	float italicTilt;
	uint32_t textOffset;
	RGBA fontColorTop;
	RGBA fontColorBottom;
	float fontSizeX;
	float fontSizeY;
	float charSpace;
	float lineSpace;
	uint32_t nameOffset;
	float shadowPosX;
	float shadowPosY;
	float shadowSizeX;
	float shadowSizeY;
	RGBA shadorColorTop;
	RGBA shadorColorBottom;
	float shadowItalicTilt;
	std::optional<uint32_t> charTransformOffset;
	
	std::optional<perCharTransform> charTransform;

	FLYTError read(std::istream& bflyt, int offset);
	FLYTError save_changes(std::ostream& out);
};

class pic1 : public PaneBase {
	magic = 'pic1';

	RGBA vertexColorTL;
	RGBA vertexColorTR;
	RGBA vertexColorBL;
	RGBA vertexColorBR;
	uint16_t matIndex;
	uint8_t numCoords;
	uint8_t padding;
	std::vector<UVCoords> coords;

	FLYTError read(std::istream& bflyt, int offset);
	FLYTError save_changes(std::ostream& out);
};

struct partProperty {
	std::string propName;
	uint8_t usageFlag;
	uint8_t basicUsageFlag;
	uint8_t matUsageFlag;
	uint8_t padding;
	uint32_t propOffset;
	uint32_t userDataOffset;
	uint32_t panelInfoOffset;

	std::optional<PaneBase*> prop;
	std::optional<usd1> userData;
	std::optional<std::string> paneInfo;
};

class prt1 : public PaneBase {
	magic = 'prt1';

	uint32_t propCount;
	float scaleX;
	float scaleY;
	std::vector<UVCoords> coords;
	std::vector<partProperty> properties;
	std::string lytFilename;

	FLYTError read(std::istream& bflyt, int offset);
	FLYTError save_changes(std::ostream& out);
};

class grp1 {
	char magic[4] = 'grp1';
	uint32_t sectionSize;
	std::string groupName;
	uint16_t numChildren;
	uint8_t[2] padding;
	std::vector<std::string> paneNames; //names of panes in group

	std::vector<grp1> children; //child groups
};

class cnt1 {
	char magic[4] = 'grp1';
	uint32_t sectionSize;
	uint32_t paneNamesOffset;
	uint16_t paneCount;
	uint16_t animCount;
	std::string name;
};

struct userDataEntry {
	//some stuff here i dont have the brain to handle rn https://github.com/KillzXGaming/Switch-Toolbox/blob/12dfbaadafb1ebcd2e07d239361039a8d05df3f7/File_Format_Library/FileFormats/Layout/Common.cs#L896 https://github.com/KillzXGaming/Switch-Toolbox/blob/12dfbaadafb1ebcd2e07d239361039a8d05df3f7/File_Format_Library/FileFormats/Layout/CAFE/USD1.cs#L10
};

class usd1 {
	char magic[4] = 'usd1';
	uint32_t sectionSize;
	uint16_t numEntries;
	std::vector<userDataEntry> entries;
};

class Pane {
	PaneBase* pane;
	std::vector<Pane> children;

	FLYTError read(std::istream& bflyt, int offset);
	FLYTError save_changes(std::ostream& out);
	//idk what else i need
};

namespace FileTypes {

	const char* FLYTErrorGetName(FLYTError err);

	class FLYTFile {
	public:
		FLYTHeader header;
		lyt1 LYT1;
		std::optional<txl1> textures; //Some layouts don't use these
		std::optional<fnl1> fonts; //Some layouts don't use these
		std::optional<mat1> materials; //Some layouts don't use these
		std::optional<cnt1> container; //Some layouts don't use these
		std::optional<usd1> userData; //Some layouts don't use these

		Pane rootPane; //all panes are children of root
		grp1 rootGroup;

		FLYTFile();
		static FLYTFile createNew(const std::string& filename);
		FLYTError loadFromBinary(std::istream& bflyt);
		FLYTError loadFromFile(const std::string& filePath);
		FLYTError writeToStream(std::ostream& out);
		FLYTError writeToFile(const std::string& outFilePath);
	};
}
