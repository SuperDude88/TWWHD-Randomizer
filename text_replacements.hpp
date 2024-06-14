#pragma once

#include <logic/World.hpp>

#include <unordered_map>
#include <string>

using TextReplacements = std::unordered_map<std::string, std::unordered_map<std::string, std::u16string>>;

TextReplacements generate_text_replacements(World& world);
