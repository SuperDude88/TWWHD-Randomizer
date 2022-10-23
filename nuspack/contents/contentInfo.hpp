#pragma once

#include <fstream>
#include <cstdint>

#include <nuspack/types.hpp>

class ContentInfo {
public:
    uint16_t indexOffset = 0;
    uint16_t contentCount = 0x0B;
    SHA256_t hash{0};

    ContentInfo() :
        ContentInfo(0)
    {}
    ContentInfo(const uint16_t& count_) :
        ContentInfo(0, count_)
    {}
    ContentInfo(const uint16_t& idxOffset_, const uint16_t& count_) :
        indexOffset(idxOffset_),
        contentCount(count_)
    {}

    void writeToStream(std::ostream& out);
};
