#include "stringUtil.hpp"

#include <codecvt>



namespace Utility::Str {
    bool endsWith(const std::string& a, const std::string& b) {
        if (b.size() > a.size()) return false;
        return std::equal(a.end() - b.size(), a.end(), b.begin());
    }

    std::string toUTF8(const std::u16string& str) {
    	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
    	return conv.to_bytes(str);
    }

    std::u16string toUTF16(const std::string& str)
    {
    	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
    	return conv.from_bytes(str);
    }
}
