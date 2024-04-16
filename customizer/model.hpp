#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <list>
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
    std::filesystem::path folder;

    std::vector<ColorPreset> presets;

    ColorPreset colors;

public:
    bool casual;
    std::string modelName;
    // holds ordering of the customizable colors in the gui
    std::list<std::string> heroOrdering;
    std::list<std::string> casualOrdering;

    const ColorMap_t& getDefaultColorsMap() const;
    const ColorMap_t& getSetColorsMap() const;
    const std::list<std::string>& getDefaultColorsOrdering() const;
    const std::vector<ColorPreset>& getPresets() const;
    const ColorPreset& getDefaultPreset() const;
    const std::string& getColor(const std::string& name_) const;
    void setColor(const std::string& name_, const std::string& color_);
    void resetSingleColor(const std::string& name);
    void resetColors();

    void loadPreset(const size_t& idx);
    void loadPreset(const std::string& name, const bool& both = true);
    void randomizeSingleColor(const std::string& name);
    void randomizeOrderly();
    void randomizeChaotically();

    ModelError loadFromFolder();
    ModelError applyModel() const;
};

std::string errorToName(const ModelError& err);
