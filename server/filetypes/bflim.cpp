#include "bflim.hpp"

#include <cmath>
#include <cstring>
#include <algorithm>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include "../command/Log.hpp"
#include "../utility/endian.hpp"
#include "./dds.hpp"
#include "bflim/addrlib.hpp"
#include "bflim/formconv.hpp"



using eType = Utility::Endian::Type;

const std::unordered_map<uint32_t, std::string> supportedFormats {
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

const std::unordered_set<uint32_t> BCn_formats = {0x31, 0x431, 0x32, 0x432, 0x33, 0x433, 0x34, 0x35};

void warn_color() {
    BasicLog::getInstance().log("Warning: colors may break!");
}

uint8_t computeSwizzleTileMode(uint8_t swizzle, GX2TileMode tileMode) {
    return (swizzle << 5) | static_cast<uint8_t>(tileMode);
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

    FLIMFile FLIMFile::createNew(const std::string& filename) {
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
        info.format = static_cast<GX2SurfaceFormat>(Utility::Endian::toPlatform(eType::Big, static_cast<uint32_t>(info.format)));
        Utility::Endian::toPlatform_inplace(eType::Big, info.dataSize);

        if (info.size_0x10 != 0x00000010) LOG_ERR_AND_RETURN(FLIMError::UNEXPECTED_VALUE)
        if ((info.alignment & (info.alignment - 1)) != 0) LOG_ERR_AND_RETURN(FLIMError::UNEXPECTED_VALUE) //check if alignment is a power of 2
        //if (supportedFormats.count(info.format) == 0) LOG_ERR_AND_RETURN(FLIMError::UNSUPPORTED_FORMAT)

        info.tileMode = GX2TileMode(info.tile_swizzle & 0b00011111);
        info.swizzle = (info.tile_swizzle & 0b11100000) >> 5;

        //pixels
        return FLIMError::NONE; //unfinished
    }

    FLIMError FLIMFile::loadFromFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERR_AND_RETURN(FLIMError::COULD_NOT_OPEN)
		}
		return loadFromBinary(file);
    }

    FLIMError FLIMFile::replaceWithDDS(const std::string& filename, GX2TileMode tileMode, uint8_t swizzle_, bool SRGB) {
        FileTypes::DDSFile dds;
        if(DDSError err = dds.loadFromFile(filename, SRGB); err != DDSError::NONE) {
            return FLIMError::BAD_DDS;
        }

        if((dds.header.width == 0 || dds.size == 0) && dds.data.empty()) {
            return FLIMError::BAD_DDS;
        }

        if(supportedFormats.count(dds.format_) == 0) {
            return FLIMError::UNSUPPORTED_FORMAT;
        }

        dds.data.resize(dds.size);
    
        if(dds.format_ == 0xB) {
            dds.data = rgba4_to_argb4(dds.data);
        }

        if(!static_cast<uint32_t>(tileMode)) tileMode = getDefaultGX2TileMode(GX2SurfaceDim(1), dds.header.width, dds.header.height, 1, GX2SurfaceFormat(1),
                                                                                GX2AAMode(0), GX2SurfaceUse(1));

        uint32_t bpp = surfaceGetBitsPerPixel(dds.format_) >> 3;

        auto surfOut = getSurfaceInfo(GX2SurfaceFormat(dds.format_), dds.header.width, dds.header.height, 1, GX2SurfaceDim(1), tileMode, GX2AAMode(0), 0);
        uint32_t alignment = surfOut.baseAlign;

        uint32_t padSize = surfOut.surfSize - dds.size;
        dds.data += std::string(padSize, '\0');

        uint32_t tilingDepth = surfOut.depth;
        if (static_cast<uint32_t>(surfOut.tileMode) == 3) tilingDepth = std::floor(tilingDepth / 4);

        if (tilingDepth != 1) {
            BasicLog::getInstance().log("Unsupported depth!");
            return FLIMError::UNSUPPORTED_FORMAT;
        }

        uint8_t swizzle_tileMode = computeSwizzleTileMode(swizzle_, tileMode);

        auto s = swizzle_ << 8;
        uint32_t tileModeTemp = static_cast<uint32_t>(tileMode);
        if (tileModeTemp != 1 && tileModeTemp != 2 && tileModeTemp != 3 && tileModeTemp != 16) s |= 0xd0000;

        this->data = swizzleSurf(dds.header.width, dds.header.height, 1, GX2SurfaceFormat(dds.format_), GX2AAMode(0), GX2SurfaceUse(1), surfOut.tileMode, s, surfOut.pitch, surfOut.bpp, 0, 0, dds.data, data.size(), true);

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
            std::unordered_map<uint32_t, uint32_t> fmt = {
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

            dds.format_ = fmt[dds.format_];
        }

        std::array<int, 4> temp;
        std::array<int, 4> temp2;
        if (dds.format_ == 0) {
            temp = {0, 0, 0, 5};
            temp2 = {0, 5, 5, 5};
            if (dds.compSel != temp && dds.compSel != temp2) {
                warn_color();
            }
        }
        else if (dds.format_ == 1) {
            temp = {5, 5, 5, 0};
            if (dds.compSel != temp) {
                warn_color();
            }
        }
        else if (dds.format_ == 2 || dds.format_ == 3) {
            temp = {0, 0, 0, 1};
            temp2 = {0, 5, 5, 1};
            if (dds.compSel != temp && dds.compSel != temp2) {
                warn_color();
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
                    warn_color();
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
                    warn_color();
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
                    warn_color();
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
                    warn_color();
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
                    warn_color();
                }
            }
        }
        
        header.fileSize = 0x28 + this->data.size();

        info.width = dds.header.width;
        info.height = dds.header.height;
        info.alignment = alignment;
        info.format = GX2SurfaceFormat(dds.format_);
        info.tile_swizzle = swizzle_tileMode;
        info.dataSize = this->data.size();

        return FLIMError::NONE;
    }

    FLIMError FLIMFile::writeToStream(std::ostream& out) {
        out.write(&data[0], data.size());
        
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
        uint8_t format_BE = static_cast<uint8_t>(info.format);
        Utility::Endian::toPlatform_inplace(eType::Big, info.dataSize);

        out.write(info.magicImag, 4);
        out.write(reinterpret_cast<const char*>(&info.size_0x10), sizeof(info.size_0x10));
        out.write(reinterpret_cast<const char*>(&info.width), sizeof(info.width));
        out.write(reinterpret_cast<const char*>(&info.height), sizeof(info.height));
        out.write(reinterpret_cast<const char*>(&info.alignment), sizeof(info.alignment));
        out.write(reinterpret_cast<const char*>(&info.format), 1);
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
