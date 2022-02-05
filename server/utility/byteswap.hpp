#pragma once

#include <cstdint>

namespace Utility
{
    uint64_t byteswap(const uint64_t& value, const bool forced = false);

    uint32_t byteswap(const uint32_t& value, const bool forced = false);

    uint16_t byteswap(const uint16_t& value, const bool forced = false);

    int64_t byteswap(const int64_t& value, const bool forced = false);

    int32_t byteswap(const int32_t& value, const bool forced = false);

    int16_t byteswap(const int16_t& value, const bool forced = false);

    float byteswap(const float& value, const bool forced = false);

    double byteswap(const double& value, const bool forced = false);

    template<typename T>
    void byteswap_inplace(T& value)
    {
        value = byteswap(value);
    }

    template<typename T>
    inline T byte_align_offset(T toAlign, T alignment = sizeof(T))
    {
        return (alignment - toAlign % alignment) % alignment;
    }

    template<typename T>
    inline T byte_align(T toAlign, T alignment = sizeof(T))
    {
        return toAlign + byte_align_offset(toAlign, alignment);
    }
}
