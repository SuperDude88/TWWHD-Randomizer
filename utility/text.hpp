#pragma once

#include <string>
#include <array>
#include <map>

namespace Text
  {
    enum struct Type
    {
        STANDARD = 0,
        PRETTY,
        CRYPTIC,
    };

    enum struct Color
    {
        RAW = 0,
        NONE,
        RED,
        GREEN,
        BLUE,
        YELLOW,
        CYAN,
        MAGENTA,
        GRAY,
        ORANGE,
    };

    enum struct Gender
    {
        NONE = 0,
        MALE,
        FEMALE,
    };

    enum struct Plurality
    {
        SINGULAR,
        PLURAL,
    };

    struct Translation
    {
        std::map<Text::Type, std::string> types;
        Gender gender;
        Plurality plurality;
    };

    extern std::array<std::string, 3> supported_languages;

    std::u16string apply_name_color(std::u16string str, const Color& color);
    std::u16string word_wrap_string(const std::u16string& string, const size_t& max_line_len);
    std::string    pad_str_4_lines(const std::string& string);
    std::u16string pad_str_4_lines(const std::u16string& string);

    Gender string_to_gender(const std::string& str);
    Plurality string_to_plurality(const std::string& str); 
}; // namespace Text
