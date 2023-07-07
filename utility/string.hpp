#pragma once

#include <concepts>
#include <ios>
#include <string>
#include <type_traits>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace Utility::Str {
    std::string toUTF8(const std::u16string& str);

    std::u16string toUTF16(const std::string& str);

    template<typename T> 
    concept StringType = std::derived_from<T, std::basic_string<typename T::value_type>>;

    template<typename T> requires StringType<T>
    std::vector<T> split(const T& string, const typename T::value_type delim) {
        std::vector<T> ret;
        T tail = string;
        auto index = tail.find_first_of(delim);

        while (index != T::npos) {
            ret.push_back(tail.substr(0, index));
            tail = tail.substr(index + 1);
            index = tail.find_first_of(delim);
        }
        ret.push_back(tail); //add anything after last line break

        return ret;
    }

    template<typename T> requires StringType<T>
    T merge(const std::vector<T>& lines, const typename T::value_type separator) {
        T ret;
        for (const T& segment : lines) {
            ret += segment + separator;
        }

        return ret;
    }

      template<typename T> requires StringType<T>
    T assureNullTermination(const T& string) {
        if(!string.empty() && string.back() == typename T::value_type(0)) return string;

        return string + typename T::value_type(0);
    }

    template<typename T> requires std::integral<T>
    std::string intToHex(const T& i, const bool& base = true)
    {
        std::stringstream stream;
        stream << std::hex << (base ? std::showbase : std::noshowbase) << i;
        return stream.str();
    }

    template<typename T> requires std::integral<T>
    std::string intToHex(const T& i, const std::streamsize& width, const bool& base = true)
    {
        std::stringstream stream;
        stream << std::hex << (base ? std::showbase : std::noshowbase) << std::setfill('0') << std::setw(width) << i;
        return stream.str();
    }

    bool contains(const std::string& str, const std::string& substr);

    //wrapper for a constexpr string, for use in other templates
    template<size_t N>
    struct StringLiteral {
        constexpr StringLiteral(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }

        constexpr operator std::string_view() const {
            return std::string_view(value, N - 1); //leave out null terminator
        }

        char value[N];
    };
}
