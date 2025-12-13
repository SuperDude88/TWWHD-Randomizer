#include "string.hpp"

#include <locale>
#include <string>

namespace Utility::Str {
    // Can't use codecvt on Wii U, deprecated in c++17 and g++ hates it
    // Borrowed from https://docs.microsoft.com/en-us/cpp/standard-library/codecvt-class?view=msvc-170#out
    std::string toUTF8(const std::u16string& str) {
        if(str.empty()) return "";

        std::string ret;
        ret.resize(str.size());
        char* pszNext;
        const char16_t* pwszNext;
        std::mbstate_t state = {0}; // zero-initialization represents the initial conversion state for mbstate_t
        std::locale loc("C");
        int res = std::use_facet<std::codecvt<char16_t, char, mbstate_t>>(loc).out(state, str.c_str(), &str[str.size()], pwszNext,
        &ret[0], &ret[ret.size()], pszNext);

        if(res == std::codecvt_base::error) return "";
        return ret;
    }

    std::u16string toUTF16(const std::string& str)
    {
        if(str.empty()) return u"";

        std::u16string ret;
        ret.resize(str.size());
        const char* pszNext;
        char16_t* pwszNext;
        std::mbstate_t state = {0}; // zero-initialization represents the initial conversion state for mbstate_t
        std::locale loc("C");
        int res = std::use_facet<std::codecvt<char16_t, char, mbstate_t>>(loc).in(state, str.c_str(), &str[str.size()], pszNext,
        &ret[0], &ret[ret.size()], pwszNext);

        if(res == std::codecvt_base::error) return u"";

        // Remove extra null terminators that may have been created from multi-byte
        // UTF-8 characters
        while(ret.size() > 0 && ret[ret.size() - 1] == u'\0')
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
