#pragma once

#include <options.hpp>

enum struct PermalinkError {
    NONE = 0,
    INVALID_VERSION,
    BAD_PERMALINK,
};

std::string create_permalink(const Settings& settings, const std::string& seed);
PermalinkError parse_permalink(std::string b64permalink, Settings& settings, std::string& seed);
std::string errorToName(PermalinkError err);
