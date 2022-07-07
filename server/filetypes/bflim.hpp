#pragma once

#include <vector>
#include <variant>
#include <string>
#include <fstream>
#include "shared/gx2.hpp"



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

    uint8_t swizzle;
    GX2TileMode tileMode;
};

class Pixel {
public:
    std::variant<uint8_t, uint16_t, uint32_t> value;
};



namespace FileTypes {

    const char* FLIMErrorGetName(FLIMError err);

    class FLIMFile {
    public:
        FLIMHeader header;
        ImageInfo info;
        std::string data;

        FLIMFile();
		static FLIMFile createNew(const std::string& filename);
		FLIMError loadFromBinary(std::istream& bflim);
		FLIMError loadFromFile(const std::string& filePath);
        FLIMError replaceWithDDS(const std::string& filename, GX2TileMode tileMode, uint8_t swizzle_, bool SRGB);
		FLIMError writeToStream(std::ostream& out);
		FLIMError writeToFile(const std::string& outFilePath);
	private:
		void initNew();
    };
}
