#pragma once

#include <string>
#include <vector>



namespace Utility::Str {
    bool endsWith(const std::string& a, const std::string& b);

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
    	if(string.back() == typename T::value_type(0)) return string;

		return string + typename T::value_type(0);
    }
}
