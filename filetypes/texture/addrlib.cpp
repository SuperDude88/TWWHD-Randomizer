#include "addrlib.hpp"

#include <array>
#include <unordered_set>


static const std::unordered_set BCn_formats = {
    GX2_SURFACE_FORMAT_UNORM_BC1, GX2_SURFACE_FORMAT_SRGB_BC1, GX2_SURFACE_FORMAT_UNORM_BC2, GX2_SURFACE_FORMAT_SRGB_BC2,
    GX2_SURFACE_FORMAT_UNORM_BC3, GX2_SURFACE_FORMAT_SRGB_BC3, GX2_SURFACE_FORMAT_UNORM_BC4, GX2_SURFACE_FORMAT_SNORM_BC4,
    GX2_SURFACE_FORMAT_UNORM_BC5, GX2_SURFACE_FORMAT_SNORM_BC5,
};

const std::array<uint8_t, 256> formatHwInfo = {
    0x00, 0x00, 0x00, 0x01, 0x08, 0x03, 0x00, 0x01, 0x08, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00, 0x10, 0x03, 0x00, 0x01, 0x10, 0x03, 0x00, 0x01,
    0x10, 0x0B, 0x00, 0x01, 0x10, 0x01, 0x00, 0x01, 0x10, 0x03, 0x00, 0x01, 0x10, 0x03, 0x00, 0x01,
    0x10, 0x03, 0x00, 0x01, 0x20, 0x03, 0x00, 0x00, 0x20, 0x07, 0x00, 0x00, 0x20, 0x03, 0x00, 0x00,
    0x20, 0x03, 0x00, 0x01, 0x20, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x20, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x01, 0x20, 0x0B, 0x00, 0x01, 0x20, 0x0B, 0x00, 0x01, 0x20, 0x0B, 0x00, 0x01,
    0x40, 0x05, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00,
    0x40, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00,
    0x10, 0x01, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x60, 0x01, 0x00, 0x00,
    0x60, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x01, 0x80, 0x01, 0x00, 0x01, 0x80, 0x01, 0x00, 0x01,
    0x40, 0x01, 0x00, 0x01, 0x80, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const std::array<uint8_t, 256> formatExInfo = {
    0x00, 0x01, 0x01, 0x03, 0x08, 0x01, 0x01, 0x03, 0x08, 0x01, 0x01, 0x03, 0x08, 0x01, 0x01, 0x03,
    0x00, 0x01, 0x01, 0x03, 0x10, 0x01, 0x01, 0x03, 0x10, 0x01, 0x01, 0x03, 0x10, 0x01, 0x01, 0x03,
    0x10, 0x01, 0x01, 0x03, 0x10, 0x01, 0x01, 0x03, 0x10, 0x01, 0x01, 0x03, 0x10, 0x01, 0x01, 0x03,
    0x10, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03,
    0x20, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03,
    0x20, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03,
    0x20, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03,
    0x40, 0x01, 0x01, 0x03, 0x40, 0x01, 0x01, 0x03, 0x40, 0x01, 0x01, 0x03, 0x40, 0x01, 0x01, 0x03,
    0x40, 0x01, 0x01, 0x03, 0x00, 0x01, 0x01, 0x03, 0x80, 0x01, 0x01, 0x03, 0x80, 0x01, 0x01, 0x03,
    0x00, 0x01, 0x01, 0x03, 0x01, 0x08, 0x01, 0x05, 0x01, 0x08, 0x01, 0x06, 0x10, 0x01, 0x01, 0x07,
    0x10, 0x01, 0x01, 0x08, 0x20, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03, 0x20, 0x01, 0x01, 0x03,
    0x18, 0x03, 0x01, 0x04, 0x30, 0x03, 0x01, 0x04, 0x30, 0x03, 0x01, 0x04, 0x60, 0x03, 0x01, 0x04,
    0x60, 0x03, 0x01, 0x04, 0x40, 0x04, 0x04, 0x09, 0x80, 0x04, 0x04, 0x0A, 0x80, 0x04, 0x04, 0x0B,
    0x40, 0x04, 0x04, 0x0C, 0x40, 0x04, 0x04, 0x0D, 0x40, 0x04, 0x04, 0x0D, 0x40, 0x04, 0x04, 0x0D,
    0x00, 0x01, 0x01, 0x03, 0x00, 0x01, 0x01, 0x03, 0x00, 0x01, 0x01, 0x03, 0x00, 0x01, 0x01, 0x03,
    0x00, 0x01, 0x01, 0x03, 0x00, 0x01, 0x01, 0x03, 0x40, 0x01, 0x01, 0x03, 0x00, 0x01, 0x01, 0x03,
};

std::array<uint8_t, 10> bankSwapOrder = {0, 1, 3, 2, 6, 7, 5, 4, 0, 0};



GX2TileMode GX2TileModeToAddrTileMode(GX2TileMode tileMode) {
    if(tileMode == GX2TileMode::GX2_TILE_MODE_LINEAR_SPECIAL) {
        return GX2TileMode::GX2_TILE_MODE_DEFAULT;
    }

    return tileMode;
}

uint8_t surfaceGetBitsPerPixel(uint32_t surfaceFormat) {
    return formatHwInfo[(surfaceFormat & 0x3F) * 4];
}

uint8_t computeSurfaceThickness(GX2TileMode tileMode) {
    if(tileMode == 3 || tileMode == 7 || tileMode == 11 || tileMode == 13 || tileMode == 15) {
        return 4;
    }
    else if(tileMode == 16 || tileMode == 17) {
        return 8;
    }

    return 1;
}

uint32_t computePixelIndexWithinMicroTile(uint32_t x, uint32_t y, uint32_t z, uint32_t bpp, GX2TileMode tileMode, bool isDepth) {
    uint32_t pixelBit0 = 0;
    uint32_t pixelBit1 = 0;
    uint32_t pixelBit2 = 0;
    uint32_t pixelBit3 = 0;
    uint32_t pixelBit4 = 0;
    uint32_t pixelBit5 = 0;
    uint32_t pixelBit6 = 0;
    uint32_t pixelBit7 = 0;
    uint32_t pixelBit8 = 0;

    uint8_t thickness = computeSurfaceThickness(tileMode);

    if(isDepth) {
        pixelBit0 = x & 1;
        pixelBit1 = y & 1;
        pixelBit2 = (x & 2) >> 1;
        pixelBit3 = (y & 2) >> 1;
        pixelBit4 = (x & 4) >> 2;
        pixelBit5 = (y & 4) >> 2;
    }
    else {
        if (bpp == 8) {
            pixelBit0 = x & 1;
            pixelBit1 = (x & 2) >> 1;
            pixelBit2 = (x & 4) >> 2;
            pixelBit3 = (y & 2) >> 1;
            pixelBit4 = y & 1;
            pixelBit5 = (y & 4) >> 2;
        }
        else if (bpp == 0x10) {
            pixelBit0 = x & 1;
            pixelBit1 = (x & 2) >> 1;
            pixelBit2 = (x & 4) >> 2;
            pixelBit3 = y & 1;
            pixelBit4 = (y & 2) >> 1;
            pixelBit5 = (y & 4) >> 2;
        }
        else if (bpp == 0x20 || bpp == 0x60) {
            pixelBit0 = x & 1;
            pixelBit1 = (x & 2) >> 1;
            pixelBit2 = y & 1;
            pixelBit3 = (x & 4) >> 2;
            pixelBit4 = (y & 2) >> 1;
            pixelBit5 = (y & 4) >> 2;
        }
        else if (bpp == 0x40) {
            pixelBit0 = x & 1;
            pixelBit1 = y & 1;
            pixelBit2 = (x & 2) >> 1;
            pixelBit3 = (x & 4) >> 2;
            pixelBit4 = (y & 2) >> 1;
            pixelBit5 = (y & 4) >> 2;
        }
        else if (bpp == 0x80) {
            pixelBit0 = y & 1;
            pixelBit1 = x & 1;
            pixelBit2 = (x & 2) >> 1;
            pixelBit3 = (x & 4) >> 2;
            pixelBit4 = (y & 2) >> 1;
            pixelBit5 = (y & 4) >> 2;
        }
        else {
            pixelBit0 = x & 1;
            pixelBit1 = (x & 2) >> 1;
            pixelBit2 = y & 1;
            pixelBit3 = (x & 4) >> 2;
            pixelBit4 = (y & 2) >> 1;
            pixelBit5 = (y & 4) >> 2;
        }
    }

    if(thickness > 1) {
        pixelBit6 = z & 1;
        pixelBit7 = (z & 2) >> 1;
    }
    if(thickness == 8) {
        pixelBit8 = (z & 4) >> 2;
    }

    return (pixelBit8 << 8) | (pixelBit7 << 7) | (pixelBit6 << 6) | 32 * pixelBit5 | 16 * pixelBit4 | 8 * pixelBit3 | 4 * pixelBit2 | pixelBit0 | 2 * pixelBit1;
}

uint32_t computePipeFromCoordWoRotation(uint32_t x, uint32_t y) {
    return ((y >> 3) ^ (x >> 3)) & 1;
}

uint32_t computeBankFromCoordWoRotation(uint32_t x, uint32_t y) {
    return (((y >> 5) ^ (x >> 3)) & 1) | (2 * (((y >> 4) ^ (x >> 4)) & 1));
}

uint8_t computeSurfaceRotationFromTileMode(GX2TileMode tileMode) {
    if(4 <= tileMode && tileMode <= 11) return 2;
    if(12 <= tileMode && tileMode <= 15) return 1;
    return 0;
}

bool isThickMacroTiled(GX2TileMode tileMode) {
    if(tileMode == 7 || tileMode == 11 || tileMode == 13 || tileMode == 15) return true;
    return false;
}

bool isBankSwappedTileMode(GX2TileMode tileMode) {
    if(tileMode == 8 || tileMode == 9 || tileMode == 10 || tileMode == 11 || tileMode == 14 || tileMode == 15) return true;
    return false;
}

uint8_t computeMacroTileAspectRatio(GX2TileMode tileMode) {
    if(tileMode == 5 || tileMode == 9) return 2;
    if(tileMode == 6 || tileMode == 10) return 4;
    return 1;
}

uint32_t computeSurfaceBankSwappedWidth(GX2TileMode tileMode, uint32_t bpp, uint32_t numSamples, uint32_t pitch) {
    if(isBankSwappedTileMode(tileMode) == 0) {
        return 0;
    }

    uint32_t bytesPerSample = 8 * bpp;
    uint32_t slicesPerTile;

    if(bytesPerSample != 0) {
        uint32_t samplesPerTile = 2048 / bytesPerSample;
        slicesPerTile = std::max<uint32_t>(1, numSamples / samplesPerTile);
    }
    else {
        slicesPerTile = 1;
    }

    if(isThickMacroTiled(tileMode) != 0) {
        numSamples = 4;
    }

    uint32_t bytesPerTileSlice = numSamples * bytesPerSample / slicesPerTile;

    uint8_t factor = computeMacroTileAspectRatio(tileMode);
    uint32_t swapTiles = std::max<uint32_t>(1, 128 / bpp);

    uint32_t swapWidth = swapTiles * 32;
    uint32_t heightBytes = numSamples * factor * bpp * 2 / slicesPerTile;
    uint32_t swapMax = 0x4000 / heightBytes;
    uint32_t swapMin = 256 / bytesPerTileSlice;
    uint32_t bankSwapWidth = std::min(swapMax, std::max(swapMin, swapWidth));
    while(bankSwapWidth >= 2 * pitch) {
        bankSwapWidth >>= 1;
    }

    return bankSwapWidth;
}

uint64_t computeSurfaceAddrFromCoordLinear(uint32_t x, uint32_t y, uint32_t slice, uint32_t sample, uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSlices) {
    uint64_t sliceOffset = pitch * height * (slice + sample * numSlices);
    return (y * pitch + x + sliceOffset) * bpp;
}

uint64_t computeSurfaceAddrFromCoordMicroTiled(uint32_t x, uint32_t y, uint32_t slice, uint32_t bpp, uint32_t pitch, uint32_t height, GX2TileMode tileMode, bool isDepth) {
    uint64_t microTileThickness = 1;
    if(tileMode == GX2TileMode::GX2_TILE_MODE_TILED_1D_THICK) microTileThickness = 4;

    uint64_t microTileBytes = (64 * microTileThickness * bpp + 7) / 8;
    uint64_t microTilesPerRow = pitch >> 3;
    uint64_t microTileIndexX = x >> 3;
    uint64_t microTileIndexY = y >> 3;
    uint64_t microTileIndexZ = slice / microTileThickness;

    uint64_t microTileOffset = microTileBytes * (microTileIndexX + microTileIndexY * microTilesPerRow);
    uint64_t sliceBytes = (pitch * height * microTileThickness * bpp + 7) / 8;
    uint64_t sliceOffset = microTileIndexZ * sliceBytes;

    uint64_t pixelIndex = computePixelIndexWithinMicroTile(x, y, slice, bpp, tileMode, isDepth);
    uint64_t pixelOffset = (bpp * pixelIndex) >> 3;

    return pixelOffset + microTileOffset + sliceOffset;
}

uint64_t computeSurfaceAddrFromCoordMacroTiled(uint32_t x, uint32_t y, uint32_t slice, uint32_t sample, uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSamples, GX2TileMode tileMode, bool isDepth, uint32_t pipeSwizzle, uint32_t bankSwizzle) {
    uint32_t microTileThickness = computeSurfaceThickness(tileMode);

    uint64_t microTileBits = numSamples * bpp * (microTileThickness * 64);
    uint64_t microTileBytes = (microTileBits + 7) / 8;

    uint64_t pixelIndex = computePixelIndexWithinMicroTile(x, y, slice, bpp, tileMode, isDepth);
    uint64_t bytesPerSample = microTileBytes / numSamples;

    uint64_t sampleOffset = 0;
    uint64_t pixelOffset = 0;
    if(isDepth) {
        sampleOffset = bpp * sample;
        pixelOffset = numSamples * bpp * pixelIndex;
    }
    else {
        sampleOffset = sample * (microTileBits / numSamples);
        pixelOffset = bpp * pixelIndex;
    }

    uint64_t elemOffset = pixelOffset + sampleOffset;

    uint64_t samplesPerSlice;
    uint64_t numSampleSplits;
    uint64_t sampleSlice;

    if(numSamples <= 1 || microTileBytes <= 2048) {
        samplesPerSlice = numSamples;
        numSampleSplits = 1;
        sampleSlice = 0;
    }
    else {
        samplesPerSlice = 2048 / bytesPerSample;
        numSampleSplits = numSamples / samplesPerSlice;
        numSamples = samplesPerSlice;

        uint64_t tileSliceBits = microTileBits / numSampleSplits;
        sampleSlice = elemOffset / tileSliceBits;
        elemOffset %= tileSliceBits;
    }

    elemOffset = (elemOffset + 7) / 8;

    uint64_t pipe = computePipeFromCoordWoRotation(x, y);
    uint64_t bank = computeBankFromCoordWoRotation(x, y);

    uint64_t swizzle_ = pipeSwizzle + 2 * bankSwizzle;
    uint64_t bankPipe = pipe + 2 * bank;
    uint64_t rotation = computeSurfaceRotationFromTileMode(tileMode);
    uint64_t sliceIn = slice;

    if(isThickMacroTiled(tileMode)) {
        sliceIn >>= 2;
    }

    bankPipe ^= 2 * sampleSlice * 3 ^ (swizzle_ + sliceIn * rotation);
    bankPipe %= 8;
    pipe = bankPipe % 2;
    bank = bankPipe / 2;

    uint64_t sliceBytes = (height * pitch * microTileThickness * bpp * numSamples + 7) / 8;
    uint64_t sliceOffset = sliceBytes * ((sampleSlice + numSampleSplits * slice) / microTileThickness);

    uint64_t macroTilePitch = 32;
    uint64_t macroTileHeight = 16;

    if(tileMode == GX2TileMode::GX2_TILE_MODE_TILED_2B_THIN2 || tileMode == GX2TileMode::GX2_TILE_MODE_TILED_2D_THIN2) {
        macroTilePitch = 16;
        macroTileHeight = 32;
    }
    else if(tileMode == GX2TileMode::GX2_TILE_MODE_TILED_2B_THIN4 || tileMode == GX2TileMode::GX2_TILE_MODE_TILED_2D_THIN4) {
        macroTilePitch = 8;
        macroTileHeight = 64;
    }

    uint64_t macroTilesPerRow = pitch / macroTilePitch;
    uint64_t macroTileBytes = (numSamples * microTileThickness * bpp * macroTileHeight * macroTilePitch + 7) / 8;
    uint64_t macroTileIndexX = x / macroTilePitch;
    uint64_t macroTileIndexY = y / macroTileHeight;
    uint64_t macroTileOffset = (macroTileIndexX + macroTilesPerRow * macroTileIndexY) * macroTileBytes;

    if(isBankSwappedTileMode(tileMode)) {
        uint32_t bankSwapWidth = computeSurfaceBankSwappedWidth(tileMode, bpp, numSamples, pitch);
        uint32_t swaindex = macroTilePitch * macroTileIndexX / bankSwapWidth;
        bank ^= bankSwapOrder[swaindex & 3];
    }

    uint64_t totalOffset = elemOffset + ((macroTileOffset + sliceOffset) >> 3);
    return (bank << 9) | (pipe << 8) | (totalOffset & 255) | ((totalOffset & -256) << 3);
}

std::string swizzleSurf(uint32_t width, uint32_t height, uint32_t depth, GX2SurfaceFormat format_, GX2AAMode aa, GX2SurfaceUse use, GX2TileMode tileMode, uint32_t swizzle_, uint32_t pitch, uint32_t bitsPerPixel, uint32_t slice, uint32_t sample, const std::string& data, bool swizzle) {
    uint32_t bytesPerPixel = bitsPerPixel / 8;

    std::string result(data.size(), '\0');

    if(BCn_formats.contains(format_)) {
        width = (width + 3) / 4;
        height = (height + 3) / 4;
    }

    uint32_t pipeSwizzle = (swizzle_ >> 8) & 1;
    uint32_t bankSwizzle = (swizzle_ >> 9) & 3;

    tileMode = GX2TileModeToAddrTileMode(tileMode);

    uint64_t pos;
    for(uint32_t y = 0; y < height; y++) {
        for(uint32_t x = 0; x < width; x++) {
            if(tileMode == GX2_TILE_MODE_DEFAULT || tileMode == GX2_TILE_MODE_LINEAR_ALIGNED) {
                pos = computeSurfaceAddrFromCoordLinear(x, y, slice, sample, bytesPerPixel, pitch, height, depth);
            }
            else if(tileMode == GX2_TILE_MODE_TILED_1D_THIN1 || tileMode == GX2_TILE_MODE_TILED_1D_THICK) {
                pos = computeSurfaceAddrFromCoordMicroTiled(x, y, slice, bitsPerPixel, pitch, height, tileMode, static_cast<bool>(use & 4));
            }
            else {
                pos = computeSurfaceAddrFromCoordMacroTiled(x, y, slice, sample, bitsPerPixel, pitch, height, 1 << aa, tileMode, static_cast<bool>(use & 4), pipeSwizzle, bankSwizzle);
            }

            uint32_t pos_ = (y * width + x) * bytesPerPixel;

            if((pos_ + bytesPerPixel <= data.size()) && (pos + bytesPerPixel <= data.size())) {
                if(swizzle == false) {
                    result.replace(pos_, bytesPerPixel, data.substr(pos, bytesPerPixel));
                }
                else {
                    result.replace(pos, bytesPerPixel, data.substr(pos_, bytesPerPixel));
                }
            }
        }
    }

    return result;
}

uint32_t powTwoAlign(uint32_t x, uint32_t align) {
    return ~(align - 1) & (x + align - 1);
}

uint32_t nextPow2(uint32_t dim) {
    uint32_t newDim = 1;
    if (dim <= 0x7FFFFFFF) {
        while (newDim < dim) {
            newDim *= 2;
        }
    }
    else {
        newDim = 0x80000000;
    }

    return newDim;
}

void getBitsPerPixel(uint32_t format_, uint8_t& bpp, uint8_t& expandX, uint8_t& expandY, uint8_t& elemMode) {
    uint32_t fmtIdx = format_ * 4;
    bpp = formatExInfo[fmtIdx];
    expandX = formatExInfo[fmtIdx + 1];
    expandY = formatExInfo[fmtIdx + 2];
    elemMode = formatExInfo[fmtIdx + 3];
}

uint32_t adjustSurfaceInfo(surfaceIn& in, uint8_t elemMode, uint8_t expandX, uint8_t expandY, uint8_t bpp, uint32_t width, uint32_t height) {
    uint32_t bBCnFormat = 0;
    if (bpp && 9 <= elemMode && elemMode <= 13 ) {
        bBCnFormat = 1;
    }
    if (width && height) {
        if (expandX > 1 || expandY > 1) {
            uint32_t widtha;
            uint32_t heighta;
            if (elemMode == 4) {
                widtha = expandX * width;
                heighta = expandY * height;
            }
            else if (bBCnFormat) {
                widtha = width / expandX;
                heighta = height / expandY;
            }
            else {
                widtha = (width + expandX - 1) / expandX;
                heighta = (height + expandY - 1) / expandY;
            }
            in.width = std::max<uint32_t>(1, widtha);
            in.height = std::max<uint32_t>(1, heighta);
        }
    }
    if (bpp) {
        if (elemMode == 4) {
            in.bpp = bpp / expandX / expandY;
        }
        else if (elemMode == 5 || elemMode == 6) {
            in.bpp = expandY * expandX * bpp;
        }
        else if (elemMode == 7 || elemMode == 8) {
            in.bpp = bpp;
        }
        else if (elemMode == 9 || elemMode == 12) {
            in.bpp = 64;
        }
        else if (elemMode == 10 || elemMode == 11 || elemMode == 13) {
            in.bpp = 128;
        }
        else if (elemMode <= 3) {
            in.bpp = bpp;
        }
        else {
            in.bpp = bpp;
        }
        return in.bpp;
    }
    return 0;
}

uint32_t hwlComputeMipLevel(surfaceIn& in) {
    uint32_t handled = 0;

    if(49 <= in.format && in.format <= 55) {
        if(in.mipLevel) {
            auto width = in.width;
            auto height = in.height;
            auto slices = in.numSlices;

            if((in.flags >> 12) & 1) {
                auto widtha = width >> in.mipLevel;
                auto heighta = height >> in.mipLevel;

                if(!((in.flags >> 4) & 1)) {
                    slices >>= in.mipLevel;
                }

                width = std::max<uint32_t>(1, widtha);
                height = std::max<uint32_t>(1, heighta);
                slices = std::max<uint32_t>(1, slices);
            }

            in.width = nextPow2(width);
            in.height = nextPow2(height);
            in.numSlices = slices;
        }

        handled = 1;
    }

    return handled;
}

void computeMipLevel(surfaceIn& in) {
    uint32_t hwlHandled = 0;

    if((49 <= in.format) && (in.format <= 55) && (!in.mipLevel || ((in.flags >> 12) & 1))) {
        in.width = powTwoAlign(in.width, 4);
        in.height = powTwoAlign(in.height, 4);
    }

    hwlHandled = hwlComputeMipLevel(in);
    if(!hwlHandled && in.mipLevel && ((in.flags >> 12) & 1)) {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t slices = 0;
        width = std::max<uint32_t>(1, in.width >> in.mipLevel);
        height = std::max<uint32_t>(1, in.height >> in.mipLevel);
        slices = std::max<uint32_t>(1, in.numSlices);

        if(!((in.flags >> 4) & 1)) {
            slices = std::max<uint32_t>(1, slices >> in.mipLevel);
        }

        if(in.format != 47 && in.format != 48) {
            width = nextPow2(width);
            height = nextPow2(height);
            slices = nextPow2(slices);
        }

        in.width = width;
        in.height = height;
        in.numSlices = slices;
    }
}

GX2TileMode convertToNonBankSwappedMode(GX2TileMode tileMode) {
    switch (tileMode) {
        case GX2_TILE_MODE_TILED_2B_THIN1:
            return GX2_TILE_MODE_TILED_2D_THIN1;
        case GX2_TILE_MODE_TILED_2B_THIN2:
            return GX2_TILE_MODE_TILED_2D_THIN2;
        case GX2_TILE_MODE_TILED_2B_THIN4:
            return GX2_TILE_MODE_TILED_2D_THIN4;
        case GX2_TILE_MODE_TILED_2B_THICK:
            return GX2_TILE_MODE_TILED_2D_THICK;
        case GX2_TILE_MODE_TILED_3B_THIN1:
            return GX2_TILE_MODE_TILED_3D_THIN1;
        case GX2_TILE_MODE_TILED_3B_THICK:
            return GX2_TILE_MODE_TILED_3D_THICK;
        default:
            return tileMode;
    }
}

uint32_t computeSurfaceTileSlices(GX2TileMode tileMode, uint32_t bpp, uint32_t numSamples) {
    uint32_t bytePerSample = ((bpp << 6) + 7) >> 3;
    uint32_t tileSlices = 1;

    if(computeSurfaceThickness(tileMode) > 1) {
        numSamples = 4;
    }

    if(bytePerSample != 0) {
        if(const uint32_t samplePerTile = 2048 / bytePerSample; samplePerTile != 0) {
            tileSlices = std::max<uint32_t>(1, numSamples / samplePerTile);
        }
    }

    return tileSlices;
}

uint32_t computeSurfaceMipLevelTileMode(GX2TileMode baseTileMode, uint32_t bpp, uint32_t level, uint32_t width, uint32_t height, uint32_t numSlices, uint32_t numSamples, bool isDepth, bool noRecursive) {
    uint32_t widthAlignFactor = 1;
    uint32_t macroTileWidth = 32;
    uint32_t macroTileHeight = 16;
    uint32_t tileSlices = computeSurfaceTileSlices(baseTileMode, bpp, numSamples);
    uint32_t expTileMode = baseTileMode;

    if(numSamples > 1 || tileSlices > 1 || isDepth) {
        if (baseTileMode == 7) {
            expTileMode = 4;
        }
        else if (baseTileMode == 13) {
            expTileMode = 12;
        }
        else if (baseTileMode == 11) {
            expTileMode = 8;
        }
        else if (baseTileMode == 15) {
            expTileMode = 14;
        }
    }

    if(baseTileMode == 2 && numSamples > 1) {
        expTileMode = 4;
    }
    else if(baseTileMode == 3) {
        if(numSamples > 1 || isDepth) {
            expTileMode = 2;
        }
        if(numSamples == 2 || numSamples == 4) {
            expTileMode = 7;
        }
    }

    if(noRecursive || !level) {
        return expTileMode;
    }

    if(bpp == 24 || bpp == 48 || bpp == 96) {
        bpp = bpp / 3;
    }

    uint32_t widtha = nextPow2(width);
    uint32_t heighta = nextPow2(height);
    uint32_t numSlicesa = nextPow2(numSlices);

    expTileMode = convertToNonBankSwappedMode(static_cast<GX2TileMode>(expTileMode));
    uint32_t thickness = computeSurfaceThickness(static_cast<GX2TileMode>(expTileMode));
    uint32_t microTileBytes = (numSamples * bpp * (thickness << 6) + 7) >> 3;

    if (microTileBytes < 256) {
        widthAlignFactor = std::max<uint32_t>(1, 256 / microTileBytes);
    }
    if (expTileMode == 4 || expTileMode == 12) {
        if ((widtha < widthAlignFactor * macroTileWidth) || heighta < macroTileHeight) {
            expTileMode = 2;
        }
    }
    else if (expTileMode == 5) {
        macroTileWidth = 16;
        macroTileHeight = 32;

        if ((widtha < widthAlignFactor * macroTileWidth) || heighta < macroTileHeight) {
            expTileMode = 2;
        }
    }
    else if (expTileMode == 6) {
        macroTileWidth = 8;
        macroTileHeight = 64;

        if ((widtha < widthAlignFactor * macroTileWidth) || heighta < macroTileHeight) {
            expTileMode = 2;
        }
    }
    if (expTileMode == 7 || expTileMode == 13) {
        if ((widtha < widthAlignFactor * macroTileWidth) || heighta < macroTileHeight) {
            expTileMode = 3;
        }
    }
    if (numSlicesa < 4) {
        if (expTileMode == 3) {
            expTileMode = 2;
        }
        else if (expTileMode == 7) {
            expTileMode = 4;
        }
        else if (expTileMode == 13) {
            expTileMode = 12;
        }
    }

    return computeSurfaceMipLevelTileMode(
        static_cast<GX2TileMode>(expTileMode),
        bpp,
        level,
        widtha,
        heighta,
        numSlicesa,
        numSamples,
        isDepth,
        true);
}

void padDimensions(GX2TileMode tileMode, uint32_t padDims, bool isCube, uint32_t pitchAlign, uint32_t heightAlign, uint32_t sliceAlign, uint32_t& expPitch, uint32_t& expHeight, uint32_t& expNumSlices) {
    auto thickness = computeSurfaceThickness(tileMode);
    if(!padDims) {
        padDims = 3;
    }

    if(!(pitchAlign & (pitchAlign - 1))) {
        expPitch = powTwoAlign(expPitch, pitchAlign);
    }
    else {
        expPitch += pitchAlign - 1;
        expPitch = expPitch / pitchAlign;
        expPitch *= pitchAlign;
    }
    if (padDims > 1) {
        expHeight = powTwoAlign(expHeight, heightAlign);
    }
    if (padDims > 2 || thickness > 1) {
        if (isCube) {
            expNumSlices = nextPow2(expNumSlices);
        }
        if (thickness > 1) {
            expNumSlices = powTwoAlign(expNumSlices, sliceAlign);
        }
    }

    }

uint32_t adjustPitchAlignment(uint32_t flags, uint32_t pitchAlign) {
    if ((flags >> 13) & 1) {
        pitchAlign = powTwoAlign(pitchAlign, 0x20);
    }
    return pitchAlign;
}

void computeSurfaceAlignmentsLinear(GX2TileMode tileMode, uint32_t bpp, uint32_t flags, uint32_t& baseAlign, uint32_t& pitchAlign, uint32_t& heightAlign) {
    if (!tileMode) {
        baseAlign = 1;
        pitchAlign = 8;
        if (bpp != 1) {
            pitchAlign = 1;
        }
        heightAlign = 1;
    }
    else if (tileMode == 1) {
        uint32_t pixelsPerPipeInterleave = 2048 / bpp;
        baseAlign = 256;
        pitchAlign = std::max<uint32_t>(0x40, pixelsPerPipeInterleave);
        heightAlign = 1;
    }
    else {
        baseAlign = 1;
        pitchAlign = 1;
        heightAlign = 1;
    }
    pitchAlign = adjustPitchAlignment(flags, pitchAlign);
}

uint32_t computeSurfaceInfoLinear(surfaceOut& pOut, GX2TileMode tileMode, uint32_t bpp, uint32_t numSamples, uint32_t pitch, uint32_t height, uint32_t numSlices, uint32_t mipLevel, uint32_t padDims, uint32_t flags, uint32_t& expPitch, uint32_t& expHeight, uint32_t& expNumSlices) {
    expPitch = pitch;
    expHeight = height;
    expNumSlices = numSlices;

    uint32_t valid = 1;
    uint32_t microTileThickness = computeSurfaceThickness(tileMode);

    uint32_t baseAlign, pitchAlign, heightAlign;
    computeSurfaceAlignmentsLinear(tileMode, bpp, flags, baseAlign, pitchAlign, heightAlign);

    if (((flags >> 9) & 1) && !mipLevel) {
        expPitch = expPitch / 3;
        expPitch = nextPow2(expPitch);
    }
    if (mipLevel) {
        expPitch = nextPow2(expPitch);
        expHeight = nextPow2(expHeight);

        if ((flags >> 4) & 1) {
            expNumSlices = numSlices;

            if (numSlices <= 1) {
                padDims = 2;
            }
            else{
                padDims = 0;
            }
        }
        else {
            expNumSlices = nextPow2(numSlices);
        }
        padDimensions(
        tileMode,
        padDims,
        (flags >> 4) & 1,
        pitchAlign,
        heightAlign,
        microTileThickness, expPitch, expHeight, expNumSlices);
    }
    if (((flags >> 9) & 1) && !mipLevel) {
        expPitch *= 3;
    }

    uint32_t slices = expNumSlices * numSamples / microTileThickness;
    pOut.pitch = expPitch;
    pOut.height = expHeight;
    pOut.depth = expNumSlices;
    pOut.surfSize = (expHeight * expPitch * slices * bpp * numSamples + 7) / 8;
    pOut.baseAlign = baseAlign;
    pOut.pitchAlign = pitchAlign;
    pOut.heightAlign = heightAlign;
    pOut.depthAlign = microTileThickness;

    return valid;
}

void computeSurfaceAlignmentsMicroTiled(GX2TileMode tileMode, uint32_t bpp, uint32_t flags, uint32_t numSamples, uint32_t& baseAlign, uint32_t& pitchAlign, uint32_t& heightAlign) {
    if (bpp == 24 || bpp == 48 || bpp == 96) {
        bpp = bpp / 3;
    }
    uint32_t thickness = computeSurfaceThickness(tileMode);
    baseAlign = 256;
    pitchAlign = std::max<uint32_t>(8, 256 / bpp / numSamples / thickness);
    heightAlign = 8;

    pitchAlign = adjustPitchAlignment(flags, pitchAlign);
}

uint32_t computeSurfaceInfoMicroTiled(surfaceOut& pOut, GX2TileMode tileMode, uint32_t bpp, uint32_t numSamples, uint32_t pitch, uint32_t height, uint32_t numSlices, uint32_t mipLevel, uint32_t padDims, uint32_t flags, uint32_t& expPitch, uint32_t& expHeight, uint32_t& expNumSlices) {
    uint32_t expTileMode = tileMode;
    expPitch = pitch;
    expHeight = height;
    expNumSlices = numSlices;

    uint32_t valid = 1;
    auto microTileThickness = computeSurfaceThickness(tileMode);

    if (mipLevel) {
        expPitch = nextPow2(pitch);
        expHeight = nextPow2(height);
        if ((flags >> 4) & 1) {
            expNumSlices = numSlices;

            if (numSlices <= 1) {
                padDims = 2;
            }
            else {
                padDims = 0;
            }
        }
        else {
            expNumSlices = nextPow2(numSlices);
        }
        if (expTileMode == 3 && expNumSlices < 4) {
            expTileMode = 2;
            microTileThickness = 1;
        }
    }

    uint32_t baseAlign, pitchAlign, heightAlign;

    computeSurfaceAlignmentsMicroTiled(
        static_cast<GX2TileMode>(expTileMode),
        bpp,
        flags,
        numSamples, baseAlign, pitchAlign, heightAlign);

    padDimensions(
        static_cast<GX2TileMode>(expTileMode),
        padDims,
        (flags >> 4) & 1,
        pitchAlign,
        heightAlign,
        microTileThickness, expPitch, expHeight, expNumSlices);

    pOut.pitch = expPitch;
    pOut.height = expHeight;
    pOut.depth = expNumSlices;
    pOut.surfSize = (expHeight * expPitch * expNumSlices * bpp * numSamples + 7) / 8;
    pOut.tileMode = static_cast<GX2TileMode>(expTileMode);
    pOut.baseAlign = baseAlign;
    pOut.pitchAlign = pitchAlign;
    pOut.heightAlign = heightAlign;
    pOut.depthAlign = microTileThickness;

    return valid;
}

void computeSurfaceAlignmentsMacroTiled(GX2TileMode tileMode, uint32_t bpp, uint32_t flags, uint32_t numSamples, uint32_t& baseAlign, uint32_t& pitchAlign, uint32_t& heightAlign, uint32_t& macroTileWidth, uint32_t& macroTileHeight) {
    uint32_t aspectRatio = computeMacroTileAspectRatio(tileMode);
    uint32_t thickness = computeSurfaceThickness(tileMode);

    if (bpp == 24 || bpp == 48 || bpp == 96) {
        bpp = bpp / 3;
    }
    if (bpp == 3) {
        bpp = 1;
    }
    macroTileWidth = 32 / aspectRatio;
    macroTileHeight = aspectRatio * 16;

    pitchAlign = std::max<uint32_t>(macroTileWidth, macroTileWidth * (256 / bpp / (8 * thickness) / numSamples));
    pitchAlign = adjustPitchAlignment(flags, pitchAlign);

    heightAlign = macroTileHeight;
    uint32_t macroTileBytes = numSamples * ((bpp * macroTileHeight * macroTileWidth + 7) >> 3);

    if (thickness == 1) {
        baseAlign = std::max<uint32_t>(macroTileBytes, (numSamples * heightAlign * bpp * pitchAlign + 7) >> 3);
    }
    else {
        baseAlign = std::max<uint32_t>(256, (4 * heightAlign * bpp * pitchAlign + 7) >> 3);
    }
    uint32_t microTileBytes = (thickness * numSamples * (bpp << 6) + 7) >> 3;
    uint32_t numSlicesPerMicroTile;
    if (microTileBytes < 2048) {
        numSlicesPerMicroTile = 1;
    }
    else {
        numSlicesPerMicroTile = microTileBytes / 2048;
    }
    baseAlign = baseAlign / numSlicesPerMicroTile;
}

uint32_t computeSurfaceInfoMacroTiled(surfaceOut& pOut, GX2TileMode tileMode, GX2TileMode baseTileMode, uint32_t bpp, uint32_t numSamples, uint32_t pitch, uint32_t height, uint32_t numSlices, uint32_t mipLevel, uint32_t padDims, uint32_t flags, uint32_t& expPitch, uint32_t& expHeight, uint32_t& expNumSlices) {
    expPitch = pitch;
    expHeight = height;
    expNumSlices = numSlices;

    uint32_t valid = 1;
    uint32_t result;
    uint32_t expTileMode = tileMode;
    auto microTileThickness = computeSurfaceThickness(tileMode);

    if (mipLevel) {
        expPitch = nextPow2(pitch);
        expHeight = nextPow2(height);

        if ((flags >> 4) & 1) {
            expNumSlices = numSlices;
            padDims = 0;
            if (numSlices <= 1) padDims = 2;
        }
        else {
            expNumSlices = nextPow2(numSlices);
        }
        if (expTileMode == 7 && expNumSlices < 4) {
            expTileMode = 4;
            microTileThickness = 1;
        }
    }
    if (tileMode == baseTileMode || !mipLevel || !isThickMacroTiled(baseTileMode) || isThickMacroTiled(tileMode)) {
        uint32_t baseAlign, pitchAlign, heightAlign, macroWidth, macroHeight;

        computeSurfaceAlignmentsMacroTiled(
            tileMode,
            bpp,
            flags,
            numSamples, baseAlign, pitchAlign, heightAlign, macroWidth, macroHeight);

        auto bankSwappedWidth = computeSurfaceBankSwappedWidth(tileMode, bpp, numSamples, pitch);

        if (bankSwappedWidth > pitchAlign) pitchAlign = bankSwappedWidth;

        padDimensions(
            tileMode,
            padDims,
            (flags >> 4) & 1,
            pitchAlign,
            heightAlign,
            microTileThickness, expPitch, expHeight, expNumSlices);

        pOut.pitch = expPitch;
        pOut.height = expHeight;
        pOut.depth = expNumSlices;
        pOut.surfSize = (expHeight * expPitch * expNumSlices * bpp * numSamples + 7) / 8;
        pOut.tileMode = static_cast<GX2TileMode>(expTileMode);
        pOut.baseAlign = baseAlign;
        pOut.pitchAlign = pitchAlign;
        pOut.heightAlign = heightAlign;
        pOut.depthAlign = microTileThickness;
        result = valid;
    }
    else {
        uint32_t baseAlign, pitchAlign, heightAlign, macroWidth, macroHeight; 

        computeSurfaceAlignmentsMacroTiled(
            baseTileMode,
            bpp,
            flags,
            numSamples, baseAlign, pitchAlign, heightAlign, macroWidth, macroHeight);

        uint32_t pitchAlignFactor = std::max<uint32_t>(1, 32 / bpp);

        if (expPitch < pitchAlign * pitchAlignFactor || expHeight < heightAlign) {
            expTileMode = 2;
            uint32_t expPitch, expHeight, expNumSlices;

            result = computeSurfaceInfoMicroTiled(
                pOut,
                GX2_TILE_MODE_TILED_1D_THIN1,
                bpp,
                numSamples,
                pitch,
                height,
                numSlices,
                mipLevel,
                padDims,
                flags, expPitch, expHeight, expNumSlices);
        }
        else {
            computeSurfaceAlignmentsMacroTiled(
                tileMode,
                bpp,
                flags,
                numSamples, baseAlign, pitchAlign, heightAlign, macroWidth, macroHeight);

            uint32_t bankSwappedWidth = computeSurfaceBankSwappedWidth(tileMode, bpp, numSamples, pitch);
            if (bankSwappedWidth > pitchAlign) {
                pitchAlign = bankSwappedWidth;
            }

            padDimensions(
                tileMode,
                padDims,
                (flags >> 4) & 1,
                pitchAlign,
                heightAlign,
                microTileThickness, expPitch, expHeight, expNumSlices);

            pOut.pitch = expPitch;
            pOut.height = expHeight;
            pOut.depth = expNumSlices;
            pOut.surfSize = (expHeight * expPitch * expNumSlices * bpp * numSamples + 7) / 8;
            pOut.tileMode = static_cast<GX2TileMode>(expTileMode);
            pOut.baseAlign = baseAlign;
            pOut.pitchAlign = pitchAlign;
            pOut.heightAlign = heightAlign;
            pOut.depthAlign = microTileThickness;
            result = valid;
        }
    }

    return result;
}

uint32_t ComputeSurfaceInfoEx(const surfaceIn& in, surfaceOut& out) {
    auto tileMode = in.tileMode;
    auto bpp = in.bpp;
    auto numSamples = std::max<uint32_t>(1, in.numSamples);
    auto pitch = in.width;
    auto height = in.height;
    auto numSlices = in.numSlices;
    auto mipLevel = in.mipLevel;
    auto flags = in.flags;
    uint32_t padDims = 0;
    uint32_t valid = 0;
    auto baseTileMode = tileMode;

    if (((flags >> 4) & 1) && !mipLevel) {
        padDims = 2;
    }
    if ((flags >> 6) & 1) {
        tileMode = convertToNonBankSwappedMode(tileMode);
    }
    else {
        tileMode = static_cast<GX2TileMode>(computeSurfaceMipLevelTileMode(
            tileMode,
            bpp,
            mipLevel,
            pitch,
            height,
            numSlices,
            numSamples,
            (flags >> 1) & 1,
            false));
    }
    if (tileMode == 0 || tileMode == 1) {
        uint32_t expPitch, expHeight, expNumSlices;

        valid = computeSurfaceInfoLinear(
            out,
            tileMode,
            bpp,
            numSamples,
            pitch,
            height,
            numSlices,
            mipLevel,
            padDims,
            flags, expPitch, expHeight, expNumSlices);

        out.tileMode = tileMode;
    }
    else if (tileMode == 2 || tileMode == 3) {
        uint32_t expPitch, expHeight, expNumSlices;

        valid = computeSurfaceInfoMicroTiled(
            out,
            tileMode,
            bpp,
            numSamples,
            pitch,
            height,
            numSlices,
            mipLevel,
            padDims,
            flags, expPitch, expHeight, expNumSlices);
    }
    else if (4 <= tileMode && tileMode <= 15) {
        uint32_t expPitch, expHeight, expNumSlices;

        valid = computeSurfaceInfoMacroTiled(
            out,
            tileMode,
            baseTileMode,
            bpp,
            numSamples,
            pitch,
            height,
            numSlices,
            mipLevel,
            padDims,
            flags, expPitch, expHeight, expNumSlices);
    }
    if (!valid) {
        return 3;
    }
    return 0;
}

uint32_t restoreSurfaceInfo(surfaceOut& pOut, uint32_t elemMode, uint32_t expandX, uint32_t expandY, uint32_t bpp) {
    if (pOut.pixelPitch && pOut.pixelHeight) {
        uint32_t width = pOut.pixelPitch;
        uint32_t height = pOut.pixelHeight;

        if (expandX > 1 || expandY > 1) {
            if (elemMode == 4) {
                width = width / expandX;
                height = height / expandY;
            }
            else {
                width *= expandX;
                height *= expandY;
            }
        }
        pOut.pixelPitch = std::max<uint32_t>(1, width);
        pOut.pixelHeight = std::max<uint32_t>(1, height);
    }
    if (bpp) {
        if (elemMode == 4) {
            return expandY * expandX * bpp;
        }
        else if (elemMode == 5 || elemMode == 6) {
            return bpp / expandX / expandY;
        }
        else if (elemMode == 9 || elemMode == 12) {
            return 64;
        }
        else if (elemMode == 10 || elemMode == 11 || elemMode == 13) {
            return 128;
        }
        return bpp;
    }
    return 0;
}

void computeSurfaceInfo(surfaceIn& aSurfIn, surfaceOut& pSurfOut) {
    uint32_t returnCode = 0;

    if(aSurfIn.bpp > 0x80) {
        returnCode = 3;
    }

    if(returnCode == 0) {
        uint8_t elemMode = 0;
        computeMipLevel(aSurfIn);

        uint32_t width = aSurfIn.width;
        uint32_t height = aSurfIn.height;
        uint8_t bpp = aSurfIn.bpp;
        uint8_t expandX = 1;
        uint8_t expandY = 1;

        pSurfOut.pixelBits = aSurfIn.bpp;

        if(aSurfIn.format) {
            getBitsPerPixel(aSurfIn.format, bpp, expandX, expandY, elemMode);

            if(elemMode == 4 && expandX == 3 && aSurfIn.tileMode == 1) {
                aSurfIn.flags |= 0x200;
            }

            bpp = adjustSurfaceInfo(aSurfIn, elemMode, expandX, expandY, bpp, width, height);
        }
        else if (aSurfIn.bpp) {
            aSurfIn.width = std::max<uint32_t>(1, aSurfIn.width);
            aSurfIn.height = std::max<uint32_t>(1, aSurfIn.height);
        }
        else {
            returnCode = 3;
        }

        if(returnCode == 0) {
            returnCode = ComputeSurfaceInfoEx(aSurfIn, pSurfOut);
        }

        if (returnCode == 0) {
            pSurfOut.bpp = aSurfIn.bpp;
            pSurfOut.pixelPitch = pSurfOut.pitch;
            pSurfOut.pixelHeight = pSurfOut.height;

            if (aSurfIn.format && (!((aSurfIn.flags >> 9) & 1) || !aSurfIn.mipLevel)) bpp = restoreSurfaceInfo(pSurfOut, elemMode, expandX, expandY, bpp);

            if ((aSurfIn.flags >> 5) & 1) pSurfOut.sliceSize = pSurfOut.surfSize;

            else {
                pSurfOut.sliceSize = pSurfOut.surfSize / pSurfOut.depth;

                if (aSurfIn.slice == (aSurfIn.numSlices - 1) && aSurfIn.numSlices > 1) {
                    pSurfOut.sliceSize += pSurfOut.sliceSize * (pSurfOut.depth - aSurfIn.numSlices);
                }
            }
            pSurfOut.pitchTileMax = (pSurfOut.pitch >> 3) - 1;
            pSurfOut.heightTileMax = (pSurfOut.height >> 3) - 1;
            pSurfOut.sliceTileMax = (pSurfOut.height * pSurfOut.pitch >> 6) - 1;
        }
    }
}

surfaceOut getSurfaceInfo(GX2SurfaceFormat surfaceFormat, uint32_t surfaceWidth, uint32_t surfaceHeight, uint32_t surfaceDepth, GX2SurfaceDim surfaceDim, GX2TileMode surfaceTileMode, GX2AAMode surfaceAA, uint32_t level) {
    auto dim = GX2_SURFACE_DIM_TEXTURE_1D;
    uint32_t hwFormat = 0;

    surfaceOut pSurfOut;

    hwFormat = surfaceFormat & 0x3F;
    if(surfaceTileMode == GX2_TILE_MODE_LINEAR_SPECIAL) {
        uint32_t width = 0;
        uint32_t numSamples = 0;
        uint32_t blockSize = 0;
        numSamples = 1 << surfaceAA;
    
        if(hwFormat < 0x31 || hwFormat > 0x35) {
            blockSize = 1;
        }
        else {
            blockSize = 4;
        }

        width = ~(blockSize - 1) & (std::max<uint32_t>(1, surfaceWidth >> level) + blockSize - 1);

        pSurfOut.bpp = formatHwInfo[hwFormat * 4];
        pSurfOut.size = 96;
        pSurfOut.pitch = width / blockSize;
        pSurfOut.pixelBits = formatHwInfo[hwFormat * 4];
        pSurfOut.baseAlign = 1;
        pSurfOut.pitchAlign = 1;
        pSurfOut.heightAlign = 1;
        pSurfOut.depthAlign = 1;
        dim = surfaceDim;

        if (dim == 0) {
            pSurfOut.height = 1;
            pSurfOut.depth = 1;
        }
        else if (dim == 1 || dim == 6) {
            pSurfOut.height = std::max<uint32_t>(1, surfaceHeight >> level);
            pSurfOut.depth = 1;
        }
        else if (dim == 2) {
            pSurfOut.height = std::max<uint32_t>(1, surfaceHeight >> level);
            pSurfOut.depth = std::max<uint32_t>(1, surfaceDepth >> level);
        }
        else if (dim == 3) {
            pSurfOut.height = std::max<uint32_t>(1, surfaceHeight >> level);
            pSurfOut.depth = std::max<uint32_t>(6, surfaceDepth);
        }
        else if (dim == 4) {
            pSurfOut.height = 1;
            pSurfOut.depth = surfaceDepth;
        }
        else if (dim == 5 || dim == 7) {
            pSurfOut.height = std::max<uint32_t>(1, surfaceHeight >> level);
            pSurfOut.depth = surfaceDepth;
        }

        pSurfOut.pixelPitch = width;
        pSurfOut.pixelHeight = ~(blockSize - 1) & (pSurfOut.height + blockSize - 1);
        pSurfOut.height = pSurfOut.pixelHeight / blockSize;
        pSurfOut.surfSize = pSurfOut.bpp * numSamples * pSurfOut.depth * pSurfOut.height * pSurfOut.pitch >> 3;

        if(surfaceDim == 2) {
            pSurfOut.sliceSize = pSurfOut.surfSize;
        }
        else {
            pSurfOut.sliceSize = pSurfOut.surfSize / pSurfOut.depth;
        }

        pSurfOut.pitchTileMax = (pSurfOut.pitch >> 3) - 1;
        pSurfOut.heightTileMax = (pSurfOut.height >> 3) - 1;
        pSurfOut.sliceTileMax = (pSurfOut.height * pSurfOut.pitch >> 6) - 1;
    }
    else {
        surfaceIn aSurfIn;
        aSurfIn.size = 60;
        aSurfIn.tileMode = static_cast<GX2TileMode>(surfaceTileMode & 0xF);
        aSurfIn.format = hwFormat;
        aSurfIn.bpp = formatHwInfo[hwFormat * 4];
        aSurfIn.numSamples = 1 << surfaceAA;
        aSurfIn.numFrags = aSurfIn.numSamples;
        aSurfIn.width = std::max<uint32_t>(1, surfaceWidth >> level);
        dim = surfaceDim;

        if(dim == 0) {
            aSurfIn.height = 1;
            aSurfIn.numSlices = 1;
        }
        else if(dim == 1 || dim == 6) {
            aSurfIn.height = std::max<uint32_t>(1, surfaceHeight >> level);
            aSurfIn.numSlices = 1;
        }
        else if(dim == 2) {
            aSurfIn.height = std::max<uint32_t>(1, surfaceHeight >> level);
            aSurfIn.numSlices = std::max<uint32_t>(1, surfaceDepth >> level);
        }
        else if(dim == 3) {
            aSurfIn.height = std::max<uint32_t>(1, surfaceHeight >> level);
            aSurfIn.numSlices = std::max<uint32_t>(6, surfaceDepth);
            aSurfIn.flags |= 0x10;
        }
        else if(dim == 4) {
            aSurfIn.height = 1;
            aSurfIn.numSlices = surfaceDepth;
        }
        else if(dim == 5 || dim == 7) {
            aSurfIn.height = std::max<uint32_t>(1, surfaceHeight >> level);
            aSurfIn.numSlices = surfaceDepth;
        }

        aSurfIn.slice = 0;
        aSurfIn.mipLevel = level;

        if (surfaceDim == 2) {
            aSurfIn.flags |= 0x20;
        }
        if(level == 0) {
            aSurfIn.flags = (1 << 12) | (aSurfIn.flags & 0xFFFFEFFF);
        }
        else {
            aSurfIn.flags = aSurfIn.flags & 0xFFFFEFFF;
        }

        pSurfOut.size = 96;
        computeSurfaceInfo(aSurfIn, pSurfOut);
    }

    if(!pSurfOut.tileMode) pSurfOut.tileMode = GX2TileMode::GX2_TILE_MODE_LINEAR_SPECIAL;

    return pSurfOut;
}

GX2TileMode getDefaultGX2TileMode(GX2SurfaceDim dim, uint32_t width, uint32_t height, uint32_t depth, GX2SurfaceFormat format_, GX2AAMode aa, GX2SurfaceUse use) {

    GX2TileMode tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;
    bool isDepthBuffer(use & 4);
    bool isColorBuffer(use & 2);

    if (dim || aa || isDepthBuffer) {
        if (dim != 2 || isColorBuffer) {
            tileMode = GX2_TILE_MODE_TILED_2D_THIN1;
        }
        else {
            tileMode = GX2_TILE_MODE_TILED_2D_THICK;
        }

        auto surfOut = getSurfaceInfo(format_, width, height, depth, dim, tileMode, aa, 0);
        if (width < surfOut.pitchAlign && height < surfOut.heightAlign) {
            if (tileMode == 7) {
                tileMode = GX2_TILE_MODE_TILED_1D_THICK;
            }
            else {
                tileMode = GX2_TILE_MODE_TILED_1D_THIN1;
            }
        }
    }

    return tileMode;
}
