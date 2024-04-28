#pragma once

#include <vector>
#include <variant>
#include <concepts>

#include <utility/endian.hpp>

template<typename T> requires std::signed_integral<T>
class RelOffset {
public:
    T offset;

    explicit operator T() { return offset; }

    bool isRelNonzero() const {
        return relOffset != 0;
    }

    void read(std::istream& in) {
        location = static_cast<T>(in.tellg());
        in.read(reinterpret_cast<char*>(&relOffset), sizeof(relOffset));
        Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, relOffset);
        offset = location + relOffset;
    }

    void write(std::ostream& out) {
        location = out.tellp();
        relOffset = offset - location;
        Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, relOffset);
        out.write(reinterpret_cast<const char*>(&relOffset), sizeof(relOffset));
    }
    
    void write(std::ostream& out, const T& absOffset) {
        offset = absOffset;
        location = out.tellp();
        relOffset = offset - location;
        Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, relOffset);
        out.write(reinterpret_cast<const char*>(&relOffset), sizeof(relOffset));
    }

private:
    T relOffset;
    T location;
};

class UserData {
public:
    std::string name;
    std::variant<
        std::vector<int32_t>,
        std::vector<float>,
        std::vector<std::string>,
        std::vector<std::u16string>,
        std::vector<uint8_t>
    > data; //index matches data type enum
};
