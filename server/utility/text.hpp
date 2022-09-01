#pragma once

#include <string>
#include <array>

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

    extern std::array<std::string, 3> supported_languages;

    std::u16string apply_name_color(std::u16string str, const Color& color);
    std::u16string word_wrap_string(const std::u16string& string, const size_t& max_line_len);
    std::string    pad_str_4_lines(const std::string& string);
    std::u16string pad_str_4_lines(const std::u16string& string);
}; // namespace Text
