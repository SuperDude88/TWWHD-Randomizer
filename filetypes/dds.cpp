#include "dds.hpp"

#include <unordered_map>
#include <cstring>
#include <algorithm>
#include <type_traits>

#include <utility/common.hpp>
#include <utility/endian.hpp>
#include <utility/file.hpp>
#include <filetypes/texture/formconv.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;
using namespace std::literals::string_literals;

static const std::unordered_map<uint32_t, uint8_t> abgr8_masks = {{0xff, 0}, {0xff00, 1}, {0xff0000, 2}, {0xff000000, 3}, {0, 5}};
static const std::unordered_map<uint32_t, uint8_t> bgr8_masks = {{0xff, 0}, {0xff00, 1}, {0xff0000, 2}, {0, 5}};
static const std::unordered_map<uint32_t, uint8_t> a2rgb10_masks = {{0x3ff00000, 0}, {0xffc00, 1}, {0x3ff, 2}, {0xc0000000, 3}, {0, 5}};
static const std::unordered_map<uint32_t, uint8_t> bgr565_masks = {{0x1f, 0}, {0x7e0, 1}, {0xf800, 2}, {0, 5}};
static const std::unordered_map<uint32_t, uint8_t> a1bgr5_masks = {{0x1f, 0}, {0x3e0, 1}, {0x7c00, 2}, {0x8000, 3}, {0, 5}};
static const std::unordered_map<uint32_t, uint8_t> abgr4_masks = {{0xf, 0}, {0xf0, 1}, {0xf00, 2}, {0xf000, 3}, {0, 5}};
static const std::unordered_map<uint32_t, uint8_t> l8_masks = {{0xff, 0}, {0, 5}};
static const std::unordered_map<uint32_t, uint8_t> a8l8_masks = {{0xff, 0}, {0xff00, 1}, {0, 5}};
static const std::unordered_map<uint32_t, uint8_t> a4l4_masks = {{0xf, 0}, {0xf0, 1}, {0, 5}};



// This might be super unsafe, no idea
template<typename T> requires std::is_enum_v<T>
T operator |(T a, T b) {
    return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) | static_cast<std::underlying_type_t<T>>(b));
}

uint32_t getMipSize(uint32_t width, uint32_t height, uint32_t bpp, uint32_t numMips, bool compressed) {
    uint32_t size = 0;
    for(uint32_t i = 0; i < numMips; i++) {
        uint32_t level = i + 1;
        if(compressed) {
            size += ((std::max<uint32_t>(1, width >> level) + 3) >> 2) * ((std::max<uint32_t>(1, height >> level) + 3) >> 2) * bpp;
        }
        else {
            size += std::max<uint32_t>(1, width >> level) * std::max<uint32_t>(1, height >> level) * bpp;
        }
    }

    return size;
}

namespace FileTypes {

    const char* DDSErrorGetName(DDSError err) {
        switch(err) {
            case DDSError::NONE:
                return "NONE";
	        case DDSError::REACHED_EOF:
                return "REACHED_EOF";
	        case DDSError::COULD_NOT_OPEN:
                return "COULD_NOT_OPEN";
	        case DDSError::NOT_DDS:
                return "NOT_DDS";
            case DDSError::UNSUPPORTED_FORMAT:
                return "UNSUPPORTED_FORMAT";
            case DDSError::UNEXPECTED_VALUE:
                return "UNEXPECTED_VALUE";
            case DDSError::INVALID_TEXTURE:
                return "INVALID_TEXTURE";
	        default:
                return "UNKNOWN";
        }
    }

	void DDSFile::initNew() {

	}

	DDSFile DDSFile::createNew() {
		DDSFile newDDS{};
		newDDS.initNew();
		return newDDS;
    }

    DDSError DDSFile::loadFromBinary(std::istream& dds, const bool SRGB) {
        if(!dds.read(reinterpret_cast<char*>(&header.magicDDS), sizeof(header.magicDDS))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(std::memcmp(header.magicDDS, "DDS ", 4) != 0) {
			LOG_ERR_AND_RETURN(DDSError::NOT_DDS);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.headerSize_0x7C), sizeof(header.headerSize_0x7C))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.flags), sizeof(header.flags))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.height), sizeof(header.height))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.width), sizeof(header.width))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.size), sizeof(header.size))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.depth), sizeof(header.depth))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.numMips), sizeof(header.numMips))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
        for(uint32_t& item : header.reserved) {
		    if(!dds.read(reinterpret_cast<char*>(&item), sizeof(item))) {
		    	LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		    }
        }
		if(!dds.read(reinterpret_cast<char*>(&header.pixelFormat.headerSize_0x20), sizeof(header.pixelFormat.headerSize_0x20))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.pixelFormat.flags), sizeof(header.pixelFormat.flags))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.pixelFormat.fourcc), sizeof(header.pixelFormat.fourcc))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.pixelFormat.RGBBitCount), sizeof(header.pixelFormat.RGBBitCount))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.pixelFormat.RBitMask), sizeof(header.pixelFormat.RBitMask))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.pixelFormat.GBitMask), sizeof(header.pixelFormat.GBitMask))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
        }
		if(!dds.read(reinterpret_cast<char*>(&header.pixelFormat.BBitMask), sizeof(header.pixelFormat.BBitMask))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.pixelFormat.ABitMask), sizeof(header.pixelFormat.ABitMask))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.caps), sizeof(header.caps))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.caps2), sizeof(header.caps2))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.caps3), sizeof(header.caps3))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.caps4), sizeof(header.caps4))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}
		if(!dds.read(reinterpret_cast<char*>(&header.reserved2), sizeof(header.reserved2))) {
			LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}

        Utility::Endian::toPlatform_inplace(eType::Little, header.headerSize_0x7C);
        header.flags = static_cast<DDSFlags>(Utility::Endian::toPlatform(eType::Little, static_cast<uint32_t>(header.flags)));
        Utility::Endian::toPlatform_inplace(eType::Little, header.height);
        Utility::Endian::toPlatform_inplace(eType::Little, header.width);
        Utility::Endian::toPlatform_inplace(eType::Little, header.size);
        Utility::Endian::toPlatform_inplace(eType::Little, header.depth);
        Utility::Endian::toPlatform_inplace(eType::Little, header.numMips);
        Utility::Endian::toPlatform_inplace(eType::Little, header.pixelFormat.headerSize_0x20);
        header.pixelFormat.flags = static_cast<PixelFormatFlags>(Utility::Endian::toPlatform(eType::Little, static_cast<uint32_t>(header.pixelFormat.flags)));
        Utility::Endian::toPlatform_inplace(eType::Little, header.pixelFormat.RGBBitCount);
        Utility::Endian::toPlatform_inplace(eType::Little, header.pixelFormat.RBitMask);
        Utility::Endian::toPlatform_inplace(eType::Little, header.pixelFormat.GBitMask);
        Utility::Endian::toPlatform_inplace(eType::Little, header.pixelFormat.BBitMask);
        Utility::Endian::toPlatform_inplace(eType::Little, header.pixelFormat.ABitMask);
        header.caps = static_cast<DDSCaps>(Utility::Endian::toPlatform(eType::Little, static_cast<uint32_t>(header.caps)));
        header.caps2 = static_cast<DDSCaps2>(Utility::Endian::toPlatform(eType::Little, static_cast<uint32_t>(header.caps2)));
        Utility::Endian::toPlatform_inplace(eType::Little, header.caps3);
        Utility::Endian::toPlatform_inplace(eType::Little, header.caps4);
        Utility::Endian::toPlatform_inplace(eType::Little, header.reserved2);

        if(header.headerSize_0x7C != 0x7C) LOG_ERR_AND_RETURN(DDSError::UNEXPECTED_VALUE);
        if(header.pixelFormat.headerSize_0x20 != 0x20) LOG_ERR_AND_RETURN(DDSError::UNEXPECTED_VALUE);
        if(header.caps != DDSCaps::TEXTURE && header.caps != DDSCaps::ALL) LOG_ERR_AND_RETURN(DDSError::INVALID_TEXTURE);

        uint32_t bpp = header.pixelFormat.RGBBitCount >> 3;
        bool compressed = false;
        bool luminance = false;
        bool rgb = false;
        bool has_alpha = false;

        const PixelFormatFlags& pflags = header.pixelFormat.flags;
        if(pflags == PixelFormatFlags::FOURCC) compressed = true;
        else if(pflags == PixelFormatFlags::LUMINANCE || pflags == PixelFormatFlags::ALPHA) luminance = true;
        else if(pflags == (PixelFormatFlags::LUMINANCE | PixelFormatFlags::ALPHAPIXELS)) luminance = has_alpha = true;
        else if(pflags == PixelFormatFlags::RGB) rgb = true;
        else if(pflags == (PixelFormatFlags::RGB | PixelFormatFlags::ALPHAPIXELS)) rgb = has_alpha = true;
        else {
            LOG_ERR_AND_RETURN(DDSError::INVALID_TEXTURE);
        }

        format_ = 0;
        compSel = {0, 1, 2, 3};
    
        uint32_t headSize;
        if(std::strncmp(header.pixelFormat.fourcc, "DX10", 4) == 0) {
            if(!compressed) {
                LOG_ERR_AND_RETURN(DDSError::UNSUPPORTED_FORMAT);
            }

            headSize = 0x94;
        }
        else {
            headSize = 0x80;
        }

        uint32_t mipSize;
        if(compressed) {
            const auto& fourCC = header.pixelFormat.fourcc;
            if(std::strncmp(fourCC, "ETC1", 4) == 0) {
                format_ = 0x31;
                bpp = 8;
            }
            else if(std::strncmp(fourCC, "DXT1", 4) == 0) {
                format_ = 0x31;
                if(SRGB) format_ = 0x431;
                bpp = 8;
            }
            else if(std::strncmp(fourCC, "DXT3", 4) == 0) {
                format_ = 0x32;
                if(SRGB) format_ = 0x432;
                bpp = 16;
            }
            else if(std::strncmp(fourCC, "DXT5", 4) == 0) {
                format_ = 0x33;
                if(SRGB) format_ = 0x433;
                bpp = 16;
            }
            else if(std::strncmp(fourCC, "BC4U", 4) == 0 || std::strncmp(fourCC, "ATI1", 4) == 0) {
                format_ = 0x34;
                bpp = 8;
            }
            else if(std::strncmp(fourCC, "BC4S", 4) == 0) {
                format_ = 0x234;
                bpp = 8;
            }
            else if(std::strncmp(fourCC, "BC5U", 4) == 0 || std::strncmp(fourCC, "ATI2", 4) == 0) {
                format_ = 0x35;
                bpp = 16;
            }
            else if(std::strncmp(fourCC, "BC5S", 4) == 0) {
                format_ = 0x235;
                bpp = 16;
            }
            else if(std::strncmp(fourCC, "DX10", 4) == 0) {
                std::string temp(20, '\0');
                dds.seekg(128, std::ios::beg);
                if(!dds.read(&temp[0], 20)) {
			        LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		        }

                if(temp == "\x50\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"s) {
                    format_ = 0x34;
                    bpp = 8;
                }
                else if (temp == "\x51\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"s) {
                    format_ = 0x234;
                    bpp = 8;
                }
                else if (temp == "\x53\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"s) {
                    format_ = 0x35;
                    bpp = 16;
                }
                else if (temp == "\x54\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"s) {
                    format_ = 0x235;
                    bpp = 16;
                }
            }

            size = ((header.width + 3) >> 2) * ((header.height + 3) >> 2) * bpp;
        }
        else {
            const auto& ch0 = header.pixelFormat.RBitMask;
            const auto& ch1 = header.pixelFormat.GBitMask;
            const auto& ch2 = header.pixelFormat.BBitMask;
            const auto& ch3 = header.pixelFormat.ABitMask;

            if(luminance) {
                if(has_alpha) {
                    if(a8l8_masks.contains(ch0) && a8l8_masks.contains(ch1) && a8l8_masks.contains(ch2) && a8l8_masks.contains(ch3) && bpp == 2) {
                        format_ = 7;
                        compSel = {a8l8_masks.at(ch0), a8l8_masks.at(ch1), a8l8_masks.at(ch2), a8l8_masks.at(ch3)};
                    }
                    else if(a4l4_masks.contains(ch0) && a4l4_masks.contains(ch1) && a4l4_masks.contains(ch2) && a4l4_masks.contains(ch3) && bpp == 1) {
                        format_ = 2;
                        compSel = {a4l4_masks.at(ch0), a4l4_masks.at(ch1), a4l4_masks.at(ch2), a4l4_masks.at(ch3)};
                    }
                }
                else {
                    if(l8_masks.contains(ch0) && l8_masks.contains(ch1) && l8_masks.contains(ch2) && l8_masks.contains(ch3) && bpp == 1) {
                        format_ = 1;
                        compSel = {l8_masks.at(ch0), l8_masks.at(ch1), l8_masks.at(ch2), l8_masks.at(ch3)};
                    }
                }
            }
            else if(rgb) {
                if(has_alpha) {
                    if(bpp == 4) {
                        if(abgr8_masks.contains(ch0) && abgr8_masks.contains(ch1) && abgr8_masks.contains(ch2) && abgr8_masks.contains(ch3)) {
                            format_ = 0x1A;
                            if(SRGB) format_ = 0x41A;
                            compSel = {abgr8_masks.at(ch0), abgr8_masks.at(ch1), abgr8_masks.at(ch2), abgr8_masks.at(ch3)};
                        }
                        else if(a2rgb10_masks.contains(ch0) && a2rgb10_masks.contains(ch1) && a2rgb10_masks.contains(ch2) && a2rgb10_masks.contains(ch3)) {
                            format_ = 0x19;
                            compSel = {a2rgb10_masks.at(ch0), a2rgb10_masks.at(ch1), a2rgb10_masks.at(ch2), a2rgb10_masks.at(ch3)};
                        }
                    }
                    else if (bpp == 2) {
                        if(a1bgr5_masks.contains(ch0) && a1bgr5_masks.contains(ch1) && a1bgr5_masks.contains(ch2) && a1bgr5_masks.contains(ch3)) {
                            format_ = 0xA;
                            compSel = {a1bgr5_masks.at(ch0), a1bgr5_masks.at(ch1), a1bgr5_masks.at(ch2), a1bgr5_masks.at(ch3)};
                        }
                        else if(abgr4_masks.contains(ch0) && abgr4_masks.contains(ch1) && abgr4_masks.contains(ch2) && abgr4_masks.contains(ch3)) {
                            format_ = 0xB;
                            compSel = {abgr4_masks.at(ch0), abgr4_masks.at(ch1), abgr4_masks.at(ch2), abgr4_masks.at(ch3)};
                        }
                    }
                }
                else {
                    if(bgr8_masks.contains(ch0) && bgr8_masks.contains(ch1) && bgr8_masks.contains(ch2) && ch3 == 0 && bpp == 3) {
                        format_ = 0x1A;
                        if(SRGB) format_ = 0x41A;
                        compSel = {bgr8_masks.at(ch0), bgr8_masks.at(ch1), bgr8_masks.at(ch2), 3};
                    }
                    else if(bgr565_masks.contains(ch0) && bgr565_masks.contains(ch1) && bgr565_masks.contains(ch2) && bgr565_masks.contains(ch3) && bpp == 2) {
                        format_ = 9;
                        compSel = {bgr565_masks.at(ch0), bgr565_masks.at(ch1), bgr565_masks.at(ch2), bgr565_masks.at(ch3)};
                    }
                }
            }

            size = header.width * header.height * bpp;
        }

        if(header.caps == DDSCaps::ALL) {
            header.numMips -= 1;
            mipSize = getMipSize(header.width, header.height, bpp, header.numMips, compressed);
        }
        else {
            header.numMips = 0;
            mipSize = 0;
        }

        dds.seekg(0, std::ios::end);
        if(dds.tellg() < headSize + size + mipSize) {
            LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
        }

        if(format_ == 0) LOG_ERR_AND_RETURN(DDSError::UNSUPPORTED_FORMAT);

        data.resize(size + mipSize);
        dds.seekg(headSize, std::ios::beg);
        if(!dds.read(&data[0], size + mipSize)) {
		    LOG_ERR_AND_RETURN(DDSError::REACHED_EOF);
		}

        if((format_ == 0x1A || format_ == 0x41A) && bpp == 3) {
            data = rgb8torgbx8(data); // convert
            bpp += 1;
            size = header.width * header.height * bpp;
        }

        return DDSError::NONE;
    }

    DDSError DDSFile::loadFromFile(const fspath& filePath, const bool SRGB) {
        std::string fileData;
        if (Utility::getFileContents(filePath, fileData, true)) LOG_ERR_AND_RETURN(DDSError::COULD_NOT_OPEN);

        std::istringstream file(fileData, std::ios::binary);
		return loadFromBinary(file, SRGB);
    }
    
	DDSError DDSFile::writeToStream(std::ostream& out) {
        Utility::Endian::toPlatform_inplace(eType::Little, header.headerSize_0x7C);
        Utility::Endian::toPlatform_inplace(eType::Little, header.flags);
        Utility::Endian::toPlatform_inplace(eType::Little, header.height);
        Utility::Endian::toPlatform_inplace(eType::Little, header.width);
        Utility::Endian::toPlatform_inplace(eType::Little, header.size);
        Utility::Endian::toPlatform_inplace(eType::Little, header.depth);
        Utility::Endian::toPlatform_inplace(eType::Little, header.numMips);
        Utility::Endian::toPlatform_inplace(eType::Little, header.pixelFormat.headerSize_0x20);
        Utility::Endian::toPlatform_inplace(eType::Little, header.pixelFormat.flags);
        Utility::Endian::toPlatform_inplace(eType::Little, header.pixelFormat.RGBBitCount);
        Utility::Endian::toPlatform_inplace(eType::Little, header.pixelFormat.RBitMask);
        Utility::Endian::toPlatform_inplace(eType::Little, header.pixelFormat.GBitMask);
        Utility::Endian::toPlatform_inplace(eType::Little, header.pixelFormat.BBitMask);
        Utility::Endian::toPlatform_inplace(eType::Little, header.pixelFormat.ABitMask);
        Utility::Endian::toPlatform_inplace(eType::Little, header.caps);
        Utility::Endian::toPlatform_inplace(eType::Little, header.caps2);
        Utility::Endian::toPlatform_inplace(eType::Little, header.caps3);
        Utility::Endian::toPlatform_inplace(eType::Little, header.caps4);
        Utility::Endian::toPlatform_inplace(eType::Little, header.reserved2);

        out.write(header.magicDDS, 4);
		out.write(reinterpret_cast<const char*>(&header.headerSize_0x7C), sizeof(header.headerSize_0x7C));
		out.write(reinterpret_cast<const char*>(&header.flags), 4);
		out.write(reinterpret_cast<const char*>(&header.height), sizeof(header.height));
		out.write(reinterpret_cast<const char*>(&header.width), sizeof(header.width));
		out.write(reinterpret_cast<const char*>(&header.size), sizeof(header.size));
		out.write(reinterpret_cast<const char*>(&header.depth), sizeof(header.depth));
		out.write(reinterpret_cast<const char*>(&header.numMips), sizeof(header.numMips));
        for(const uint32_t& item : header.reserved) {
		    out.write(reinterpret_cast<const char*>(&item), sizeof(item));
        }
		out.write(reinterpret_cast<const char*>(&header.pixelFormat.headerSize_0x20), sizeof(header.pixelFormat.headerSize_0x20));
		out.write(reinterpret_cast<const char*>(&header.pixelFormat.flags), sizeof(header.pixelFormat.flags));
		out.write(reinterpret_cast<const char*>(&header.pixelFormat.fourcc), sizeof(header.pixelFormat.fourcc));
		out.write(reinterpret_cast<const char*>(&header.pixelFormat.RGBBitCount), sizeof(header.pixelFormat.RGBBitCount));
		out.write(reinterpret_cast<const char*>(&header.pixelFormat.RBitMask), sizeof(header.pixelFormat.RBitMask));
		out.write(reinterpret_cast<const char*>(&header.pixelFormat.GBitMask), sizeof(header.pixelFormat.GBitMask));
		out.write(reinterpret_cast<const char*>(&header.pixelFormat.BBitMask), sizeof(header.pixelFormat.BBitMask));
		out.write(reinterpret_cast<const char*>(&header.pixelFormat.ABitMask), sizeof(header.pixelFormat.ABitMask));
		out.write(reinterpret_cast<const char*>(&header.caps), sizeof(header.caps));
		out.write(reinterpret_cast<const char*>(&header.caps2), sizeof(header.caps2));
		out.write(reinterpret_cast<const char*>(&header.caps3), sizeof(header.caps3));
		out.write(reinterpret_cast<const char*>(&header.caps4), sizeof(header.caps4));
		out.write(reinterpret_cast<const char*>(&header.reserved2), sizeof(header.reserved2));
		padToLen(out, 0x10);

        out.write(&data[0], data.size());

        return DDSError::NONE;
    }

	DDSError DDSFile::writeToFile(const fspath& outFilePath) {
        std::ofstream file(outFilePath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERR_AND_RETURN(DDSError::COULD_NOT_OPEN);
		}
		return writeToStream(file);
    }
}
