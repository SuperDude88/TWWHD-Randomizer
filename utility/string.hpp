#pragma once

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

    template<typename T>
    T merge(const std::vector<T>& lines, const typename T::value_type separator) {
    	T ret;
    	for (const T& segment : lines) {
    		ret += segment + separator;
    	}

    	return ret;
    }

	  template<typename T>
    T assureNullTermination(const T& string) {
    	if(!string.empty() && string.back() == typename T::value_type(0)) return string;

      return string + typename T::value_type(0);
    }

    std::string InsertUnicodeReplacements(std::string text);
    std::string RemoveUnicodeReplacements(std::string text);
    std::u16string RemoveUnicodeReplacements(std::u16string text);

    template<typename T>
    std::string intToHex(const T& i, const bool& base = true)
    {
      static_assert(std::is_integral_v<T>, "intToHex<T> must be integral type!");

      std::stringstream stream;
      stream << std::hex << (base ? std::showbase : std::noshowbase) << i;
      return stream.str();
    }

    template<typename T>
    std::string intToHex(const T& i, const std::streamsize& width, const bool& base = true)
    {
      static_assert(std::is_integral_v<T>, "intToHex<T> must be integral type!");

      std::stringstream stream;
      stream << std::hex << (base ? std::showbase : std::noshowbase) << std::setfill('0') << std::setw(width) << i;
      return stream.str();
    }

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
