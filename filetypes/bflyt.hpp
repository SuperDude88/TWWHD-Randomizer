//Format is a part of NintendoWare::lyt (a UI library)
//BFLYT files store a pane tree to create a 2D layout
//They are used in conjunction with BFLAN and BFLIM files

#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <optional>
#include <variant>
#include <memory>

#include <utility/common.hpp>
#include <filetypes/baseFiletype.hpp>



enum struct [[nodiscard]] FLYTError {
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

namespace NintendoWare::Layout {
	enum struct WrapMode : uint8_t {
		NEAR_CLAMP = 0,
		NEAR_REPEAT,
		NEAR_MIRROR,
		GX2_MIRROR_ONCE,
		CLAMP,
		REPEAT,
		MIRROR,
		GX2_MIRROR_ONCE_BORDER
	};

	enum struct TexGenMatrixType : uint8_t {
		MATRIX_2x4 = 0
	};

	enum struct TexGenType : uint8_t {
		TEX_COORD_0 = 0,
		TEX_COORD_1,
		TEX_COORD_2,
		ORTHO_PROJECT,
		PANE_PROJECT,
		PERSPECTIVE_PROJECT
	};

	enum struct tevMode : uint8_t {
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

	enum struct alphaCompareType : uint8_t {
		NEVER = 0,
		LESS,
		LESS_OR_EQUAL,
		EQUAL,
		NOT_EQUAL,
		GREATER_OR_EQUAL,
		GREATER,
		ALWAYS
	};

	enum struct GX2BlendOp : uint8_t {
		DISABLE = 0,
		ADD,
		SUBTRACT,
		REVERSE_SUBTRACT,
		SELECT_MIN,
		SELECT_MAX
	};

	enum struct GX2BlendFactor : uint8_t {
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

	enum struct GX2LogicOp : uint8_t {
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


	struct UVCoords {
		vec2<float> coordTL;
		vec2<float> coordTR;
		vec2<float> coordBL;
		vec2<float> coordBR;
	};

	struct texRef {
		uint16_t nameIndex;

		WrapMode wrapModeU;
		WrapMode wrapModeV;
	};

	struct texTransform {
		vec2<float> translation;
		float rotation;
		vec2<float> scale;
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
		RGBA8 blackColor;
		RGBA8 whiteColor;
	};

	class lyt1 {
	private:
		char magicLYT1[4];
		uint32_t sectionSize;
		uint8_t padding_0x00[3];

	public:
		uint8_t drawCentered;
		float width;
		float height;
		float maxPartWidth;
		float maxPartHeight;
		std::string layoutName;

		FLYTError read(std::istream& bflyt);
		FLYTError save_changes(std::ostream& out);
	};

	class txl1 {
	private:
		char magicTXL1[4];
		uint32_t sectionSize;
		uint16_t numTextures;
		uint8_t padding_0x00[2];
		std::vector<uint32_t> texStrOffsets;

	public:
		std::vector<std::string> texNames;

		FLYTError read(std::istream& bflyt);
		FLYTError save_changes(std::ostream& out);
	};

	class fnl1 {
	private:
		char magicFNL1[4];
		uint32_t sectionSize;
		uint16_t numFonts;
		uint8_t padding_0x00[2];
		std::vector<uint32_t> fontStrOffsets;

	public:
		std::vector<std::string> fontNames;

		FLYTError read(std::istream& bflyt);
		FLYTError save_changes(std::ostream& out);
	};

	class  material { //add functions to handle bitflags?
	private:
		uint32_t flags;

	public:
		std::string name;

		RGBA8 blackColor;
		RGBA8 whiteColor;

		std::vector<texRef> texRefs = {};
		std::vector<texTransform> texTransforms = {};
		std::vector<texCoordGen> texCoordGens = {};
		std::vector<tevStage> tevStages = {};
		std::optional<alphaCompare> alphaComparison = std::nullopt;
		std::optional<blendMode> blendingMode = std::nullopt;
		bool textureOnly = false;
		std::optional<blendMode> blendModeLogic = std::nullopt;
		std::optional<indirectParam> indParameter = std::nullopt;
		std::vector<projectionMap> projectionMaps = {};
		std::optional<fontShadowParameter> fontShadowParam = std::nullopt;
		bool alphaInterpolation;

		FLYTError read(std::istream& bflyt);

		inline void setFlag(const uint32_t value, const uint32_t mask, const uint8_t pos) {
			flags = (flags & ~mask) | ((value << pos) & mask);
			return;
		}

		FLYTError save_changes(std::ostream& out);
	};

	class mat1 {
	private:
		char magicMAT1[4];
		uint32_t sectionSize;
		uint16_t numMats;
		uint8_t padding_0x00[2];
		std::vector<uint32_t> matOffsets;

	public:
		std::vector<material> materials; //fields don't really need editing, just need to read data (theyre also quite complex)

		FLYTError read(std::istream& bflyt);
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
	private:
		char magic[4];
		uint32_t sectionSize;
		uint16_t numEntries;
		uint8_t padding_0x00[2];

	public:
		std::vector<userDataEntry> entries;

		FLYTError read(std::istream& bflyt);
		FLYTError save_changes(std::ostream& out);
	};

	class cnt1 {
	private:
		unsigned int animNameTableOffset; //offset in section, not file

		char magic[4];
		uint32_t sectionSize;
		uint32_t paneNamesOffset;
		uint16_t paneCount;
		uint16_t animCount;

		std::vector<uint32_t> animNameOffsets;

	public:
		std::string name;
		std::vector<std::string> paneNames;
		std::vector<std::string> animNames;

		FLYTError read(std::istream& bflyt);
		FLYTError save_changes(std::ostream& out);
	};

	class grp1 {
	private:
		char magic[4];
		uint32_t sectionSize;
		uint16_t numPanes;
		uint8_t padding_0x00[2];

	public:
		std::string groupName;
		std::vector<std::string> paneNames; //names of panes in group

		std::vector<grp1> children;

		FLYTError read(std::istream& bflyt);
		FLYTError save_changes(std::ostream& out, uint16_t& sectionNum);
	};

	class PaneBase {
	protected:
		char magic[4];
		uint32_t sectionSize;
		
	public:
		uint8_t bitFlags;
		uint8_t originFlags;
		uint8_t alpha;
		uint8_t paneMagFlags;
		std::string name; //always 24 bytes, padded with null (hence no char array)
		std::string userInfo;
		vec3<float> translation;
		vec3<float> rotation;
		vec2<float> scale;
		float width;
		float height;
		virtual ~PaneBase();
		virtual FLYTError read(std::istream& bflyt);
		virtual std::unique_ptr<PaneBase> clonePane();
		virtual FLYTError save_changes(std::ostream& out);
	};
	class pan1 : public PaneBase {
	public:	
		~pan1() override;
		FLYTError read(std::istream& bflyt) override;
		std::unique_ptr<PaneBase> clonePane() override;
		FLYTError save_changes(std::ostream& out) override;
	};
	class bnd1 : public PaneBase {
	public:	
		~bnd1() override;
		FLYTError read(std::istream& bflyt) override;
		std::unique_ptr<PaneBase> clonePane() override;
		FLYTError save_changes(std::ostream& out) override;
	};
	struct windowContent {
		RGBA8 vertexColorTL;
		RGBA8 vertexColorTR;
		RGBA8 vertexColorBL;
		RGBA8 vertexColorBR;
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
	private:
		uint8_t padding_0x00[2];
		uint32_t contentOffset;
		uint8_t frameNum;
		uint32_t frameTableOffset;
		std::vector<uint32_t> frameTable; //table of frame offsets	
	public:
		int16_t leftStretch;
		int16_t rightStretch;
		int16_t topStretch;
		int16_t bottomStretch;
		int16_t frameSizeLeft;
		int16_t frameSizeRight;
		int16_t frameSizeTop;
		int16_t frameSizeBottom;
		uint8_t wndFlags;
		windowContent content;
		std::vector<windowFrame> frames;
		~wnd1() override;
		FLYTError read(std::istream& bflyt) override;
		std::unique_ptr<PaneBase> clonePane() override;
		FLYTError save_changes(std::ostream& out) override;
	};
	struct perCharTransform {
		float curveTimeOffset;
		float animCurveWidth;
		uint8_t loopType;
		uint8_t verticalOrigin;
		uint8_t hasAnimInfo;
		uint8_t padding_0x00;
	};
	class txt1 : public PaneBase {
	private:
		uint8_t padding_0x00;
		uint32_t textOffset;
		uint32_t nameOffset;
		std::optional<uint32_t> charTransformOffset;
	public:
		enum struct LineAlignment : uint8_t {
			NOT_SPECIFIED = 0,
			LEFT,
			CENTER,
			RIGHT
		};
		uint16_t texLen;
		uint16_t restrictedLen;
		uint16_t matIndex;
		uint16_t fontIndex;
		uint8_t textAlignment;
		LineAlignment lineAlignment;
		uint8_t txtFlags;
		float italicTilt;
		RGBA8 fontColorTop;
		RGBA8 fontColorBottom;
		float fontSizeX;
		float fontSizeY;
		float charSpace;
		float lineSpace;
		float shadowPosX;
		float shadowPosY;
		float shadowSizeX;
		float shadowSizeY;
		RGBA8 shadowColorTop;
		RGBA8 shadowColorBottom;
		float shadowItalicTilt;
		std::u16string text;
		std::string textBoxName;
		std::optional<perCharTransform> charTransform;
		~txt1() override;
		FLYTError read(std::istream& bflyt) override;
		std::unique_ptr<PaneBase> clonePane() override;
		FLYTError save_changes(std::ostream& out) override;
	};
	class pic1 : public PaneBase {
	private:
		uint8_t padding_0x00;
		uint8_t numCoords;
	public:
		RGBA8 vertexColorTL;
		RGBA8 vertexColorTR;
		RGBA8 vertexColorBL;
		RGBA8 vertexColorBR;
		uint16_t matIndex;
		std::vector<UVCoords> coords;
		~pic1() override;
		FLYTError read(std::istream& bflyt) override;
		std::unique_ptr<PaneBase> clonePane() override;
		FLYTError save_changes(std::ostream& out) override;
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
		std::optional<std::unique_ptr<PaneBase>> prop;
		std::optional<usd1> userData;
		std::optional<std::string> paneInfo;
		partProperty() = default;
		partProperty(const partProperty& property); //define this manually to deepcopy unique_ptr
	};
	class prt1 : public PaneBase {
	private:
		uint32_t propCount;
	public:
		float magnifyX;
		float magnifyY;
		std::vector<partProperty> properties;
		std::string lytFilename;
		~prt1() override;
		FLYTError read(std::istream& bflyt) override;
		std::unique_ptr<PaneBase> clonePane() override;
		FLYTError save_changes(std::ostream& out) override;
	};

}

struct FLYTHeader {
	char magicFLYT[4];
	uint16_t byteOrderMarker;
	uint16_t headerSize_0x14;
	uint32_t version_0x0202;
	uint32_t fileSize;
	uint16_t numSections;
	uint8_t padding_0x00[2];
};

class Pane {
private:
	char magic[4];

public:
	std::unique_ptr<NintendoWare::Layout::PaneBase> pane;
	std::optional<NintendoWare::Layout::usd1> userData;
	std::vector<Pane> children;

	Pane();

	FLYTError read(std::istream& bflyt);
	Pane& duplicateChildPane(const unsigned int originalIndex);
	FLYTError save_changes(std::ostream& out, uint16_t& sectionNum);
};

namespace FileTypes {

	const char* FLYTErrorGetName(FLYTError err);

	class FLYTFile : public FileType {
	public:
		FLYTHeader header;
		NintendoWare::Layout::lyt1 LYT1;
		std::optional<NintendoWare::Layout::txl1> textures; //Some layouts don't use these
		std::optional<NintendoWare::Layout::fnl1> fonts; //Some layouts don't use these
		std::optional<NintendoWare::Layout::mat1> materials; //Some layouts don't use these
		std::optional<NintendoWare::Layout::cnt1> container; //Some layouts don't use these
		std::optional<NintendoWare::Layout::usd1> userData; //Some layouts don't use these

		Pane rootPane; //all panes are children of root
		NintendoWare::Layout::grp1 rootGroup;

		FLYTFile();

		static FLYTFile createNew();
		FLYTError loadFromBinary(std::istream& bflyt);
		FLYTError loadFromFile(const std::string& filePath);
		FLYTError writeToStream(std::ostream& out);
		FLYTError writeToFile(const std::string& outFilePath);
	private:
		void initNew() override;
	};
}
