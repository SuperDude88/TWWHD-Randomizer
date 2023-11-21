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

    bool converted = false;
    bool updated = false;
    bool configSet = false;

    Config();
    
    void resetDefaults();
    ConfigError loadFromFile(const std::string& filePath, bool ignoreErrors = false);
    ConfigError writeToFile(const std::string& filePath);
    
    static ConfigError writeDefault(const std::string& filePath);
};

std::string errorToName(ConfigError err);
