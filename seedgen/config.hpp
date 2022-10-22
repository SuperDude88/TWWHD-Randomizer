#pragma once

#include <string>

#include <options.hpp>



enum struct [[nodiscard]] ConfigError {
    NONE = 0,
    COULD_NOT_OPEN,
    MISSING_KEY,
    DIFFERENT_FILE_VERSION,
    DIFFERENT_RANDO_VERSION,
    INVALID_VALUE,
    TOO_MANY_OF_ITEM,
    UNKNOWN,
    COUNT
};

struct Config {
    std::string gameBaseDir;
    std::string outputDir;

    std::string seed;

    Settings settings;
};

SwordMode nameToSwordMode(const std::string& name);

ConfigError createDefaultConfig(const std::string& filePath);

ConfigError loadFromFile(const std::string& filePath, Config& out);

ConfigError writeToFile(const std::string& filePath, const Config& config);

std::string errorToName(ConfigError err);
