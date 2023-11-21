#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

enum struct [[nodiscard]] ModelError {
    NONE = 0,
    COULD_NOT_OPEN,
    UNEXPECTED_VALUE,
    MISSING_KEY,
    UNKNOWN,
    COUNT
};

using ColorMap_t = std::unordered_map<std::string, std::string>;

class ColorPreset {
public:
    std::string name;

    ColorMap_t heroColors;
    ColorMap_t casualColors;

    ColorPreset(const std::string& name_ = "Default") :
        name(name_)
    {}
};

class CustomModel {
private:
    std::string modelName;
    std::filesystem::path folder;

    std::vector<ColorPreset> presets;

    ColorPreset colors;

public:
    bool casual;

    const ColorMap_t& getDefaultColorsMap() const;
    const ColorMap_t& getSetColorsMap() const;
    const std::vector<ColorPreset>& getPresets() const;
    const std::string& getColor(const std::string& name_) const;
    void setColor(const std::string& name_, const std::string& color_);
    void resetColors();

    void loadPreset(const size_t& idx);
    void randomizeOrderly();
    void randomizeChaotically();

    ModelError loadFromFolder(const std::string& name_);
    ModelError applyModel() const;
};
