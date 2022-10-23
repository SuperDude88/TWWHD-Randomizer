#include "string.hpp"

#include <locale>
#include <string>
#include <map>
#include <iostream>

namespace Utility::Str {
    bool endsWith(const std::string& str, const std::string& substr) {
        if (substr.size() > str.size()) return false;
        return std::equal(str.end() - substr.size(), str.end(), substr.begin());
    }

    //can't use codecvt on Wii U, deprecated in c++17 and g++ hates it
    //Borrowed from https://docs.microsoft.com/en-us/cpp/standard-library/codecvt-class?view=msvc-170#out
    std::string toUTF8(const std::u16string& str) {
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
        while (ret[ret.size() - 1] == u'\0')
        {
            ret.pop_back();
        }

        return ret;
    }

    // This map is for characters that the YAML parsing library seems to not like.
    // These characters will be translated into the literal string of their unicode
    // mapping and then tranalated back after YAML parsing.
    std::map<std::string, std::string> unicodeMappings = {
        {"Á", "\\u00C1"},
        {"à", "\\u00E0"},
        {"á", "\\u00E1"},
        {"â", "\\u00E2"},
        {"ç", "\\u00E7"},
        {"è", "\\u00E8"},
        {"é", "\\u00E9"},
        {"ê", "\\u00EA"},
        {"í", "\\u00ED"},
        {"î", "\\u00EE"},
        {"ï", "\\u00EF"},
        {"ñ", "\\u00F1"},
        {"ó", "\\u00F3"},
        {"ô", "\\u00F4"},
        {"ú", "\\u00FA"},
        {"œ", "\\u0153"},
    };

    std::string InsertUnicodeReplacements(std::string text)
    {
        for (auto& [c, unicode] : unicodeMappings)
        {
            auto pos = text.find(c);
            while (pos != std::string::npos)
            {
                text.erase(pos, c.length());
                text.insert(pos, unicode);
                pos = text.find(c);
            }
        }

        return text;
    }

    std::string RemoveUnicodeReplacements(std::string text)
    {
        for (auto& [c, unicode] : unicodeMappings)
        {
            auto pos = text.find(unicode);
            while (pos != std::string::npos)
            {
                text.erase(pos, unicode.length());
                text.insert(pos, c);
                pos = text.find(unicode);
            }
        }

        return text;
    }

    std::u16string RemoveUnicodeReplacements(std::u16string text)
    {
        std::map<std::u16string, std::u16string> unicodeMappingsUTF16 = {
            {u"Á", u"\\u00C1"},
            {u"à", u"\\u00E0"},
            {u"á", u"\\u00E1"},
            {u"â", u"\\u00E2"},
            {u"ç", u"\\u00E7"},
            {u"è", u"\\u00E8"},
            {u"é", u"\\u00E9"},
            {u"ê", u"\\u00EA"},
            {u"í", u"\\u00ED"},
            {u"î", u"\\u00EE"},
            {u"ï", u"\\u00EF"},
            {u"ñ", u"\\u00F1"},
            {u"ó", u"\\u00F3"},
            {u"ô", u"\\u00F4"},
            {u"ú", u"\\u00FA"},
            {u"œ", u"\\u0153"},
        };

        for (auto& [c, unicode] : unicodeMappingsUTF16)
        {
            auto pos = text.find(unicode);
            while (pos != std::string::npos)
            {
                text.erase(pos, unicode.length());
                text.insert(pos, c);
                pos = text.find(unicode);
            }
        }

        return text;
    }
}
