#pragma once

#include <cstdint>
#include <string>
#include <type_traits>



namespace Utility::Endian
{
    enum struct Type {
        Big = 0,
        Little = 1
    };

#ifdef DEVKITPRO
    constexpr Type target = Type::Big;
#elif defined(_WIN32) || defined(__linux__)
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

    char16_t byteswap(const char16_t& value);

    std::u16string byteswap(const std::u16string& value);

    template<typename T, class = typename std::enable_if<std::is_enum<T>::value == false>::type>
    T toPlatform(const Type& src, const T& value) {
        static_assert(sizeof(T) > 1, "Cannot byteswap a single-byte value");
        if (src != target) return byteswap(value);
        return value;
    }

    //for enums
    template<typename T, class = typename std::enable_if<std::is_enum<T>::value>::type, typename TBase = std::underlying_type_t<T>>
    T toPlatform(const Type& src, const T& value) {
        static_assert(sizeof(T) > 1, "Cannot byteswap a single-byte value");
        if (src != target) return static_cast<T>(byteswap(static_cast<TBase>(value)));
        return value;
    }

    //doesn't work for enums
    template<typename T, class = typename std::enable_if<std::is_enum<T>::value == false>::type>
    void toPlatform_inplace(const Type& src, T& value) {
        static_assert(sizeof(T) > 1, "Cannot byteswap a single-byte value");
        if (src != target) value = byteswap(value);
    }

    //for enums
    template<typename T, class = typename std::enable_if<std::is_enum<T>::value>::type, typename TBase = std::underlying_type_t<T>>
    void toPlatform_inplace(const Type& src, T& value) {
        static_assert(sizeof(T) > 1, "Cannot byteswap a single-byte value");
        if (src != target) value = static_cast<T>(byteswap(static_cast<TBase>(value)));
    }
}
