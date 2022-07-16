#pragma once

#include <array>
#include <cstdint>
#include <stdint.h>
#include <unordered_set>
#include <string>
#include <cmath>
#include "../shared/gx2.hpp"



struct tileInfo {
    uint32_t banks = 0;
    uint32_t bankWidth = 0;
    uint32_t bankHeight = 0;
    uint32_t macroAspectRatio = 0;
    uint32_t tileSplitBytes = 0;
    uint32_t pipeConfig = 0;
};

struct surfaceIn {
    uint32_t size = 0;
    GX2TileMode tileMode = GX2TileMode::GX2_TILE_MODE_DEFAULT;
    uint32_t format = 0;
    uint32_t bpp = 0;
    uint32_t numSamples = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t numSlices = 0;
    uint32_t slice = 0;
    uint32_t mipLevel = 0;
    uint32_t flags = 0;
    uint32_t numFrags = 0;
    tileInfo pTileInfo;
    uint32_t tileIndex = 0;
};

struct surfaceOut {
    uint32_t size = 0;
    uint32_t pitch = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
    int64_t surfSize = 0;
    GX2TileMode tileMode = GX2TileMode::GX2_TILE_MODE_DEFAULT;
    uint32_t baseAlign = 0;
    uint32_t pitchAlign = 0;
    uint32_t heightAlign = 0;
    uint32_t depthAlign = 0;
    uint32_t bpp = 0;
    uint32_t pixelPitch = 0;
    uint32_t pixelHeight = 0;
    uint32_t pixelBits = 0;
    uint32_t sliceSize = 0;
    uint32_t pitchTileMax = 0;
    uint32_t heightTileMax = 0;
    uint32_t sliceTileMax = 0;
    tileInfo pTileInfo;
    uint32_t tileType = 0;
    uint32_t tileIndex = 0;
};

GX2TileMode GX2TileModeToAddrTileMode(GX2TileMode tileMode);

uint8_t surfaceGetBitsPerPixel(uint32_t surfaceFormat);

uint8_t computeSurfaceThickness(GX2TileMode tileMode);

uint32_t computePixelIndexWithinMicroTile(uint32_t x, uint32_t y, uint32_t z, uint32_t bpp, GX2TileMode tileMode, bool isDepth);

uint32_t computePipeFromCoordWoRotation(uint32_t x, uint32_t y);

uint32_t computeBankFromCoordWoRotation(uint32_t x, uint32_t y);

uint8_t computeSurfaceRotationFromTileMode(GX2TileMode tileMode);

bool isThickMacroTiled(GX2TileMode tileMode);

bool isBankSwappedTileMode(GX2TileMode tileMode);

uint8_t computeMacroTileAspectRatio(GX2TileMode tileMode);

uint32_t computeSurfaceBankSwappedWidth(GX2TileMode tileMode, uint32_t bpp, uint32_t numSamples, uint32_t pitch);

uint64_t computeSurfaceAddrFromCoordLinear(uint32_t x, uint32_t y, uint32_t slice, uint32_t sample, uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSlices);

uint64_t computeSurfaceAddrFromCoordMicroTiled(uint32_t x, uint32_t y, uint32_t slice, uint32_t bpp, uint32_t pitch, uint32_t height, GX2TileMode tileMode, bool isDepth);

uint64_t computeSurfaceAddrFromCoordMacroTiled(uint32_t x, uint32_t y, uint32_t slice, uint32_t sample, uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSamples, GX2TileMode tileMode, bool isDepth, uint32_t pipeSwizzle, uint32_t bankSwizzle);

std::string swizzleSurf(uint32_t width, uint32_t height, uint32_t depth, GX2SurfaceFormat format_, GX2AAMode aa, GX2SurfaceUse use, GX2TileMode tileMode, uint32_t swizzle_, uint32_t pitch, uint32_t bitsPerPixel, uint32_t slice, uint32_t sample, std::string& data, bool swizzle);

uint32_t powTwoAlign(uint32_t x, uint32_t align);

uint32_t nextPow2(uint32_t dim);

void getBitsPerPixel(uint32_t format_, uint8_t& bpp, uint8_t& expandX, uint8_t& expandY, uint8_t& elemMode);

uint32_t adjustSurfaceInfo(surfaceIn& in, uint8_t elemMode, uint8_t expandX, uint8_t expandY, uint8_t bpp, uint32_t width, uint32_t height);

uint32_t hwlComputeMipLevel(surfaceIn& in);

void computeMipLevel(surfaceIn& in);

GX2TileMode convertToNonBankSwappedMode(GX2TileMode tileMode);

uint32_t computeSurfaceTileSlices(GX2TileMode tileMode, uint32_t bpp, uint32_t numSamples);

uint32_t computeSurfaceMipLevelTileMode(GX2TileMode baseTileMode, uint32_t bpp, uint32_t level, uint32_t width, uint32_t height, uint32_t numSlices, uint32_t numSamples, bool isDepth, bool noRecursive);

void padDimensions(GX2TileMode tileMode, uint32_t padDims, bool isCube, uint32_t pitchAlign, uint32_t heightAlign, uint32_t sliceAlign, uint32_t& expPitch, uint32_t& expHeight, uint32_t& expNumSlices);

uint32_t adjustPitchAlignment(uint32_t flags, uint32_t pitchAlign);

void computeSurfaceAlignmentsLinear(GX2TileMode tileMode, uint32_t bpp, uint32_t flags, uint32_t& baseAlign, uint32_t& pitchAlign, uint32_t& heightAlign);

uint32_t computeSurfaceInfoLinear(surfaceOut& pOut, GX2TileMode tileMode, uint32_t bpp, uint32_t numSamples, uint32_t pitch, uint32_t height, uint32_t numSlices, uint32_t mipLevel, uint32_t padDims, uint32_t flags, uint32_t& expPitch, uint32_t& expHeight, uint32_t& expNumSlices);

void computeSurfaceAlignmentsMicroTiled(GX2TileMode tileMode, uint32_t bpp, uint32_t flags, uint32_t numSamples, uint32_t& baseAlign, uint32_t& pitchAlign, uint32_t& heightAlign);

uint32_t computeSurfaceInfoMicroTiled(surfaceOut& pOut, GX2TileMode tileMode, uint32_t bpp, uint32_t numSamples, uint32_t pitch, uint32_t height, uint32_t numSlices, uint32_t mipLevel, uint32_t padDims, uint32_t flags, uint32_t& expPitch, uint32_t& expHeight, uint32_t& expNumSlices);

void computeSurfaceAlignmentsMacroTiled(GX2TileMode tileMode, uint32_t bpp, uint32_t flags, uint32_t numSamples, uint32_t& baseAlign, uint32_t& pitchAlign, uint32_t& heightAlign, uint32_t& macroTileWidth, uint32_t& macroTileHeight);

uint32_t computeSurfaceInfoMacroTiled(surfaceOut& pOut, GX2TileMode tileMode, GX2TileMode baseTileMode, uint32_t bpp, uint32_t numSamples, uint32_t pitch, uint32_t height, uint32_t numSlices, uint32_t mipLevel, uint32_t padDims, uint32_t flags, uint32_t& expPitch, uint32_t& expHeight, uint32_t& expNumSlices);

uint32_t ComputeSurfaceInfoEx(surfaceIn& in, surfaceOut& out);

uint32_t restoreSurfaceInfo(surfaceOut& pOut, uint32_t elemMode, uint32_t expandX, uint32_t expandY, uint32_t bpp);

void computeSurfaceInfo(surfaceIn& aSurfIn, surfaceOut& pSurfOut);

surfaceOut getSurfaceInfo(GX2SurfaceFormat surfaceFormat, uint32_t surfaceWidth, uint32_t surfaceHeight, uint32_t surfaceDepth, GX2SurfaceDim surfaceDim, GX2TileMode surfaceTileMode, GX2AAMode surfaceAA, uint32_t level);

GX2TileMode getDefaultGX2TileMode(GX2SurfaceDim dim, uint32_t width, uint32_t height, uint32_t depth, GX2SurfaceFormat format_, GX2AAMode aa, GX2SurfaceUse use);
