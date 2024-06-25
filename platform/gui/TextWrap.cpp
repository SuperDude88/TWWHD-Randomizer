#include "TextWrap.hpp"

std::vector<std::string> wrap_string(const std::string& string, const size_t& max_line_len) {
    std::vector<std::string> wrapped_lines = {""};

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

            // if the first word is longer than the line - add what we have so far, then split it and move to a new one
            // if the current word + prior words are longer than the line - split at the word boundary and move to a new one
            // if the current word + prior words were too long and the line was split, and then the current word alone becomes longer than the line,
            // this check order ensures that the large word is added to the previous blank line before it breaks, instead
            // of adding a second newline and leaving the first one blank
            if (current_word.size() > max_line_len) {
                wrapped_lines.back() += current_word;
                wrapped_lines.emplace_back("");
                current_word = "";
            }
            else if (wrapped_lines.back().size() + current_word.size() > max_line_len) {
                wrapped_lines.emplace_back("");
            }
        }
    }

    wrapped_lines.back() += current_word;

    return wrapped_lines;
}
