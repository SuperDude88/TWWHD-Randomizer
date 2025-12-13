// DDS files store image data

#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <fstream>
#include <unordered_set>

#include <utility/path.hpp>



enum struct [[nodiscard]] DDSError {
    NONE = 0,
    REACHED_EOF,
    COULD_NOT_OPEN,
    NOT_DDS,
    UNSUPPORTED_FORMAT,
    UNEXPECTED_VALUE,
    INVALID_TEXTURE,
    UNKNOWN,
    COUNT
};

enum struct DDSFlags : uint32_t {
    CAPS = 0x1,
    HEIGHT = 0x2,
    WIDTH = 0x4,
    PITCH = 0x8,
    PIXELFORMAT = 0x1000,
    MIPMAPCOUNT = 0x20000,
    LINEARSIZE = 0x80000,
    DEPTH = 0x800000
};

enum struct DDSCaps : uint32_t {
    COMPLEX = 0x8,
    MIPMAP = 0x400000,
    TEXTURE = 0x1000,
    ALL = (COMPLEX | MIPMAP | TEXTURE)
};

enum struct DDSCaps2 : uint32_t {
    CUBEMAP = 0x200,
    CUBEMAP_POSITIVEX = 0x400,
    CUBEMAP_NEGATIVEX = 0x800,
    CUBEMAP_POSITIVEY = 0x1000,
    CUBEMAP_NEGATIVEY = 0x2000,
    CUBEMAP_POSITIVEZ = 0x4000,
    CUBEMAP_NEGATIVEZ = 0x8000,
    VOLUME = 0x20000
};

enum struct PixelFormatFlags : uint32_t {
    ALPHAPIXELS = 0x1,
    ALPHA = 0x2,
    FOURCC = 0x4,
    RGB = 0x40,
    YUV = 0x200,
    LUMINANCE = 0x20000
};

struct DDSPixelFormat {
    uint32_t headerSize_0x20;
    PixelFormatFlags flags;
    char fourcc[4];
    uint32_t RGBBitCount;
    uint32_t RBitMask;
    uint32_t GBitMask;
    uint32_t BBitMask;
    uint32_t ABitMask;
};

struct DDSHeader {
    char magicDDS[4];
    uint32_t headerSize_0x7C;
    DDSFlags flags;
    uint32_t height;
    uint32_t width;
    uint32_t size;
    uint32_t depth;
    uint32_t numMips;
    std::array<uint32_t, 11> reserved;
    DDSPixelFormat pixelFormat;
    DDSCaps caps;
    DDSCaps2 caps2;
    uint32_t caps3;
    uint32_t caps4;
    uint32_t reserved2;

    static inline const std::unordered_set<std::string> dx10_formats = {"BC4U", "BC4S", "BC5U", "BC5S"};
};

namespace FileTypes {

    const char* DDSErrorGetName(DDSError err);

    class DDSFile {
    public:
        DDSHeader header{};
        std::string data;

        uint32_t format_{};
        uint32_t size{};
        std::array<uint8_t, 4> compSel{};

        DDSFile() = default;
        static DDSFile createNew();
        DDSError loadFromBinary(std::istream& dds, const bool SRGB);
        DDSError loadFromFile(const fspath& filePath, const bool SRGB = false);
        DDSError writeToStream(std::ostream& out);
        DDSError writeToFile(const fspath& outFilePath);
    private:
        void initNew();
    };
}
