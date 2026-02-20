#include "string.hpp"

#include <locale>
#include <string>

namespace Utility::Str {
    // Can't use codecvt on Wii U, deprecated in c++17 and g++ hates it
    // Borrowed from https://docs.microsoft.com/en-us/cpp/standard-library/codecvt-class?view=msvc-170#out
    std::string toUTF8(const std::u16string& str) {
        if(str.empty()) return "";

        std::string ret(str.size(), '\0');
        char* pszNext;
        const char16_t* pwszNext;
        std::mbstate_t state = {0}; // zero-initialization represents the initial conversion state for mbstate_t
        const std::codecvt_base::result& res = std::use_facet<std::codecvt<char16_t, char, mbstate_t>>(std::locale::classic()).out(
            state, str.data(), str.data() + str.size(), pwszNext, ret.data(), ret.data() + ret.size(), pszNext);

        if(res == std::codecvt_base::error) return "";
        return ret;
    }

    std::u16string toUTF16(const std::string& str)
    {
        if(str.empty()) return u"";

        std::u16string ret(str.size(), u'\0');
        const char* pszNext;
        char16_t* pwszNext;
        std::mbstate_t state = {0}; // zero-initialization represents the initial conversion state for mbstate_t
        const std::codecvt_base::result& res = std::use_facet<std::codecvt<char16_t, char, mbstate_t>>(std::locale::classic()).in(
            state, str.data(), str.data() + str.size(), pszNext, ret.data(), ret.data() + ret.size(), pwszNext);

        if(res == std::codecvt_base::error) return u"";

        // Remove extra null terminators that may have been created from multi-byte
        // UTF-8 characters
        while(ret.ends_with(u'\0'))
        {
            ret.pop_back();
        }

        return ret;
    }

    bool contains(const std::string& str, const std::string& substr)
    {
        return str.find(substr) != std::string::npos;
    }
}
