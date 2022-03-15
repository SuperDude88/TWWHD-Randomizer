#pragma once

#include <cstdint>



namespace Utility::Endian
{
    enum struct Type {
        Big = 0,
        Little = 1
    };

#ifdef DEVKITPRO
    constexpr Type target = Type::Big;
#elif defined(_WIN32)
    constexpr Type target = Type::Little;
#endif

    uint64_t byteswap(const uint64_t& value);

    uint32_t byteswap(const uint32_t& value);

    uint16_t byteswap(const uint16_t& value);

    int64_t byteswap(const int64_t& value);

    int32_t byteswap(const int32_t& value);

    int16_t byteswap(const int16_t& value);

    float byteswap(const float& value);

    double byteswap(const double& value);

    template<typename T>
    T toPlatform(const Type& src, const T& value) {
        static_assert(sizeof(T) > 1, "Cannot byteswap a single-byte value");
        if (src != target) return byteswap(value);
        return value;
    }

    template<typename T>
    void toPlatform_inplace(const Type& src, T& value) {
        static_assert(sizeof(T) > 1, "Cannot byteswap a single-byte value");
        if (src != target) value = byteswap(value);
    }
}
