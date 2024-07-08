//Format is a part of NintendoWare::lyt (a UI library)
//BFLIM files store textures used for 2D layouts
//They are used in conjunction with BFLYT files

#pragma once

#include <string>
#include <filetypes/shared/gx2.hpp>
#include <filetypes/baseFiletype.hpp>



enum struct [[nodiscard]] FLIMError {
	NONE = 0,
	REACHED_EOF,
	COULD_NOT_OPEN,
	NOT_BFLIM,
    UNKNOWN_VERSION,
	NOT_IMAG,
    UNSUPPORTED_FORMAT,
    UNEXPECTED_VALUE,
    BAD_DDS,
	UNKNOWN,
	COUNT
};

struct FLIMHeader {
    char magicFLIM[4];
    uint16_t byteOrderMarker;
    uint16_t headerSize_0x14;
    uint32_t version;
    uint32_t fileSize;
    uint16_t blockNum_0x01;
    uint8_t padding[2];
};

struct ImageInfo {
    char magicImag[4];
    uint32_t size_0x10;
    uint16_t width;
    uint16_t height;
    uint16_t alignment;
    GX2SurfaceFormat format;
    uint8_t tile_swizzle;
    uint32_t dataSize;

    uint32_t swizzle;
    GX2TileMode tileMode;
};



namespace FileTypes {

    const char* FLIMErrorGetName(FLIMError err);

    class FLIMFile final : public FileType {
    public:
        FLIMHeader header{};
        ImageInfo info{};
        std::string data;

        FLIMFile() = default;
		static FLIMFile createNew();
		FLIMError loadFromBinary(std::istream& bflim);
		FLIMError loadFromFile(const fspath& filePath);
		FLIMError exportAsDDS(const fspath& outPath);
		FLIMError replaceWithDDS(const fspath& filename, GX2TileMode tileMode, uint8_t swizzle_, bool SRGB);
		FLIMError writeToStream(std::ostream& out);
		FLIMError writeToFile(const fspath& outFilePath);
	private:
		void initNew() override;
    };
}
