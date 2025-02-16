#include "model.hpp"
#include <tuple>
#include <cmath>

#include <libs/yaml.hpp>
#include <logic/PoolFunctions.hpp>
#include <utility/endian.hpp>
#include <utility/color.hpp>
#include <utility/file.hpp>
#include <utility/platform.hpp>
#include <filetypes/bfres.hpp>
#include <command/RandoSession.hpp>


using eType = Utility::Endian::Type;

ModelError CustomModel::loadFromFolder() {
    auto model = modelName;
    if (user_provided || model == "") {
        model = "Link";
    }

    folder = Utility::dirExists(Utility::get_models_dir() / model) ? Utility::get_models_dir() / model : Utility::get_data_path() / "customizer" / "Link";
    
    presets.clear();
    heroOrdering.clear();
    casualOrdering.clear();

    std::string metaStr;
    if(Utility::getFileContents(folder / "metadata.yaml", metaStr, true) != 0) {
        return ModelError::COULD_NOT_OPEN;
    }

    YAML::Node metaTree = YAML::Load(metaStr);

    if(!metaTree["default_hero_colors"] || !metaTree["default_casual_colors"]) {
        return ModelError::MISSING_KEY;
    }

    ColorPreset& defaultPreset = presets.emplace_back();
    for (const auto& presetObject : metaTree["default_hero_colors"]) {
        for (const auto& presetColor : presetObject)
        {
            const std::string colorName = presetColor.first.as<std::string>();
            const std::string color = presetColor.second.as<std::string>();
            defaultPreset.heroColors[colorName] = color;
            heroOrdering.emplace_back(colorName);
        }
    }
    for (const auto& presetObject : metaTree["default_casual_colors"]) {
        for (const auto& presetColor : presetObject)
        {
            const std::string colorName = presetColor.first.as<std::string>();
            const std::string color = presetColor.second.as<std::string>();
            defaultPreset.casualColors[colorName] = color;
            casualOrdering.emplace_back(colorName);
        }
    }

    casual = metaTree["default_casual"].as<bool>();

    if(std::string presetStr; Utility::getFileContents(folder / "color_presets.yaml", presetStr, true) == 0) {
        YAML::Node presetTree = YAML::Load(presetStr);

        // Loop through and add each preset
        for (const auto& presetObject : presetTree) {
            const std::string presetName = presetObject["Name"].as<std::string>();
            ColorPreset& preset = presets.emplace_back(presetName);

            for (const auto& presetColor : presetObject["Colors"]["hero"]) {
                const std::string colorName = presetColor.first.as<std::string>();
                const std::string color = presetColor.second.as<std::string>();

                preset.heroColors[colorName] = color;
            }
            for (const auto& presetColor : presetObject["Colors"]["casual"]) {
                const std::string colorName = presetColor.first.as<std::string>();
                const std::string color = presetColor.second.as<std::string>();

                preset.casualColors[colorName] = color;
            }

            // Add in defaults for unspecified colors
            for (auto& [colorName, color] : getDefaultPreset().heroColors) {
                if (!preset.heroColors.contains(colorName)) {
                    preset.heroColors[colorName] = color;
                }
            }
            for (auto& [colorName, color] : getDefaultPreset().casualColors) {
                if (!preset.casualColors.contains(colorName)) {
                    preset.casualColors[colorName] = color;
                }
            }
        }
    }

    return ModelError::NONE;
}

const ColorMap_t& CustomModel::getDefaultColorsMap() const {
    if(presets.size() == 0) return {};

    if(casual) {
        return presets.front().casualColors;
    }
    else {
        return presets.front().heroColors;
    }
}

const ColorMap_t& CustomModel::getSetColorsMap() const {
    if(casual) {
        return colors.casualColors;
    }
    else {
        return colors.heroColors;
    }
}

const std::list<std::string>& CustomModel::getDefaultColorsOrdering() const {
    if(casual) {
        return casualOrdering;
    }
    else {
        return heroOrdering;
    }
}

const std::vector<ColorPreset>& CustomModel::getPresets() const {
    return presets;
}

const ColorPreset& CustomModel::getDefaultPreset() const {
    return presets[0];
}

const std::string& CustomModel::getColor(const std::string& name_) const {
    const ColorMap_t& custom = getSetColorsMap();
    if(custom.contains(name_)) {
        return custom.at(name_);
    }
    
    const ColorMap_t& def = getDefaultColorsMap();
    if(def.contains(name_)) {
        return def.at(name_);
    }

    return ""; //error somehow?
}

void CustomModel::setColor(const std::string& name_, const std::string& color_) {
    if(casual) {
        colors.casualColors[name_] = color_;
    }
    else {
        colors.heroColors[name_] = color_;
    }
}

void CustomModel::resetSingleColor(const std::string& name) {
    if(casual) {
        colors.casualColors.erase(name);
    }
    else {
        colors.heroColors.erase(name);
    }
}

void CustomModel::resetColors() {
    if(casual) {
        colors.casualColors.clear();
    }
    else {
        colors.heroColors.clear();
    }
}

void CustomModel::loadPreset(const size_t& idx) {
    colors = presets[idx];
}

void CustomModel::loadPreset(const std::string& name, const bool& both /*= true*/) {
    for (const auto& preset : presets) {
        if (preset.name == name) {
            if (both) {
                colors = preset;
            } else if (casual) {
                colors.casualColors = preset.casualColors;
            } else {
                colors.heroColors = preset.heroColors;
            }
            break;
        }
    }
}

void CustomModel::randomizeSingleColor(const std::string& name) {
    const auto& defaultColor = getDefaultColorsMap().at(name);

    const auto [hShift, vShift] = get_random_h_and_v_shifts_for_custom_color(defaultColor);
    setColor(name, HSVShiftColor(defaultColor, hShift, vShift));
}

void CustomModel::randomizeOrderly() {
    const ColorMap_t& defaultColors = getDefaultColorsMap();

    const int hShift = rand() % 360;
    const int vShift = (rand() % 81) - 40;
    for (const auto& [customColorName, color] : defaultColors) {
        setColor(customColorName, HSVShiftColor(color, hShift, vShift));
    }
}

void CustomModel::randomizeChaotically() {
    const ColorMap_t& defaultColors = getDefaultColorsMap();

    for (auto& [customColorName, color] : defaultColors) {
        const auto [hShift, vShift] = get_random_h_and_v_shifts_for_custom_color(color);
        setColor(customColorName, HSVShiftColor(color, hShift, vShift));
    }
}

// Maps each recolor option to the texture files that need recoloring
static const std::unordered_map<std::string, std::list<std::string>> heroTextureMappings = {
    {"Hair", {"linktexS3TC"}},
    {"Skin", {"linktexS3TC", "handsS3TC", "mouthS3TC.1", "mouthS3TC.2", "mouthS3TC.3", "mouthS3TC.4", "mouthS3TC.5", "mouthS3TC.6", "mouthS3TC.7", "mouthS3TC.8", "mouthS3TC.9"}},
    {"Mouth", {"mouthS3TC.2", "mouthS3TC.3", "mouthS3TC.6", "mouthS3TC.7"}},
    {"Eyes", {"hitomi"}},
    {"Sclera", {"hitomi"}},
    {"Tunic", {"linktexS3TC"}},
    {"Undershirt", {"linktexS3TC"}},
    {"Pants", {"linktexS3TC"}},
    {"Boots", {"linktexS3TC"}},
    {"Belt", {"linktexS3TC"}},
    {"Belt Buckle", {"linktexS3TC"}},
};

static const std::unordered_map<std::string, std::list<std::string>> casualTextureMappings = {
    {"Hair", {"linktexbci4", "katsuraS3TC"}},
    {"Skin", {"linktexbci4", "handsS3TC", "mouthS3TC.1", "mouthS3TC.2", "mouthS3TC.3", "mouthS3TC.4", "mouthS3TC.5", "mouthS3TC.6", "mouthS3TC.7", "mouthS3TC.8", "mouthS3TC.9"}},
    {"Mouth", {"mouthS3TC.2", "mouthS3TC.3", "mouthS3TC.6", "mouthS3TC.7"}},
    {"Eyes", {"hitomi"}},
    {"Sclera", {"hitomi"}},
    {"Shirt", {"linktexbci4"}},
    {"Shirt Emblem", {"linktexbci4"}},
    {"Armbands", {"linktexbci4"}},
    {"Pants", {"linktexbci4"}},
    {"Shoes", {"linktexbci4"}},
    {"Shoe Soles", {"linktexbci4"}},
};


/*void CustomModel::nextModel() {
    // Each time the button is clicked, we load the directories in the
    // models dir, sort it, and then binsearch the current model in it.
    // We then go to the next one.
    //
    // This works fine even if binsearch fails: if a model was deleted, we
    // just go to the next one alphabetically.
    //
    // This also works fine when customModel is "": this will give the first
    // element of the list.
    
    auto dir = Utility::get_models_dir();

    std::vector<fspath> models;
    for (auto entry : std::filesystem::directory_iterator(dir)) {
        if (!entry.is_directory()) continue;
        models.push_back(entry.path().filename());
    }
    std::sort(models.begin(), models.end());
    // std::lower_bound returns the least element greater than or equal to
    // current. Thus we need to do an extra == below to check if we found
    // it and need to advance to the next slot or not.
    auto found = std::lower_bound(models.begin(), models.end(), modelName);
    
    if (found == models.end()) {
        // Wrap back around.
        modelName = "";
        user_provided = false;
    } else if (*found == modelName) {
        // Advance to the next one, unless this is the last one.
        auto idx = found - models.begin();
        idx++;
        if (idx < models.size()) {
            modelName = models[idx];
            user_provided = true;
        } else {
            modelName = "";
            user_provided = false;
        }
    } else {
        // Seems that the current value isn't actually a model.
        // The model we found is the next one after it.
        modelName = *found;
        user_provided = true;
    }
}
*/

// IMPROVEMENT: Better generalize this in the future
ModelError CustomModel::applyModel() const {
    //start by counting the folders in case the user decided to select a random model even though theres no model
    int fileCount = std::count_if(
        begin(std::filesystem::directory_iterator(Utility::get_models_dir())),
        end(std::filesystem::directory_iterator(Utility::get_models_dir())),
        [](auto& entry) { return entry.is_directory(); }
    );
    if (!modelName.empty() && fileCount != 0) {

        Utility::platformLog("Applying custom model " + modelName + "...");

        auto model = Utility::get_models_dir() / modelName;
        if (modelName == "random") {
            //we'll pick a random model from the models 
            //theres probably a better way to do that but thats not very important
            size_t n = 0;
            size_t chosen = rand() % fileCount;
            for (const auto& entry : std::filesystem::recursive_directory_iterator(Utility::get_models_dir())) {
                if (chosen == n) {
                    model = entry.path().string();
                }
                n++;
            }
        } 
        
        struct Resource {
            std::string_view src, dst;
            bool required = false;
        };

        static constexpr Resource resources[] = {
            {"Link.szs", "content/Common/Pack/permanent_3d.pack@SARC@Link.szs", true},
            // TODO: DungeonMapLink_00.szs, LinkPos_00.szs
            
            // Sound clips.
            {"voice_0.aw", "content/Cafe/US/AudioRes/JAudioRes/Banks/voice_0.aw"},
            {"JaiInit.aaf", "content/Cafe/US/AudioRes/JAudioRes/JaiInit.aaf"}
        };

        for (auto rec : resources) {
            auto src = model / rec.src;
            if (!std::filesystem::exists(src)) {
                //if the file doesn't exist we'll restore the original
                g_session.openGameFile(rec.dst);
                //also check if it was required.
                if (rec.required) {
                    return ModelError::COULD_NOT_OPEN;
                }
                continue;
            }

            Utility::platformLog("Patching " + std::string(rec.src) + "...");
            if (!g_session.copyToGameFile(src, rec.dst)) {
                Utility::platformLog("Couldn't patch " + std::string(rec.dst));
                return ModelError::COULD_NOT_OPEN;
            }
        }

        Utility::platformLog("Applied custom model " + modelName + "!");
        return ModelError::NONE;
    }
    //openGameFile fetches files from vanilla install and stores them into the rando install
    //Restore original files in case a custom model was perviously used (voice clips only, as permanent_3d will be opened after that anyway)
    g_session.openGameFile("content/Cafe/US/AudioRes/JAudioRes/Banks/voice_0.aw");
    g_session.openGameFile("content/Cafe/US/AudioRes/JAudioRes/JaiInit.aaf");

    RandoSession::CacheEntry& link = g_session.openGameFile("content/Common/Pack/permanent_3d.pack@SARC@Link.szs@YAZ0@SARC@Link.bfres@BFRES");
    link.addAction([&](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(bfres, FileTypes::resFile, data)

        std::string maskFile = "";
        uint16_t baseColor = 0;
        uint16_t replacementColor = 0;

        auto baseColors = getDefaultColorsMap();
        std::unordered_map<std::string, std::list<std::string>> textureMappings = {};
        if (casual) {
            textureMappings = casualTextureMappings;
        } else {
            textureMappings = heroTextureMappings;
        }

        for (FileTypes::Subfiles::FTEXFile& texture : bfres.textures) {
            for (auto& [name, textureNames] : textureMappings) {
                replacementColor = hexColorStrTo16Bit(getColor(name));
                baseColor = hexColorStrTo16Bit(baseColors[name]);

                // Don't modify colors if it's not necessary
                if (baseColor == replacementColor) {
                    continue;
                }

                for (std::string& textureName : textureNames) {
                    if (texture.name.substr(0, textureName.length()) == textureName) {
                        // Get the data from the mask file
                        const std::string filename = (casual ? "casual" : "hero") + name + "_" + textureName + "_mask.bftex";

                        if (Utility::getFileContents(folder / "color_masks" / filename, maskFile, true) != 0) {
                            Utility::platformLog("Could not open " + filename + " mask file. Will skip recoloring " + name);
                            continue;
                        }

                        // Textures are stored using various BCn compression formats

                        // Actual texture data doesn't start until 0x2000, so remove
                        // everything before that
                        maskFile = maskFile.substr(0x2000);
                        for (size_t i = 0; i < maskFile.length(); i += 8) {
                            // Skip over alpha data in BC3 format
                            if (texture.format == GX2_SURFACE_FORMAT_SRGB_BC3) {
                                i += 8;
                            }

                            // The mask file color tells us if this is a color we should
                            // replace or not in the current texture
                            const uint16_t maskColor1  = Utility::Endian::toPlatform(eType::Big, *(uint16_t*)&maskFile[i]);
                            const uint16_t maskColor2  = Utility::Endian::toPlatform(eType::Big, *(uint16_t*)&maskFile[i + 2]);

                            uint16_t texColor1;
                            uint16_t texColor2;

                            // The most defined texture data is stored in texture.data
                            // All smaller mipmaps are stored in texture.mipData
                            // Using little endian here is intentional
                            if (i < texture.data.length()) {
                                texColor1  = Utility::Endian::toPlatform(eType::Little, *(uint16_t*)&texture.data[i]);
                                texColor2  = Utility::Endian::toPlatform(eType::Little, *(uint16_t*)&texture.data[i + 2]);
                            } else if (i >= texture.data.length() && i < texture.data.length() + texture.mipData.length()) {
                                texColor1  = Utility::Endian::toPlatform(eType::Little, *(uint16_t*)&texture.mipData[i - texture.data.length()]);
                                texColor2  = Utility::Endian::toPlatform(eType::Little, *(uint16_t*)&texture.mipData[i + 2 - texture.data.length()]);
                            }

                            std::list<std::tuple<uint16_t, size_t, uint16_t>> masksOffsetsColors = {{maskColor1, i, texColor1} , {maskColor2, i + 2, texColor2}};

                            // For the two colors in this iteration
                            for (auto& [mask, offset, curColor] : masksOffsetsColors) {
                                if (mask == 0x00F8) {
                                    // TEMP FIX: Remove red from really dark base eye colors, otherwise
                                    // we can get some really light colors back that look weird
                                    if (name == "Eyes") {
                                        curColor &= 0x07FF;
                                    }

                                    uint16_t newColor = colorExchange(baseColor, replacementColor, curColor);
                                    Utility::Endian::toPlatform_inplace(eType::Little, newColor);

                                    if (offset < texture.data.length()) {
                                        texture.data.replace(offset, 2, reinterpret_cast<const char*>(&newColor), 2);
                                    } else if (i >= texture.data.length() && i < texture.data.length() + texture.mipData.length()) {
                                        texture.mipData.replace(offset - texture.data.length(), 2, reinterpret_cast<const char*>(&newColor), 2);
                                    }
                                }
                            }

                            // Check and potentially change pixel indices to avoid 
                            // accidental transparent colors in BC1 format
                            if (isAnyOf(0xF8, maskColor1, maskColor2) && texture.format == GX2_SURFACE_FORMAT_SRGB_BC1) {
                                uint16_t newColor1 = 0;
                                uint16_t newColor2 = 0;
                                uint32_t indices = 0;
                                uint32_t newIndices = 0;

                                if (i < texture.data.length()) {
                                    newColor1 = Utility::Endian::toPlatform(eType::Little, *(uint16_t*)&texture.data[i]);
                                    newColor2 = Utility::Endian::toPlatform(eType::Little, *(uint16_t*)&texture.data[i + 2]);
                                    indices   = Utility::Endian::toPlatform(eType::Little, *(uint32_t*)&texture.data[i + 4]);
                                } else if (i >= texture.data.length() && i < texture.data.length() + texture.mipData.length()) {
                                    newColor1 = Utility::Endian::toPlatform(eType::Little, *(uint16_t*)&texture.mipData[i - texture.data.length()]);
                                    newColor2 = Utility::Endian::toPlatform(eType::Little, *(uint16_t*)&texture.mipData[i + 2 - texture.data.length()]);
                                    indices   = Utility::Endian::toPlatform(eType::Little, *(uint32_t*)&texture.mipData[i + 4 - texture.data.length()]);
                                }

                                // If the first color is less than the second color,
                                // change any pixels using index 3 to use index 2. Index
                                // 3 in this case is used to denote transparent pixels
                                if (newColor1 <= newColor2 && texColor1 > texColor2) {

                                    for (size_t j = 0; j < 32; j += 2) {

                                        uint32_t curIndex = (indices & (0b11 << j)) >> j;
                                        if (curIndex == 3) {
                                            curIndex = 2;
                                        }
                                        newIndices |= (curIndex << j);
                                    }

                                    if (i < texture.data.length()) {
                                        texture.data.replace(i + 4, 4, reinterpret_cast<const char*>(&newIndices), 4);
                                    } else if (i >= texture.data.length() && i < texture.data.length() + texture.mipData.length()) {
                                        texture.mipData.replace(i + 4 - texture.data.length(), 4, reinterpret_cast<const char*>(&newIndices), 4);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return true;
    });

    return ModelError::NONE;
}

std::string errorToName(const ModelError& err) {
    switch(err)
    {
    case ModelError::COULD_NOT_OPEN:
        return "COULD_NOT_OPEN";
    case ModelError::UNEXPECTED_VALUE:
        return "UNEXPECTED_VALUE";
    case ModelError::MISSING_KEY:
        return "MISSING_KEY";
    default:
        return "UNKNOWN";
    }
}