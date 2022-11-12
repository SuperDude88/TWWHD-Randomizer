#include "bftex.hpp"

#include <unordered_map>
#include <cstring>

#include <utility/endian.hpp>
#include <utility/common.hpp>
#include <utility/math.hpp>
#include <filetypes/texture/addrlib.hpp>
#include <command/Log.hpp>
#include <filetypes/dds.hpp>

using eType = Utility::Endian::Type;

static const std::unordered_map<uint32_t, std::string> supportedFormats {
    {0x00000001, "GX2_SURFACE_FORMAT_TC_R8_UNORM"},
    {0x00000002, "GX2_SURFACE_FORMAT_TC_R4_G4_UNORM"},
    {0x00000007, "GX2_SURFACE_FORMAT_TC_R8_G8_UNORM"},
    {0x00000008, "GX2_SURFACE_FORMAT_TCS_R5_G6_B5_UNORM"},
    {0x0000000a, "GX2_SURFACE_FORMAT_TC_R5_G5_B5_A1_UNORM"},
    {0x0000000b, "GX2_SURFACE_FORMAT_TC_R4_G4_B4_A4_UNORM"},
    {0x00000019, "GX2_SURFACE_FORMAT_TCS_R10_G10_B10_A2_UNORM"},
    {0x0000001a, "GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM"},
    {0x0000041a, "GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_SRGB"},
    {0x00000031, "GX2_SURFACE_FORMAT_T_BC1_UNORM"},
    {0x00000431, "GX2_SURFACE_FORMAT_T_BC1_SRGB"},
    {0x00000032, "GX2_SURFACE_FORMAT_T_BC2_UNORM"},
    {0x00000432, "GX2_SURFACE_FORMAT_T_BC2_SRGB"},
    {0x00000033, "GX2_SURFACE_FORMAT_T_BC3_UNORM"},
    {0x00000433, "GX2_SURFACE_FORMAT_T_BC3_SRGB"},
    {0x00000034, "GX2_SURFACE_FORMAT_T_BC4_UNORM"},
    {0x00000234, "GX2_SURFACE_FORMAT_T_BC4_SNORM"},
    {0x00000035, "GX2_SURFACE_FORMAT_T_BC5_UNORM"},
    {0x00000235, "GX2_SURFACE_FORMAT_T_BC5_SNORM"}
};

static const std::unordered_set<uint32_t> BCn_formats = {0x31, 0x431, 0x32, 0x432, 0x33, 0x433, 0x34, 0x234, 0x35, 0x235};

std::pair<uint32_t, uint32_t> getCurrentMipOffset_Size(uint32_t width, uint32_t height, uint32_t blkWidth, uint32_t blkHeight, uint32_t bpp, uint32_t currLevel) {
    uint32_t offset = 0;
    uint32_t width_ = 0;
    uint32_t height_ = 0;

    for (unsigned int mipLevel = 0; mipLevel < currLevel; mipLevel++) {
        width_ = std::max<uint32_t>(1, width >> mipLevel) / blkWidth; //TODO: make division ceil instead of floor
        height_ = std::max<uint32_t>(1, height >> mipLevel) / blkHeight; //TODO: make division ceil instead of floor

        offset += width_ * height_ * bpp;
    }

    width_ = std::max<uint32_t>(1, width >> currLevel) / blkWidth; //TODO: make division ceil instead of floor
    height_ = std::max<uint32_t>(1, height >> currLevel) / blkHeight; //TODO: make division ceil instead of floor

    uint32_t size = width_ * height_ * bpp;

    return {offset, size};
}

namespace FileTypes::Subfiles {

    const char* FTEXErrorGetName(FTEXError err) {
        switch (err) {
            case FTEXError::NONE:
                return "NONE";
	        case FTEXError::REACHED_EOF:
                return "REACHED_EOF";
	        case FTEXError::COULD_NOT_OPEN:
                return "COULD_NOT_OPEN";
	        case FTEXError::NOT_FTEX:
                return "NOT_FTEX";
            case FTEXError::UNSUPPORTED_FORMAT:
                return "UNSUPPORTED_FORMAT";
            case FTEXError::UNEXPECTED_VALUE:
                return "UNEXPECTED_VALUE";
            case FTEXError::BAD_DDS:
                return "BAD_DDS";
            case FTEXError::REPLACEMENT_IMAGE_TOO_LARGE:
                return "REPLACEMENT_IMAGE_TOO_LARGE";
            case FTEXError::TOO_MANY_MIPS:
                return "TOO_MANY_MIPS";
            case FTEXError::UNSUPPORTED_DEPTH:
                return "UNSUPPORTED_DEPTH";
            default:
                return "UNKNOWN";
        }
    }

    FTEXFile::FTEXFile() {
        
    }

    void FTEXFile::initNew() {
        
    }

    FTEXFile FTEXFile::createNew() {
		FTEXFile newFTEX{};
		newFTEX.initNew();
		return newFTEX;
    }

    FTEXError FTEXFile::loadFromBinary(std::istream& ftex) {
        this->offset = ftex.tellg();

        if (!ftex.read(magicFTEX, 4)) LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        if (std::strncmp(magicFTEX, "FTEX", 4) != 0)
        {
            LOG_ERR_AND_RETURN(FTEXError::NOT_FTEX);
        }
        if (!ftex.read(reinterpret_cast<char*>(&dimension), 4))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&width), sizeof(width)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&height), sizeof(height)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&depth), sizeof(depth)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&mipCount), sizeof(mipCount)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&format), sizeof(format)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&aaMode), sizeof(aaMode)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&use), sizeof(use)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&dataLen), sizeof(dataLen)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&dataPtr), sizeof(dataPtr)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&mipDataLen), sizeof(mipDataLen)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&mipPtr), sizeof(mipPtr)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&tileMode), sizeof(tileMode)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&swizzle), sizeof(swizzle)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&alignment), sizeof(alignment)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&pitch), sizeof(pitch)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        for(unsigned int i = 0; i < mipOffsets.size(); i++) {
            if (!ftex.read(reinterpret_cast<char*>(&mipOffsets[i]), sizeof(mipOffsets[i])))
            {
                LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
            }
        }
        if (!ftex.read(reinterpret_cast<char*>(&viewMipFirst), sizeof(viewMipFirst)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&viewMipCount), sizeof(viewMipCount)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&viewSliceFirst), sizeof(viewSliceFirst)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&viewSliceCount), sizeof(viewSliceCount)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&compSel), sizeof(compSel)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        for(unsigned int i = 0; i < texRegs.size(); i++) {
            if (!ftex.read(reinterpret_cast<char*>(&texRegs[i]), sizeof(texRegs[i])))
            {
                LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
            }
        }
        if (!ftex.read(reinterpret_cast<char*>(&texHandle), sizeof(texHandle)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        if (!ftex.read(reinterpret_cast<char*>(&arrayLen), sizeof(arrayLen)))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, dimension);
        Utility::Endian::toPlatform_inplace(eType::Big, width);
        Utility::Endian::toPlatform_inplace(eType::Big, height);
        Utility::Endian::toPlatform_inplace(eType::Big, depth);
        Utility::Endian::toPlatform_inplace(eType::Big, mipCount);
        Utility::Endian::toPlatform_inplace(eType::Big, format);
        Utility::Endian::toPlatform_inplace(eType::Big, aaMode);
        Utility::Endian::toPlatform_inplace(eType::Big, use);
        Utility::Endian::toPlatform_inplace(eType::Big, dataLen);
        Utility::Endian::toPlatform_inplace(eType::Big, dataPtr);
        Utility::Endian::toPlatform_inplace(eType::Big, mipDataLen);
        Utility::Endian::toPlatform_inplace(eType::Big, mipPtr);
        Utility::Endian::toPlatform_inplace(eType::Big, tileMode);
        Utility::Endian::toPlatform_inplace(eType::Big, swizzle);
        Utility::Endian::toPlatform_inplace(eType::Big, alignment);
        Utility::Endian::toPlatform_inplace(eType::Big, pitch);
        for (uint32_t& offset : mipOffsets) {
            Utility::Endian::toPlatform_inplace(eType::Big, offset);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, viewMipFirst);
        Utility::Endian::toPlatform_inplace(eType::Big, viewMipCount);
        Utility::Endian::toPlatform_inplace(eType::Big, viewSliceFirst);
        Utility::Endian::toPlatform_inplace(eType::Big, viewSliceCount);
        for(uint32_t& reg : texRegs) {
            Utility::Endian::toPlatform_inplace(eType::Big, reg);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, texHandle);
        Utility::Endian::toPlatform_inplace(eType::Big, arrayLen);

        nameOffset.read(ftex);
        std::istream::pos_type curPos = ftex.tellg();
        name = readNullTerminatedStr(ftex, nameOffset.offset);
        ftex.seekg(curPos, std::ios::beg);
        
        pathOffset.read(ftex);
        curPos = ftex.tellg();
        path = readNullTerminatedStr(ftex, pathOffset.offset);
        ftex.seekg(curPos, std::ios::beg);
        
        dataOffset.read(ftex);
        curPos = ftex.tellg();
        ftex.seekg(dataOffset.offset, std::ios::beg);
        data.resize(dataLen);
        if (!ftex.read(&data[0], dataLen))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        ftex.seekg(curPos, std::ios::beg);

        mipOffset.read(ftex);
        curPos = ftex.tellg();
        ftex.seekg(mipOffset.offset, std::ios::beg);
        mipData.resize(mipDataLen);
        if (!ftex.read(&mipData[0], mipDataLen))
        {
            LOG_ERR_AND_RETURN(FTEXError::REACHED_EOF);
        }
        ftex.seekg(curPos, std::ios::beg);

        userDataOffset.read(ftex);
        curPos = ftex.tellg();
        ftex.seekg(userDataOffset.offset, std::ios::beg);
        //TODO: finish this
        ftex.seekg(curPos, std::ios::beg);

        return FTEXError::NONE;
    }

    FTEXError FTEXFile::replaceImageData(const std::string& filePath, const GX2TileMode& tileMode, const uint32_t& swizzle_, const bool& SRGB, const bool& importMips) {
        FileTypes::DDSFile dds;
        if(DDSError err = dds.loadFromFile(filePath, SRGB); err != DDSError::NONE) {
            LOG_ERR_AND_RETURN(FTEXError::BAD_DDS);
        }

        if((dds.header.width == 0 || dds.size == 0) && dds.data.empty()) {
            LOG_ERR_AND_RETURN(FTEXError::BAD_DDS);
        }

        if(supportedFormats.count(dds.format_) == 0) {
            LOG_ERR_AND_RETURN(FTEXError::UNSUPPORTED_FORMAT);
        }

        if(dds.header.numMips > 13) LOG_ERR_AND_RETURN(FTEXError::TOO_MANY_MIPS);

        if(importMips) {
            if(mipCount < dds.header.numMips + 1) {
                LOG_ERR_AND_RETURN(FTEXError::TOO_MANY_MIPS);
            }
            dds.header.numMips += 1;
        }
        else {
            dds.header.numMips = 1;
        }

        uint32_t bpp = surfaceGetBitsPerPixel(dds.format_) / 8;
        auto surfInfo = getSurfaceInfo(GX2SurfaceFormat(dds.format_), dds.header.width, dds.header.height, GX2SurfaceDim(1), GX2SurfaceDim(1), tileMode, GX2AAMode(0), 0);

        if(surfInfo.depth != 1) LOG_ERR_AND_RETURN(FTEXError::UNSUPPORTED_DEPTH);
        if((uint64_t)surfInfo.surfSize > data.size()) LOG_ERR_AND_RETURN(FTEXError::REPLACEMENT_IMAGE_TOO_LARGE);

        uint32_t s = 0xd0000 | swizzle_ << 8;
        if(tileMode == 1 || tileMode == 2 || tileMode == 3 || tileMode == 16) {
            s = swizzle_ << 8;
        }

        uint32_t blkWidth, blkHeight;
        if(BCn_formats.count(dds.format_) > 0) {
            blkWidth = 4;
            blkHeight = 4;
        }
        else {
            blkWidth = 1;
            blkHeight = 1;
        }

        uint32_t mipSize = 0;
        std::vector<uint32_t> mipOffsets_ = {};

        std::vector<std::string> result;
        uint32_t mipLevel = 0;
        for(; mipLevel < dds.header.numMips; mipLevel++) {
            auto offset_size = getCurrentMipOffset_Size(width, height, blkWidth, blkHeight, bpp, mipLevel);
            std::string data_ = dds.data.substr(offset_size.first, offset_size.second);

            uint32_t width_ = std::max<uint32_t>(1, width >> mipLevel);
            uint32_t height_ = std::max<uint32_t>(1, height >> mipLevel);

            if(mipLevel) {
                auto surfInfo2 = getSurfaceInfo(GX2SurfaceFormat(dds.format_), dds.header.width, dds.header.height, 1, GX2SurfaceDim(1), tileMode, GX2AAMode(0), mipLevel);
                if(mipLevel == 1) {
                    mipOffsets_.push_back(surfInfo2.surfSize);
                }
                else {
                    mipOffsets_.push_back(mipSize);
                }
            }

            data_ += std::string(surfInfo.surfSize - offset_size.second, '\0');
            std::string dataAlignBytes(roundUp(mipSize, surfInfo.baseAlign) - mipSize, '\0');

            if(mipLevel) {
                mipSize += surfInfo.surfSize + dataAlignBytes.size();
            }

            if(mipSize > mipData.size()) {
                mipSize -= surfInfo.surfSize + dataAlignBytes.size();
                mipLevel -= 1;
                break;
            }

            result.push_back(dataAlignBytes + swizzleSurf(width_, height_, 1, GX2SurfaceFormat(dds.format_), aaMode, use, surfInfo.tileMode, s, surfInfo.pitch, surfInfo.bpp, 0, 0, data_, true));
        }
        
        dimension = GX2SurfaceDim(1);
        width = dds.header.width;
        height = dds.header.height;
        depth = 1;
        mipCount = mipLevel + 1;
        format = GX2SurfaceFormat(dds.format_);
        aaMode = GX2AAMode(0);
        use = GX2SurfaceUse(1);
        this->tileMode = tileMode;
        swizzle = s;
        alignment = surfInfo.baseAlign;
        pitch = surfInfo.pitch;
        compSel = dds.compSel;
        mipOffsets = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        for(size_t i = 0; i < mipOffsets_.size(); i++) {
            mipOffsets[i] = mipOffsets_[i];
        }
        data = result[0];
        result.erase(result.begin());
        for(const std::string& mip : result) {
            mipData += mip;
        }

        return FTEXError::NONE;
    }

    FTEXError FTEXFile::writeToStream(std::ostream& out) {
        //doesnt support updating offsets/lengths, would mess up offsets in other (unsupported) file sections
        out.write(magicFTEX, 4);
        
        viewMipFirst = 0;
        viewMipCount = mipCount;
        viewSliceFirst = 0;
        viewSliceCount = 1;
        uint32_t imageSize = Utility::Endian::toPlatform(eType::Big, static_cast<uint32_t>(data.size()));
        uint32_t imagePtr = 0;
        uint32_t mipDataSize = Utility::Endian::toPlatform(eType::Big, static_cast<uint32_t>(mipData.size()));
        uint32_t mipPtr = 0;
        
        Utility::Endian::toPlatform_inplace(eType::Big, dimension);
        Utility::Endian::toPlatform_inplace(eType::Big, width);
        Utility::Endian::toPlatform_inplace(eType::Big, height);
        Utility::Endian::toPlatform_inplace(eType::Big, depth);
        Utility::Endian::toPlatform_inplace(eType::Big, mipCount);
        Utility::Endian::toPlatform_inplace(eType::Big, format);
        Utility::Endian::toPlatform_inplace(eType::Big, aaMode);
        Utility::Endian::toPlatform_inplace(eType::Big, use);
        Utility::Endian::toPlatform_inplace(eType::Big, tileMode);
        Utility::Endian::toPlatform_inplace(eType::Big, swizzle);
        Utility::Endian::toPlatform_inplace(eType::Big, alignment);
        Utility::Endian::toPlatform_inplace(eType::Big, pitch);

        out.write(reinterpret_cast<const char*>(&dimension), sizeof(dimension));
        out.write(reinterpret_cast<const char*>(&width), sizeof(width));
        out.write(reinterpret_cast<const char*>(&height), sizeof(height));
        out.write(reinterpret_cast<const char*>(&depth), sizeof(depth));
        out.write(reinterpret_cast<const char*>(&mipCount), sizeof(mipCount));
        out.write(reinterpret_cast<const char*>(&format), sizeof(format));
        out.write(reinterpret_cast<const char*>(&aaMode), sizeof(aaMode));
        out.write(reinterpret_cast<const char*>(&use), sizeof(use));
        out.write(reinterpret_cast<const char*>(&imageSize), sizeof(imageSize));
        out.write(reinterpret_cast<const char*>(&imagePtr), sizeof(imagePtr));
        out.write(reinterpret_cast<const char*>(&mipDataSize), sizeof(mipDataSize));
        out.write(reinterpret_cast<const char*>(&mipPtr), sizeof(mipPtr));
        out.write(reinterpret_cast<const char*>(&tileMode), sizeof(tileMode));
        out.write(reinterpret_cast<const char*>(&swizzle), sizeof(swizzle));
        out.write(reinterpret_cast<const char*>(&alignment), sizeof(alignment));
        out.write(reinterpret_cast<const char*>(&pitch), sizeof(pitch));
        for(auto mipOffset : mipOffsets) {
            Utility::Endian::toPlatform_inplace(eType::Big, mipOffset);
            out.write(reinterpret_cast<const char*>(&mipOffset), sizeof(mipOffset));
        }
        Utility::Endian::toPlatform_inplace(eType::Big, viewMipFirst);
        Utility::Endian::toPlatform_inplace(eType::Big, viewMipCount);
        Utility::Endian::toPlatform_inplace(eType::Big, viewSliceFirst);
        Utility::Endian::toPlatform_inplace(eType::Big, viewSliceCount);

        out.write(reinterpret_cast<const char*>(&viewMipFirst), sizeof(viewMipFirst));
        out.write(reinterpret_cast<const char*>(&viewMipCount), sizeof(viewMipCount));
        out.write(reinterpret_cast<const char*>(&viewSliceFirst), sizeof(viewSliceFirst));
        out.write(reinterpret_cast<const char*>(&viewSliceCount), sizeof(viewSliceCount));
        out.write(reinterpret_cast<const char*>(&compSel), 4);

        out.seekp(dataOffset.offset, std::ios::beg);
        out.write(&data[0], data.size());

        if(mipOffset.isRelNonzero()) {
            out.seekp(mipOffset.offset, std::ios::beg);
            out.write(&mipData[0], mipData.size());
        }

        return FTEXError::NONE;
    }
}
