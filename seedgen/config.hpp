#pragma once

#include <string>
#include <filesystem>

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

class Config {
public:
    std::filesystem::path gameBaseDir;
    std::filesystem::path outputDir;

    std::string seed;

    bool repack_for_console;
    std::string consoleOutputDir;

    Settings settings;

    bool configSet = false;
    

    ConfigError loadFromFile(const std::string& filePath, bool ignoreErrors = false);
    ConfigError writeToFile(const std::string& filePath);
};

ConfigError createDefaultConfig(const std::string& filePath);

std::string errorToName(ConfigError err);

namespace DefaultColors {
    extern std::list<std::tuple<std::string, std::string>> heroColors;
    extern std::list<std::tuple<std::string, std::string>> casualColors;

    std::unordered_map<std::string, std::string> getDefaultColorsMap(const bool& casualClothes);
} // namespace DefaultColors