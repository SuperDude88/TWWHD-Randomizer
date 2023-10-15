#include "TextWrap.hpp"

std::vector<std::string> wrap_string(const std::string& string, const size_t& max_line_len) {
    std::vector<std::string> wrapped_lines;

    std::string current_word;
    for(const char& character : string) {
        if (character == '\n') {
            wrapped_lines.back() += current_word;
            wrapped_lines.emplace_back("");
            current_word = "";
        }
        else if (character == ' ') {
            wrapped_lines.back() += current_word + character;
            current_word = "";
        }
        else {
            current_word += character;

            if (wrapped_lines.back().size() + current_word.size() > max_line_len) {
                wrapped_lines.emplace_back("");

                if (current_word.size() > max_line_len) {
                    wrapped_lines.back() += current_word;
                    wrapped_lines.emplace_back("");
                    current_word = "";
                }
            }
        }
    }

    wrapped_lines.back() += current_word;

    return wrapped_lines;
}
