#include "byteswap.hpp"

namespace Utility
{
    //Check if the platform is Wii U, if it is, don't byteswap (already big endian)
    //Allow that to be overridden with the forced bool, since some files on console (shaders) are little endian for some reason

    uint64_t byteswap(const uint64_t& value, const bool forced)
    {
#ifdef DEVKITPRO
        if (!forced) return value;
#endif

        return ((value & 0xFF00000000000000) >> 56) | 
            ((value & 0x00FF000000000000) >> 40) | 
            ((value & 0x0000FF0000000000) >> 24) | 
            ((value & 0x000000FF00000000) >> 8) | 
            ((value & 0x00000000FF000000) << 8) | 
            ((value & 0x0000000000FF0000) << 24) | 
            ((value & 0x000000000000FF00) << 40) | 
            ((value & 0x00000000000000FF) << 56);
    }

    uint32_t byteswap(const uint32_t& value, const bool forced)
    {
#ifdef DEVKITPRO
        if (!forced) return value;
#endif

        return ((value & 0xFF000000) >> 24) |
            ((value & 0x00FF0000) >> 8) | 
            ((value & 0x0000FF00) << 8) | 
            ((value & 0x000000FF) << 24);
    }

    uint16_t byteswap(const uint16_t& value, const bool forced)
    {
#ifdef DEVKITPRO
        if (!forced) return value;
#endif

        return ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);
    }

    int64_t byteswap(const int64_t& value, const bool forced)
    {
#ifdef DEVKITPRO
        if (!forced) return value;
#endif

        uint64_t result = byteswap(reinterpret_cast<const uint64_t&>(value));
        return *(reinterpret_cast<int64_t*>(&result));
    }

    int32_t byteswap(const int32_t& value, const bool forced)
    {
#ifdef DEVKITPRO
        if (!forced) return value;
#endif

        uint32_t result = byteswap(reinterpret_cast<const uint32_t&>(value));
        return *(reinterpret_cast<int32_t*>(&result));
    }

    int16_t byteswap(const int16_t& value, const bool forced)
    {
#ifdef DEVKITPRO
        if (!forced) return value;
#endif


        uint16_t result = byteswap(reinterpret_cast<const uint16_t&>(value));
        return *(reinterpret_cast<int16_t*>(&result));
    }

    float byteswap(const float& value, const bool forced)
    {
#ifdef DEVKITPRO
        if (!forced) return value;
#endif

        uint32_t result = byteswap(reinterpret_cast<const uint32_t&>(value));
        return *(reinterpret_cast<float*>(&result));
    }

    double byteswap(const double& value, const bool forced)
    {
#ifdef DEVKITPRO
        if (!forced) return value;
#endif

        uint64_t result = byteswap(reinterpret_cast<const uint64_t&>(value));
        return *(reinterpret_cast<double*>(&result));
    }
}
