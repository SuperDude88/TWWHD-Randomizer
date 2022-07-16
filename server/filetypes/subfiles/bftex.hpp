//Format is a part of NintendoWare::g3d (a model rendering library)
//BFTEX are a BFRES subfile that stores texture data

#pragma once

#include <string>
#include <array>
#include <vector>
#include "../shared/gx2.hpp"
#include "../shared/bfres_structs.hpp"

enum struct [[nodiscard]] FTEXError {
	NONE = 0,
	REACHED_EOF,
	COULD_NOT_OPEN,
	NOT_FTEX,
    UNSUPPORTED_FORMAT,
    UNEXPECTED_VALUE,
    BAD_DDS,
    REPLACEMENT_IMAGE_TOO_LARGE,
    TOO_MANY_MIPS,
    UNSUPPORTED_DEPTH,
	UNKNOWN,
	COUNT
};

struct FTEXHeader {
    
};

namespace FileTypes::Subfiles {

    const char* FTEXErrorGetName(FTEXError err);

    class FTEXFile {
    public:
        unsigned int offset;

        char magicFTEX[4];
        GX2SurfaceDim dimension;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t mipCount;
        GX2SurfaceFormat format;
        GX2AAMode aaMode;
        GX2SurfaceUse use;
        uint32_t dataLen;
        uint32_t dataPtr;
        uint32_t mipDataLen;
        uint32_t mipPtr;
        GX2TileMode tileMode;
        uint32_t swizzle;
        uint32_t alignment;
        uint32_t pitch;
        std::array<uint32_t, 13> mipOffsets;
        uint32_t texHandle;
        uint32_t viewMipFirst;
        uint32_t viewMipCount;
        uint32_t viewSliceFirst;
        uint32_t viewSliceCount;
        std::array<uint8_t, 4> compSel;
        std::array<uint32_t, 5> texRegs;
        uint32_t arrayLen;
        RelOffset<int32_t> nameOffset, pathOffset, dataOffset, mipOffset, userDataOffset;
        
        std::string name;
        std::string path;
        std::string data;
        std::string mipData;
        std::vector<UserData> userData;

        FTEXFile();
		static FTEXFile createNew(const std::string& filename);
		FTEXError loadFromBinary(std::istream& ftex);
        FTEXError replaceImageData(const std::string& filePath, const GX2TileMode& tileMode, const uint32_t& swizzle_, const bool& SRGB, const bool& importMips);
        FTEXError writeToStream(std::ostream& out);
	private:
		void initNew();
    };
}

