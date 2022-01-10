#pragma once

#include <fstream>
#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <variant>

#include "../utility/byteswap.hpp"
//#include "../utility/macros.hpp"


enum struct WrapMode {
	NEAR_CLAMP = 0,
	NEAR_REPEAT,
	NEAR_MIRROR,
	GX2_MIRROR_ONCE,
	CLAMP,
	REPEAT,
	MIRROR,
	GX2_MIRROR_ONCE_BORDER
};

enum struct TexGenMatrixType {
	MATRIX_2x4 = 0
};

enum struct TexGenType {
	TEX_COORD_0 = 0,
	TEX_COORD_1,
	TEX_COORD_2,
	ORTHO_PROJECT,
	PANE_PROJECT,
	PERSPECTIVE_PROJECT
};

enum struct tevMode {
	REPLACE = 0,
	MODULATE,
	ADD,
	EXCLUDE,
	INTERPOLATE,
	SUBTRACT,
	DODGE,
	BURN,
	OVERLAY,
	INDIRECT,
	BLEND_INDIRECT,
	EACH_INDIRECT
};

enum struct alphaCompareType {
	NEVER = 0,
	LESS,
	LESS_OR_EQUAL,
	EQUAL,
	NOT_EQUAL,
	GREATER_OR_EQUAL,
	GREATER,
	ALWAYS
};

enum struct GX2BlendOp {
	DISABLE = 0,
	ADD,
	SUBTRACT,
	REVERSE_SUBTRACT,
	SELECT_MIN,
	SELECT_MAX
};

enum struct GX2BlendFactor {
	FACTOR_0 = 0,
	FACTOR_1,
	DEST_COLOR,
	DEST_INV_COLOR,
	SOURCE_ALPHA,
	SOURCE_INV_ALPHA,
	DEST_ALPHA,
	DEST_INV_ALPHA,
	SOURCE_COLOR,
	SOURCE_INV_COLOR
};

enum struct GX2LogicOp {
	DISABLE = 0,
	NO_OP,
	CLEAR,
	SET,
	COPY,
	INV_COPY,
	INV,
	AND,
	NAND,
	OR,
	NOR,
	XOR,
	EQUIV,
	REV_AND,
	INV_AD,
	REV_OR,
	INV_OR
};

enum struct UserDataType : uint8_t {
	STRING = 0,
	INT,
	FLOAT,
	STRUCT //only seen in Switch files?
};



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

struct texRef {
	uint16_t nameIndex;
	
	WrapMode wrapModeU;
	WrapMode wrapModeV;
};

struct texTransform {
	float translation[2];
	float rotation;
	float scale[2];
};

struct texCoordGen {
	TexGenMatrixType matrix;
	TexGenType source;
	uint8_t unk[6];
};

struct tevStage {
	tevMode colorMode;
	tevMode alphaMode;
	uint8_t padding_0x00[2];
};

struct alphaCompare {
	alphaCompareType compareMode;
	uint8_t unk[3];
	float value;
};

struct blendMode {
	GX2BlendOp blendOp;
	GX2BlendFactor sourceFactor;
	GX2BlendFactor destFactor;
	GX2LogicOp logicOp;
};

struct indirectParam {
	float rotation;
	float scaleX;
	float scaleY;
};

struct projectionMap {
	float posX;
	float posY;
	float scaleX;
	float scaleY;
	uint8_t flags;
	uint8_t unk[3];
};

struct fontShadowParameter {
	RGBA blackColor;
	RGBA whiteColor;
};



struct material {
	std::string name;

	RGBA blackColor;
	RGBA whiteColor;

	uint32_t texCount : 2;
	uint32_t mtxCount : 2;
	uint32_t texCoordGenCount : 2;
	uint32_t tevStageCount : 3;
	uint32_t enableAlphaCompare : 1;
	uint32_t enableBlend : 1;
	uint32_t textureOnly : 1;
	uint32_t blendLogic : 1;
	uint32_t indParams : 1;
	uint32_t projMapCount : 3;
	uint32_t fontShadowParams : 1;
	uint32_t alphaInterpolation : 1;

	std::vector<texRef> texRefs = {};
	std::vector<texTransform> texTransforms = {};
	std::vector<texCoordGen> texCoordGens = {};
	std::vector<tevStage> tevStages = {};
	std::optional<alphaCompare> alphaComparison = std::nullopt;
	std::optional<blendMode> blendingMode = std::nullopt;
	std::optional<blendMode> blendModeLogic = std::nullopt;
	std::optional<indirectParam> indParameter = std::nullopt;
	std::vector<projectionMap> projectionMaps = {};
	std::optional<fontShadowParameter> fontShadowParam = std::nullopt;
};



enum struct FLYTError {
	NONE = 0,
	COULD_NOT_OPEN,
	NOT_FLYT,
	UNKNOWN_VERSION,
	UNEXPECTED_VALUE,
	UNKNOWN_SECTION,
	REACHED_EOF,
	UNKNOWN,
	COUNT
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
	unsigned int offset;

	char magicTXL1[4];
	uint32_t sectionSize;
	uint16_t numTextures;
	uint8_t padding_0x00[2];
	std::vector<uint32_t> texStrOffsets;
	std::vector<std::string> texNames;
};

struct fnl1 {
	unsigned int offset;

	char magicFNL1[4];
	uint32_t sectionSize;
	uint16_t numFonts;
	uint8_t padding_0x00[2];
	std::vector<uint32_t> fontStrOffsets;
	std::vector<std::string> fontNames;
};

struct mat1 {
	unsigned int offset;

	char magicMAT1[4];
	uint32_t sectionSize;
	uint16_t numMats;
	uint8_t padding_0x00[2];
	std::vector<uint32_t> matOffsets;
	std::vector<material> materials; //fields don't really need editing, just need to read data (theyre also quite complex)
};

class cnt1 {
public:
	unsigned int offset;

	char magic[4] = { 'c', 'n', 't', '1' };
	uint32_t sectionSize;
	uint32_t paneNamesOffset;
	uint16_t paneCount;
	uint16_t animCount;
	std::string name;

	std::vector<std::string> paneNames;

	unsigned int animNameTableOffset; //offset in section, not file
	std::vector<uint32_t> animNameOffsets;
	std::vector<std::string> animNames;

	FLYTError read(std::istream& bflyt, const unsigned int offset);
	FLYTError save_changes(std::ostream& out);
};

struct userDataEntry {
	uint32_t nameOffset;
	uint32_t dataOffset;
	uint16_t dataLen;
	UserDataType dataType;
	uint8_t unk;

	std::string name;
	std::variant<std::string, std::vector<int32_t>, std::vector<float>> data;
};

class usd1 {
public:
	unsigned int offset;

	char magic[4] = { 'u', 's', 'd', '1' };
	uint32_t sectionSize;
	uint16_t numEntries;
	uint8_t padding_0x00[2];
	std::vector<userDataEntry> entries;

	FLYTError read(std::istream& bflyt, const unsigned int offset);
	FLYTError save_changes(std::ostream& out);
};

struct paneData {
	uint8_t bitFlags;
	uint8_t originFlags;
	uint8_t alpha;
	uint8_t paneMagFlags;
	std::string name; //always 32 bytes, padded with null (hence no char array)
	std::string userInfo;
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
	unsigned int offset;

	char magic[4];
	uint32_t sectionSize;
	paneData baseData;

	virtual ~PaneBase();

	virtual FLYTError read(std::istream& bflyt, const unsigned int offset) = 0;
	virtual FLYTError save_changes(std::ostream& out) = 0;
};

class pan1 : public PaneBase {
public:

	~pan1();

	FLYTError read(std::istream& bflyt, const unsigned int offset);
	FLYTError save_changes(std::ostream& out);
};

class bnd1 : public PaneBase {
public:

	~bnd1();

	FLYTError read(std::istream& bflyt, const unsigned int offset);
	FLYTError save_changes(std::ostream& out);
};

struct windowContent {
	RGBA vertexColTL;
	RGBA vertexColTR;
	RGBA vertexColBL;
	RGBA vertexColBR;
	uint16_t matIndex;
	uint8_t numCoords;
	uint8_t padding_0x00;
	std::vector<UVCoords> coords;
};

struct windowFrame {
	uint16_t matIndex;
	uint8_t texFlip;
	uint8_t padding_0x00;
};

class wnd1 : public PaneBase {
public:
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
	uint8_t padding_0x00[2];
	uint32_t contentOffset;
	uint32_t frameTableOffset;

	windowContent content;
	std::vector<uint32_t> frameTable; //table of frame offsets
	std::vector<windowFrame> frames;

	~wnd1();

	FLYTError read(std::istream& bflyt, const unsigned int offset);
	FLYTError save_changes(std::ostream& out);
};

struct perCharTransform {
	float curveTimeOffset;
	float animCurveWidth;
	uint8_t loopType;
	uint8_t verticalOrigin;
	bool hasAnimInfo;
	uint8_t padding_0x00;
};

class txt1 : public PaneBase {
public:
	uint16_t texLen;
	uint16_t restrictedLen;
	uint16_t matIndex;
	uint16_t fontIndex;
	uint8_t textAlignment;
	uint8_t lineAlignment;
	uint8_t bitflags;
	uint8_t padding_0x00;
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
	RGBA shadowColorTop;
	RGBA shadowColorBottom;
	float shadowItalicTilt;
	std::optional<uint32_t> charTransformOffset;

	std::u16string text;
	std::string textBoxName;
	std::optional<perCharTransform> charTransform;

	~txt1();

	FLYTError read(std::istream& bflyt, const unsigned int offset);
	FLYTError save_changes(std::ostream& out);
};

class pic1 : public PaneBase {
public:
	RGBA vertexColorTL;
	RGBA vertexColorTR;
	RGBA vertexColorBL;
	RGBA vertexColorBR;
	uint16_t matIndex;
	uint8_t numCoords;
	uint8_t padding_0x00;
	std::vector<UVCoords> coords;

	~pic1();

	FLYTError read(std::istream& bflyt, const unsigned int offset);
	FLYTError save_changes(std::ostream& out);
};

struct partProperty {
	std::string propName;
	uint8_t usageFlag;
	uint8_t basicUsageFlag;
	uint8_t matUsageFlag;
	uint8_t padding_0x00;
	uint32_t propOffset;
	uint32_t userDataOffset;
	uint32_t panelInfoOffset;

	std::optional<std::unique_ptr<PaneBase>> prop; //do something here to deal with unique_ptr
	std::optional<usd1> userData;
	std::optional<std::string> paneInfo;
};

class prt1 : public PaneBase {
public:
	uint32_t propCount;
	float magnifyX;
	float magnifyY;
	std::vector<partProperty> properties;
	std::string lytFilename;

	~prt1();

	FLYTError read(std::istream& bflyt, const unsigned int offset);
	FLYTError save_changes(std::ostream& out);
};

class grp1 {
public:
	unsigned int offset;

	char magic[4] = {'g', 'r', 'p', '1'};
	uint32_t sectionSize;
	std::string groupName;
	uint16_t numPanes;
	uint8_t padding_0x00[2];
	std::vector<std::string> paneNames; //names of panes in group

	std::vector<grp1> children;

	FLYTError read(std::istream& bflyt, const unsigned int offset);
	FLYTError save_changes(std::ostream& out);
};

class Pane {
public:
	unsigned int offset;

	char magic[4];
	std::unique_ptr<PaneBase> pane;
	std::optional<usd1> userData;
	std::vector<Pane> children;

	Pane();

	FLYTError read(std::istream& bflyt, const unsigned int offset);
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
	private:
		void initNew();
		FLYTError readLYT1(std::istream& bflyt);
		FLYTError readTextures(std::istream& bflyt);
		FLYTError readFonts(std::istream& bflyt);
		FLYTError readMaterials(std::istream& bflyt);
	};
}
