#include "bflim.hpp"

#include <cstring>
#include <algorithm>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <command/Log.hpp>
#include <utility/endian.hpp>
#include <filetypes/dds.hpp>
#include <filetypes/texture/addrlib.hpp>
#include <filetypes/texture/formconv.hpp>



using eType = Utility::Endian::Type;

static const std::unordered_map<uint32_t, std::string> supportedFormats {
    {0x00000000, "GX2_SURFACE_FORMAT_INVALID"},
    {0x0000001a, "GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM"},
    {0x0000041a, "GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_SRGB"},
    {0x00000019, "GX2_SURFACE_FORMAT_TCS_R10_G10_B10_A2_UNORM"},
    {0x00000008, "GX2_SURFACE_FORMAT_TCS_R5_G6_B5_UNORM"},
    {0x0000000a, "GX2_SURFACE_FORMAT_TC_R5_G5_B5_A1_UNORM"},
    {0x0000000b, "GX2_SURFACE_FORMAT_TC_R4_G4_B4_A4_UNORM"},
    {0x00000001, "GX2_SURFACE_FORMAT_TC_R8_UNORM"},
    {0x00000007, "GX2_SURFACE_FORMAT_TC_R8_G8_UNORM"},
    {0x00000002, "GX2_SURFACE_FORMAT_TC_R4_G4_UNORM"},
    {0x00000031, "GX2_SURFACE_FORMAT_T_BC1_UNORM"},
    {0x00000431, "GX2_SURFACE_FORMAT_T_BC1_SRGB"},
    {0x00000032, "GX2_SURFACE_FORMAT_T_BC2_UNORM"},
    {0x00000432, "GX2_SURFACE_FORMAT_T_BC2_SRGB"},
    {0x00000033, "GX2_SURFACE_FORMAT_T_BC3_UNORM"},
    {0x00000433, "GX2_SURFACE_FORMAT_T_BC3_SRGB"},
    {0x00000034, "GX2_SURFACE_FORMAT_T_BC4_UNORM"},
    {0x00000035, "GX2_SURFACE_FORMAT_T_BC5_UNORM"},
};

static const std::unordered_set<uint32_t> BCn_formats = {0x31, 0x431, 0x32, 0x432, 0x33, 0x433, 0x34, 0x35};

void computeSwizzleTileMode(const uint8_t& in, uint32_t& swizzle, GX2TileMode& tileMode) {
    tileMode = GX2TileMode(in & 0x1F);
    swizzle = ((in >> 5) & 7) << 8;
    if (tileMode != 1 && tileMode != 2 && tileMode != 3 && tileMode != 16) {
        swizzle |= 0xd0000;
    }

    return;
}

uint8_t computeSwizzleTileMode(uint8_t swizzle, GX2TileMode tileMode) {
    return uint8_t(swizzle << 5) | static_cast<uint8_t>(tileMode);
}

bool generateDDSHeader(DDSHeader& out, const uint32_t& num_mipmaps_, const uint32_t& width, const uint32_t& height, const std::variant<std::string, uint32_t>& format_, const std::array<uint8_t, 4>& compSel, const uint32_t& size_, const bool& compressed) {
    bool luminance = false;
    bool RGB = false;

    std::unordered_map<uint8_t, uint32_t> compSels;
    uint32_t fmtbpp = 0;
    std::string fourcc(4, '\0');

    bool has_alpha = true;

    if(format_.index() == 1) {
        uint32_t format = std::get<uint32_t>(format_);
        if (format == 28) { // ABGR8
            RGB = true;
            compSels = {{0, 0x000000ff}, {1, 0x0000ff00}, {2, 0x00ff0000}, {3, 0xff000000}, {5, 0}};
            fmtbpp = 4;
        }
        else if (format == 24) { // A2RGB10
            RGB = true;
            compSels = {{0, 0x3ff00000}, {1, 0x000ffc00}, {2, 0x000003ff}, {3, 0xc0000000}, {5, 0}};
            fmtbpp = 4;
        }
        else if (format == 85) { // BGR565
            RGB = true;
            compSels = {{0, 0x0000001f}, {1, 0x000007e0}, {2, 0x0000f800}, {3, 0}, {5, 0}};
            fmtbpp = 2;
            has_alpha = false;
        }
        else if (format == 86) { // A1BGR5
            RGB = true;
            compSels = {{0, 0x0000001f}, {1, 0x000003e0}, {2, 0x00007c00}, {3, 0x00008000}, {5, 0}};
            fmtbpp = 2;
        }
        else if (format == 115) { // ABGR4
            RGB = true;
            compSels = {{0, 0x0000000f}, {1, 0x000000f0}, {2, 0x00000f00}, {3, 0x0000f000}, {5, 0}};
            fmtbpp = 2;
        }
        else if (format == 61) { // L8
            luminance = true;
            compSels = {{0, 0x000000ff}, {1, 0}, {2, 0}, {3, 0}, {5, 0}};
            fmtbpp = 1;

            if (compSel[3] != 0) has_alpha = false;
        }
        else if (format == 49) { // A8L8
            luminance = true;
            compSels = {{0, 0x000000ff}, {1, 0x0000ff00}, {2, 0}, {3, 0}, {5, 0}};
            fmtbpp = 2;
        }
        else if (format == 112) { // A4L4
            luminance = true;
            compSels = {{0, 0x0000000f}, {1, 0x000000f0}, {2, 0}, {3, 0}, {5, 0}};
            fmtbpp = 1;
        }
    }

    uint32_t flags = 0x00000001 | 0x00001000 | 0x00000004 | 0x00000002;
    uint32_t caps = 0x00001000;

    uint32_t num_mipmaps = num_mipmaps_;
    if(num_mipmaps == 0) {
        num_mipmaps = 1;
    }
    else if(num_mipmaps != 1) {
        flags |= 0x00020000;
        caps |= 0x00000008 | 0x00400000;
    }

    bool a;
    uint32_t pflags;
    uint32_t size = size_;
    if(!compressed) {
        flags |= 0x00000008;

        a = false;

        if (compSel[0] != 0 && compSel[1] != 0 && compSel[2] != 0 && compSel[3] == 0) {  // ALPHA
            a = true;
            pflags = 0x00000002;
        }
        else if (luminance) {  // LUMINANCE
            pflags = 0x00020000;
        }
        else if (RGB) {  // RGB
            pflags = 0x00000040;
        }
        else {  // Not possible
          {
            ErrorLog::getInstance().log("Could not generate DDS header!");
            return false;
          }
        }
        if (has_alpha && !a) {
            pflags |= 0x00000001;
        }

        size = width * fmtbpp;
    }
    else {
        flags |= 0x00080000;
        pflags = 0x00000004;

        std::string format = std::get<std::string>(format_);
        if (format == "ETC1") {
            fourcc = "ETC1";
        }
        else if (format == "BC1") {
            fourcc = "DXT1";
        }
        else if (format == "BC2") {
            fourcc = "DXT3";
        }
        else if (format == "BC3") {
            fourcc = "DXT5";
        }
        else if (dx10_formats.count(format)) {
            fourcc = "DX10";
        }
    }

    std::memcpy(out.magicDDS, "DDS ", 4);
    out.headerSize_0x7C = 0x7C;
    out.flags = DDSFlags(flags);
    out.height = height;
    out.width = width;
    out.size = size;
    out.numMips = num_mipmaps;
    out.reserved = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    out.pixelFormat.headerSize_0x20 = 0x20;
    out.pixelFormat.flags = PixelFormatFlags(pflags);

    std::memcpy(out.pixelFormat.fourcc, "\0\0\0\0", 4);
    if(compressed) {
        std::memcpy(out.pixelFormat.fourcc, &fourcc[0], 4);
    }
    else {
        out.pixelFormat.RGBBitCount = (fmtbpp << 3);

        if (compSels.count(compSel[0]) > 0) {
            out.pixelFormat.RBitMask = compSels[compSel[0]];
        }
        else {
            out.pixelFormat.RBitMask = compSels[0];
        }
        if (compSels.count(compSel[1]) > 0) {
            out.pixelFormat.GBitMask = compSels[compSel[1]];
        }
        else {
            out.pixelFormat.GBitMask = compSels[1];
        }
        if (compSels.count(compSel[2]) > 0) {
            out.pixelFormat.BBitMask = compSels[compSel[2]];
        }
        else {
            out.pixelFormat.BBitMask = compSels[2];
        }
        if (compSels.count(compSel[3]) > 0) {
            out.pixelFormat.ABitMask = compSels[compSel[3]];
        }
        else {
            out.pixelFormat.ABitMask = compSels[3];
        }
    }

    out.caps = DDSCaps(caps);
    out.caps4 = 0x00000000;
    out.reserved2 = 0x00000000;

    if(format_.index() == 0) {
        std::string format = std::get<std::string>(format_);
        if (format == "BC4U") {
            out.caps2 = DDSCaps2(0x50000000);
            out.caps3 = 0x03000000;
            out.caps4 = 0x00000000;
            out.reserved2 = 0x01000000;
        }
        else if (format == "BC4S") {
            out.caps2 = DDSCaps2(0x51000000);
            out.caps3 = 0x03000000;
            out.caps4 = 0x00000000;
            out.reserved2 = 0x01000000;
        }
        else if (format == "BC5U") {
            out.caps2 = DDSCaps2(0x53000000);
            out.caps3 = 0x03000000;
            out.caps4 = 0x00000000;
            out.reserved2 = 0x01000000;
        }
        else if (format == "BC5S") {
            out.caps2 = DDSCaps2(0x54000000);
            out.caps3 = 0x03000000;
            out.caps4 = 0x00000000;
            out.reserved2 = 0x01000000;
        }
    }

    return true;
}



namespace FileTypes {

    const char* FLIMErrorGetName(FLIMError err) {
        switch(err) {
        case FLIMError::NONE:
            return "NONE";
        case FLIMError::REACHED_EOF:
            return "REACHED_EOF";
        case FLIMError::COULD_NOT_OPEN:
            return "COULD_NOT_OPEN";
        case FLIMError::NOT_BFLIM:
            return "NOT_BFLIM";
        case FLIMError::UNKNOWN_VERSION:
            return "UNKNOWN_VERSION";
        case FLIMError::NOT_IMAG:
            return "NOT_IMAG";
        case FLIMError::UNSUPPORTED_FORMAT:
            return "UNSUPPORTED_FORMAT";
        case FLIMError::UNEXPECTED_VALUE:
            return "UNEXPECTED_VALUE";
        case FLIMError::BAD_DDS:
            return "BAD_DDS";
        default:
            return "UNKNOWN";
        }
    }

    FLIMFile::FLIMFile() {

    }

    void FLIMFile::initNew() {
        
    }

    FLIMFile FLIMFile::createNew() {
        FLIMFile newFLIM{};
        newFLIM.initNew();
        return newFLIM;
    }
    
    FLIMError FLIMFile::loadFromBinary(std::istream& bflim) {
        bflim.seekg(-0x28, std::ios::end);

        if (!bflim.read(header.magicFLIM, 4)) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }
        if (std::strncmp(header.magicFLIM, "FLIM", 4) != 0) {
            LOG_ERR_AND_RETURN(FLIMError::NOT_BFLIM)
        }
        if (!bflim.read(reinterpret_cast<char*>(&header.byteOrderMarker), sizeof(header.byteOrderMarker))) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }
        if (!bflim.read(reinterpret_cast<char*>(&header.headerSize_0x14), sizeof(header.headerSize_0x14))) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }
        if (!bflim.read(reinterpret_cast<char*>(&header.version), sizeof(header.version))) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }
        if (!bflim.read(reinterpret_cast<char*>(&header.fileSize), sizeof(header.fileSize))) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }
        if (!bflim.read(reinterpret_cast<char*>(&header.blockNum_0x01), sizeof(header.blockNum_0x01))) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }
        if (!bflim.read(reinterpret_cast<char*>(&header.padding), sizeof(header.padding))) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, header.byteOrderMarker);
        Utility::Endian::toPlatform_inplace(eType::Big, header.headerSize_0x14);
        Utility::Endian::toPlatform_inplace(eType::Big, header.version);
        Utility::Endian::toPlatform_inplace(eType::Big, header.fileSize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.blockNum_0x01);

        if (header.byteOrderMarker != 0xFEFF) LOG_ERR_AND_RETURN(FLIMError::UNEXPECTED_VALUE)
        if (header.version != 0x02020000) LOG_ERR_AND_RETURN(FLIMError::UNKNOWN_VERSION)
        if (header.blockNum_0x01 != 0x01) LOG_ERR_AND_RETURN(FLIMError::UNEXPECTED_VALUE)



        if (!bflim.read(info.magicImag, 4)) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }
        if (std::strncmp(info.magicImag, "imag", 4) != 0) {
            LOG_ERR_AND_RETURN(FLIMError::NOT_IMAG)
        }
        if (!bflim.read(reinterpret_cast<char*>(&info.size_0x10), sizeof(info.size_0x10))) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }
        if (!bflim.read(reinterpret_cast<char*>(&info.width), sizeof(info.width))) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }
        if (!bflim.read(reinterpret_cast<char*>(&info.height), sizeof(info.height))) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }
        if (!bflim.read(reinterpret_cast<char*>(&info.alignment), sizeof(info.alignment))) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }
        if (!bflim.read(reinterpret_cast<char*>(&info.format), 1)) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }
        if (!bflim.read(reinterpret_cast<char*>(&info.tile_swizzle), sizeof(info.tile_swizzle))) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }
        if (!bflim.read(reinterpret_cast<char*>(&info.dataSize), sizeof(info.dataSize))) {
            LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF)
        }

        Utility::Endian::toPlatform_inplace(eType::Big, info.size_0x10);
        Utility::Endian::toPlatform_inplace(eType::Big, info.width);
        Utility::Endian::toPlatform_inplace(eType::Big, info.height);
        Utility::Endian::toPlatform_inplace(eType::Big, info.alignment);
        Utility::Endian::toPlatform_inplace(eType::Big, info.dataSize);

        if (info.size_0x10 != 0x00000010) LOG_ERR_AND_RETURN(FLIMError::UNEXPECTED_VALUE)
        if ((info.alignment & (info.alignment - 1)) != 0) LOG_ERR_AND_RETURN(FLIMError::UNEXPECTED_VALUE) //check if alignment is a power of 2
        //if (supportedFormats.count(info.format) == 0) LOG_ERR_AND_RETURN(FLIMError::UNSUPPORTED_FORMAT)

        computeSwizzleTileMode(info.tile_swizzle, info.swizzle, info.tileMode);

        bflim.seekg(0, std::ios::beg);
        data.resize(info.dataSize);
        if(!bflim.read(&data[0], info.dataSize)) LOG_ERR_AND_RETURN(FLIMError::REACHED_EOF);
        return FLIMError::NONE;
    }

    FLIMError FLIMFile::loadFromFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERR_AND_RETURN(FLIMError::COULD_NOT_OPEN)
        }
        return loadFromBinary(file);
    }
    
    FLIMError FLIMFile::exportAsDDS(const std::string& outPath) {
        uint32_t format;
        std::string format_;
        std::array<uint8_t, 4> compSel;

        if (info.format == 0x00) {
            format = 0x01;
            compSel = {0, 0, 0, 5};
        }
        else if (info.format == 0x01) {
            format = 0x01;
            compSel = {5, 5, 5, 0};
        }
        else if (info.format == 0x02) {
            format = 0x02;
            compSel = {0, 0, 0, 1};
        }
        else if (info.format == 0x03) {
            format = 0x07;
            compSel = {0, 0, 0, 1};
        }
        else if (info.format == 0x05 || info.format == 0x19) {
            format = 0x08;
            compSel = {2, 1, 0, 5};
        }
        else if (info.format == 0x06) {
            format = 0x1a;
            compSel = {0, 1, 2, 5};
        }
        else if (info.format == 0x07) {
            format = 0x0a;
            compSel = {0, 1, 2, 3};
        }
        else if (info.format == 0x08) {
            format = 0x0b;
            compSel = {2, 1, 0, 3};
        }
        else if (info.format == 0x09) {
            format = 0x1a;
            compSel = {0, 1, 2, 3};
        }
        else if (info.format == 0x0a) {
            format = 0x31;
            format_ = "ETC1";
            compSel = {0, 1, 2, 3};
        }
        else if (info.format == 0x0C) {
            format = 0x31;
            format_ = "BC1";
            compSel = {0, 1, 2, 3};
        }
        else if (info.format == 0x0D) {
            format = 0x32;
            compSel = {0, 1, 2, 3};
        }
        else if (info.format == 0x0E) {
            format = 0x33;
            compSel = {0, 1, 2, 3};
        }
        else if (info.format  == 0x0F || info.format == 0x10) {
            format = 0x34;
            compSel = {0, 1, 2, 3};
        }
        else if (info.format == 0x11) {
            format = 0x35;
            compSel = {0, 1, 2, 3};
        }
        else if (info.format == 0x14) {
            format = 0x41a;
            compSel = {0, 1, 2, 3};
        }
        else if (info.format == 0x15) {
            format = 0x431;
            format_ = "BC1";
            compSel = {0, 1, 2, 3};
        }
        else if (info.format == 0x16) {
            format = 0x432;
            compSel = {0, 1, 2, 3};
        }
        else if (info.format == 0x17) {
            format = 0x433;
            compSel = {0, 1, 2, 3};
        }
        else if (info.format == 0x18) {
            format = 0x19;
            compSel = {0, 1, 2, 3};
        }
        else {
            LOG_ERR_AND_RETURN(FLIMError::UNSUPPORTED_FORMAT);
        }

        auto surfOut = getSurfaceInfo(GX2SurfaceFormat(format), info.width, info.height, 1, GX2SurfaceDim(1), info.tileMode, GX2AAMode(0), 0);
    
        uint32_t tilingDepth = surfOut.depth;
        if(surfOut.tileMode == 3) {
            tilingDepth = tilingDepth / 4;
        }

        if(tilingDepth != 1) {
            ErrorLog::getInstance().log("Unsupported depth!");
            LOG_AND_RETURN_IF_ERR(FLIMError::UNSUPPORTED_FORMAT);
        }

        std::variant<std::string, uint32_t> ddsFormat;
        if (format == 0x01) {
            ddsFormat = 61U;
        }
        else if (format == 0x02) {
            ddsFormat = 112U;
        }
        else if (format == 0x07) {
            ddsFormat = 49U;
        }
        else if (format == 0x08) {
            ddsFormat = 85U;
        }
        else if (format == 0x0a) {
            ddsFormat = 86U;
        }
        else if (format == 0x0b) {
            ddsFormat = 115U;
        }
        else if (format == 0x1a || format == 0x41a) {
            ddsFormat = 28U;
        }
        else if (format == 0x19) {
            ddsFormat = 24U;
        }
        else if (format == 0x31 || format == 0x431) {
            ddsFormat = format_;
        }
        else if (format == 0x32 || format == 0x432) {
            ddsFormat = "BC2";
        }
        else if (format == 0x33 || format == 0x433) {
            ddsFormat = "BC3";
        }
        else if (format == 0x34) {
            ddsFormat = "BC4U";
        }
        else if (format == 0x35) {
            ddsFormat = "BC5U";
        }

        std::string result = swizzleSurf(info.width, info.height, 1, GX2SurfaceFormat(format), GX2AAMode(0), GX2SurfaceUse(1), surfOut.tileMode, info.swizzle, surfOut.pitch, surfOut.bpp, 0, 0, this->data, false);
        uint32_t size;

        if(BCn_formats.count(format) > 0) {
            size = ((info.width + 3) >> 2) * ((info.height + 3) >> 2) * (surfaceGetBitsPerPixel(format) >> 3);
        }
        else {
            size = info.width * info.height * (surfaceGetBitsPerPixel(format) >> 3);
        }

        DDSFile file;
        generateDDSHeader(file.header, 1, info.width, info.height, ddsFormat, compSel, size, (BCn_formats.count(format) > 0));
        file.data = result.substr(0, size);
        file.format_ = format;
        file.size = size;
        file.compSel = compSel;
        if(const DDSError& err = file.writeToFile(outPath); err != DDSError::NONE) {
            LOG_ERR_AND_RETURN(FLIMError::BAD_DDS);
        }

        return FLIMError::NONE;
    }

    FLIMError FLIMFile::replaceWithDDS(const std::string& filename, GX2TileMode tileMode, uint8_t swizzle_, bool SRGB) {
        FileTypes::DDSFile dds;
        if(DDSError err = dds.loadFromFile(filename, SRGB); err != DDSError::NONE) {
            LOG_ERR_AND_RETURN(FLIMError::BAD_DDS);
        }

        if((dds.header.width == 0 || dds.size == 0) && dds.data.empty()) {
            LOG_ERR_AND_RETURN(FLIMError::BAD_DDS);
        }

        if(supportedFormats.count(dds.format_) == 0) {
            LOG_ERR_AND_RETURN(FLIMError::UNSUPPORTED_FORMAT);
        }

        dds.data.resize(dds.size);
    
        if(dds.format_ == 0xB) {
            dds.data = rgba4_to_argb4(dds.data);
        }

        if(tileMode == 0) {
            tileMode = getDefaultGX2TileMode(GX2SurfaceDim(1), dds.header.width, dds.header.height,
                    1, GX2SurfaceFormat(1), GX2AAMode(0), GX2SurfaceUse(1));
        }

        auto surfOut = getSurfaceInfo(GX2SurfaceFormat(dds.format_), dds.header.width, dds.header.height, 1, GX2SurfaceDim(1), tileMode, GX2AAMode(0), 0);
        uint32_t alignment = surfOut.baseAlign;

        uint32_t padSize = surfOut.surfSize - dds.size;
        dds.data += std::string(padSize, '\0');

        uint32_t tilingDepth = surfOut.depth;
        if (surfOut.tileMode == 3) tilingDepth = tilingDepth / 4;

        if (tilingDepth != 1) {
            ErrorLog::getInstance().log("Unsupported depth!");
            LOG_ERR_AND_RETURN(FLIMError::UNSUPPORTED_FORMAT);
        }

        uint8_t swizzle_tileMode = computeSwizzleTileMode(swizzle_, tileMode);

        uint32_t s = swizzle_ << 8;
        if (tileMode != 1 && tileMode != 2 && tileMode != 3 && tileMode != 16) s |= 0xd0000;

        this->data = swizzleSurf(dds.header.width, dds.header.height, 1, GX2SurfaceFormat(dds.format_), GX2AAMode(0), GX2SurfaceUse(1), surfOut.tileMode, s, surfOut.pitch, surfOut.bpp, 0, 0, dds.data, true);

        if (dds.format_ == 1) {
            if (dds.compSel[3] == 0) dds.format_ = 1;
            else dds.format_ = 0;
        }
        else if (dds.format_ == 0x1a) {
            if (std::find(dds.compSel.begin(), dds.compSel.end(), 5) != dds.compSel.end()) dds.format_ = 6;
            else dds.format_ = 9;
        }
        else if (dds.format_ == 0x31) {
            if (std::strncmp(dds.header.pixelFormat.fourcc, "ETC1", 4)) dds.format_ = 0xa;
            else dds.format_ = 0xc;
        }
        else {
            static const std::unordered_map<uint32_t, uint32_t> fmt = {
                {2, 2},
                {7, 3},
                {8, 5},
                {0xa, 7},
                {0xb, 8},
                {0x32, 0xd},
                {0x33, 0xe},
                {0x34, 0x10},
                {0x35, 0x11},
                {0x41a, 0x14},
                {0x431, 0x15},
                {0x432, 0x16},
                {0x433, 0x17},
                {0x19, 0x18}
            };

            if(fmt.count(dds.format_) == 0) LOG_ERR_AND_RETURN(FLIMError::UNSUPPORTED_FORMAT);
            dds.format_ = fmt.at(dds.format_);
        }

        std::array<uint8_t, 4> temp;
        std::array<uint8_t, 4> temp2;
        if (dds.format_ == 0) {
            temp = {0, 0, 0, 5};
            temp2 = {0, 5, 5, 5};
            if (dds.compSel != temp && dds.compSel != temp2) {
                LOG_TO_DEBUG("Warning: colors may break!");
            }
        }
        else if (dds.format_ == 1) {
            temp = {5, 5, 5, 0};
            if (dds.compSel != temp) {
                LOG_TO_DEBUG("Warning: colors may break!");
            }
        }
        else if (dds.format_ == 2 || dds.format_ == 3) {
            temp = {0, 0, 0, 1};
            temp2 = {0, 5, 5, 1};
            if (dds.compSel != temp && dds.compSel != temp2) {
                LOG_TO_DEBUG("Warning: colors may break!");
            }
        }

        if(dds.format_ == 5){
            temp = {2, 1, 0, 5};
            if (dds.compSel != temp) {
                temp = {0, 1, 2, 5};
                if (dds.compSel == temp) {
                    this->data = swapRB_16bpp(this->data, "rgb565");
                }
                else {
                    LOG_TO_DEBUG("Warning: colors may break!");
                }
            }
        }
        else if (dds.format_ == 6) {
            temp = {0, 1, 2, 5};
            if (dds.compSel != temp) {
                temp = {2, 1, 0, 5};
                if (dds.compSel == temp) {
                    this->data = swapRB_32bpp(this->data, "rgba8");
                }
                else {
                    LOG_TO_DEBUG("Warning: colors may break!");
                }
            }
        }
        else if (dds.format_ == 7) {
            temp = {0, 1, 2, 3};
            if (dds.compSel != temp) {
            temp = {2, 1, 0, 3};
                if (dds.compSel == temp) {
                    this->data = swapRB_16bpp(this->data, "rgb5a1");
                }
                else {
                    LOG_TO_DEBUG("Warning: colors may break!");
                }
            }
        }
        else if (dds.format_ == 8) {
            temp = {2, 1, 0, 3};
            if (dds.compSel != temp) {
                temp = {0, 1, 2, 3};
                if (dds.compSel == temp) {
                    this->data = swapRB_16bpp(this->data, "argb4");
                }
                else {
                    LOG_TO_DEBUG("Warning: colors may break!");
                }
            }
        }
        else if (dds.format_ == 9 || dds.format_ == 0x14 || dds.format_ == 0x18) {
            temp = {0, 1, 2, 3};
            if (dds.compSel != temp) {
                temp = {2, 1, 0, 3};
                if (dds.compSel == temp) {
                    if (dds.format_ == 0x18) {
                        this->data = swapRB_32bpp(this->data, "bgr10a2");
                    }
                    else {
                        this->data = swapRB_32bpp(this->data, "rgba8");
                    }
                }
                else {
                    LOG_TO_DEBUG("Warning: colors may break!");
                }
            }
        }
        
        header.fileSize = 0x28 + this->data.size();

        info.width = dds.header.width;
        info.height = dds.header.height;
        info.alignment = alignment;
        info.format = GX2SurfaceFormat(dds.format_);
        
        info.tile_swizzle = swizzle_tileMode;
        computeSwizzleTileMode(info.tile_swizzle, info.swizzle, info.tileMode);

        info.dataSize = this->data.size();

        return FLIMError::NONE;
    }

    FLIMError FLIMFile::writeToStream(std::ostream& out) {
        out.write(&data[0], data.size());
        
        info.tile_swizzle = computeSwizzleTileMode(info.swizzle, info.tileMode);
        
        Utility::Endian::toPlatform_inplace(eType::Big, header.byteOrderMarker);
        Utility::Endian::toPlatform_inplace(eType::Big, header.headerSize_0x14);
        Utility::Endian::toPlatform_inplace(eType::Big, header.version);
        Utility::Endian::toPlatform_inplace(eType::Big, header.fileSize);
        Utility::Endian::toPlatform_inplace(eType::Big, header.blockNum_0x01);

        out.write(header.magicFLIM, 4);
        out.write(reinterpret_cast<const char*>(&header.byteOrderMarker), sizeof(header.byteOrderMarker));
        out.write(reinterpret_cast<const char*>(&header.headerSize_0x14), sizeof(header.headerSize_0x14));
        out.write(reinterpret_cast<const char*>(&header.version), sizeof(header.version));
        out.write(reinterpret_cast<const char*>(&header.fileSize), sizeof(header.fileSize));
        out.write(reinterpret_cast<const char*>(&header.blockNum_0x01), sizeof(header.blockNum_0x01));
        out.write(reinterpret_cast<const char*>(&header.padding), sizeof(header.padding));

        Utility::Endian::toPlatform_inplace(eType::Big, info.size_0x10);
        Utility::Endian::toPlatform_inplace(eType::Big, info.width);
        Utility::Endian::toPlatform_inplace(eType::Big, info.height);
        Utility::Endian::toPlatform_inplace(eType::Big, info.alignment);
        Utility::Endian::toPlatform_inplace(eType::Big, info.dataSize);
        const uint8_t format = static_cast<uint8_t>(info.format); //narrow from uint32_t so endianness doesn't affect the byte written

        out.write(info.magicImag, 4);
        out.write(reinterpret_cast<const char*>(&info.size_0x10), sizeof(info.size_0x10));
        out.write(reinterpret_cast<const char*>(&info.width), sizeof(info.width));
        out.write(reinterpret_cast<const char*>(&info.height), sizeof(info.height));
        out.write(reinterpret_cast<const char*>(&info.alignment), sizeof(info.alignment));
        out.write(reinterpret_cast<const char*>(&format), 1);
        out.write(reinterpret_cast<const char*>(&info.tile_swizzle), sizeof(info.tile_swizzle));
        out.write(reinterpret_cast<const char*>(&info.dataSize), sizeof(info.dataSize));

        return FLIMError::NONE;
    }

    FLIMError FLIMFile::writeToFile(const std::string& outFilePath) {
        std::ofstream outFile(outFilePath, std::ios::binary);
        if (!outFile.is_open()) {
            LOG_ERR_AND_RETURN(FLIMError::COULD_NOT_OPEN)
        }
        return writeToStream(outFile);
    }
}
