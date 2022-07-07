#include "endian.hpp"

namespace Utility::Endian
{
        uint64_t byteswap(const uint64_t& value)
        {
            return ((value & 0xFF00000000000000) >> 56) |
                ((value & 0x00FF000000000000) >> 40) |
                ((value & 0x0000FF0000000000) >> 24) |
                ((value & 0x000000FF00000000) >> 8) |
                ((value & 0x00000000FF000000) << 8) |
                ((value & 0x0000000000FF0000) << 24) |
                ((value & 0x000000000000FF00) << 40) |
                ((value & 0x00000000000000FF) << 56);
        }

        uint32_t byteswap(const uint32_t& value)
        {
            return ((value & 0xFF000000) >> 24) |
                ((value & 0x00FF0000) >> 8) |
                ((value & 0x0000FF00) << 8) |
                ((value & 0x000000FF) << 24);
        }

        uint16_t byteswap(const uint16_t& value)
        {
            return ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);
        }

        int64_t byteswap(const int64_t& value)
        {
            uint64_t result = byteswap(reinterpret_cast<const uint64_t&>(value));
            return *(reinterpret_cast<int64_t*>(&result));
        }

        int32_t byteswap(const int32_t& value)
        {
            uint32_t result = byteswap(reinterpret_cast<const uint32_t&>(value));
            return *(reinterpret_cast<int32_t*>(&result));
        }

        int16_t byteswap(const int16_t& value)
        {
            uint16_t result = byteswap(reinterpret_cast<const uint16_t&>(value));
            return *(reinterpret_cast<int16_t*>(&result));
        }

        float byteswap(const float& value)
        {
            uint32_t result = byteswap(reinterpret_cast<const uint32_t&>(value));
            return *(reinterpret_cast<float*>(&result));
        }

        double byteswap(const double& value)
        {
            uint64_t result = byteswap(reinterpret_cast<const uint64_t&>(value));
            return *(reinterpret_cast<double*>(&result));
        }
        
        char16_t byteswap(const char16_t& value)
        {
            uint16_t result = byteswap(reinterpret_cast<const uint16_t&>(value));
            return *(reinterpret_cast<char16_t*>(&result));
        }

        std::u16string byteswap(const std::u16string& value) {
            std::u16string result = value;
            for(char16_t& character : result) {
                character = byteswap(character);
            }

            return result;
        }
}
