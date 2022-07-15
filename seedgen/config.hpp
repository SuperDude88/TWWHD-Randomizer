#pragma once

#include <string>

#include "../options.hpp"



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
    std::string workingDir;
    std::string outputDir;

    std::string seed;

    Settings settings;

    Config() = default;
    Config(const std::string& seed_, const Settings& settings_) :
        seed(seed_),
        settings(settings_)
    {}
};

ConfigError loadFromFile(const std::string& filePath, Config& out);

ConfigError writeToFile(const std::string& filePath, const Config& config);
