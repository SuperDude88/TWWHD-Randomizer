#pragma once

#include <string>

#include <libs/yaml.hpp>

#include <options.hpp>
#include <utility/path.hpp>


enum struct [[nodiscard]] ConfigError {
    NONE = 0,
    COULD_NOT_OPEN,
    MISSING_KEY,
    DIFFERENT_FILE_VERSION,
    DIFFERENT_RANDO_VERSION,
    BAD_PERMALINK,
    INVALID_VALUE,
    MODEL_ERROR,
    UNKNOWN,
    COUNT
};

enum struct [[nodiscard]] PermalinkError {
    NONE = 0,
    EMPTY,
    BAD_ENCODING,
    MISSING_PARTS,
    INVALID_VERSION,
    INCORRECT_LENGTH,
    COULD_NOT_READ,
    UNHANDLED_OPTION,
    UNKNOWN,
    COUNT
};

class Config {
public:
    fspath gameBaseDir;
    fspath outputDir;

    std::string seed;
    Settings settings;

    bool converted = false;
    bool updated = false;
    bool configSet = false;

    Config();
    
    void resetDefaults();
    ConfigError loadFromFile(const fspath& filePath, const fspath& preferencesPath, bool ignoreErrors = false);
    YAML::Node settingsToYaml();
    YAML::Node preferencesToYaml();
    ConfigError writeSettings(const fspath& filePath);
    ConfigError writePreferences(const fspath& preferencesPath);
    ConfigError writeToFile(const fspath& filePath, const fspath& preferencesPath);

    PermalinkError loadPermalink(std::string b64permalink);
    std::string getPermalink(const bool& internal = false) const;
    
    static ConfigError writeDefault(const fspath& filePath, const fspath& preferencesPath);
};

std::string ConfigErrorGetName(ConfigError err);
std::string PermalinkErrorGetName(ConfigError err);
