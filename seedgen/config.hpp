#pragma once

#include <string>
#include <filesystem>

#include <options.hpp>
#include <libs/yaml.hpp>


enum struct [[nodiscard]] ConfigError {
    NONE = 0,
    COULD_NOT_OPEN,
    MISSING_KEY,
    DIFFERENT_FILE_VERSION,
    DIFFERENT_RANDO_VERSION,
    INVALID_VALUE,
    MODEL_ERROR,
    UNKNOWN,
    COUNT
};

class Config {
public:
    std::filesystem::path gameBaseDir;
    std::filesystem::path outputDir;

    std::string seed;
    Settings settings;
    std::string permalink;

    bool converted = false;
    bool updated = false;
    bool configSet = false;

    Config();
    
    void resetDefaults();
    ConfigError loadFromFile(const std::string& filePath, const std::string& preferencesPath, bool ignoreErrors = false);
    YAML::Node settingsToYaml();
    YAML::Node preferencesToYaml();
    ConfigError writeSettings(const std::string& filePath);
    ConfigError writePreferences(const std::string& preferencesPath);
    ConfigError writeToFile(const std::string& filePath, const std::string& preferencesPath);
    
    static ConfigError writeDefault(const std::string& filePath, const std::string& preferencesPath);
};

std::string errorToName(ConfigError err);
