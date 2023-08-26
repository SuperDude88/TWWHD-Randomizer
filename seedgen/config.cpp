#include "config.hpp"

#include <fstream>
#include <unordered_set>
#include <filesystem>

#include <version.hpp>
#include <libs/yaml.h>
#include <logic/GameItem.hpp>
#include <seedgen/random.hpp>
#include <utility/color.hpp>
#include <utility/platform.hpp>
#include <command/Log.hpp>



#define GET_FIELD(yaml, name, out) {                                        \
        if(!yaml[#name]) {                                                  \
            Utility::platformLog("\""#name"\" not found in config.yaml\n"); \
            if (!ignoreErrors) return ConfigError::MISSING_KEY;}            \
        out = yaml[#name].as<std::string>();                                \
    }

#define SET_CONFIG_FIELD(yaml, config, name) {                              \
        if(!yaml[#name]) {                                                  \
            Utility::platformLog("\""#name"\" not found in config.yaml\n"); \
            if (!ignoreErrors) return ConfigError::MISSING_KEY;}            \
        config.name = yaml[#name].as<std::string>();                        \
    }

#define SET_CONFIG_BOOL_FIELD(yaml, config, name) {                         \
        if(!yaml[#name]) {                                                  \
            Utility::platformLog("\""#name"\" not found in config.yaml\n"); \
            if (!ignoreErrors) return ConfigError::MISSING_KEY;}            \
        config.name = yaml[#name].as<bool>();                               \
    }

#define SET_FIELD_EMPTY_STR_IF_FAIL(yaml, config, name) {                   \
        if(!yaml[#name])                                                    \
            config.name = "";                                               \
        config.name = yaml[#name].as<std::string>();                        \
    }

#define SET_BOOL_FIELD(yaml, config, name) {                                \
        if(!yaml[#name]) {                                                  \
            Utility::platformLog("\""#name"\" not found in config.yaml\n"); \
            if (!ignoreErrors) return ConfigError::MISSING_KEY;}            \
        config.settings.name = yaml[#name].as<bool>();                      \
    }

#define SET_INT_FIELD(yaml, config, name) {                                 \
        if(!yaml[#name]) {                                                  \
            Utility::platformLog("\""#name"\" not found in config.yaml\n"); \
            if (!ignoreErrors) return ConfigError::MISSING_KEY;}            \
        config.settings.name = yaml[#name].as<int>();                       \
    }

#define SET_STR_FIELD(yaml, config, name) {                                 \
        if(!yaml[#name]) {                                                  \
            Utility::platformLog("\""#name"\" not found in config.yaml\n"); \
            if (!ignoreErrors) return ConfigError::MISSING_KEY;}            \
        config.settings.name = yaml[#name].as<std::string>();               \
    }

#define SET_STR_FIELD_EMPTY_STR_IF_FAIL(yaml, config, name) {               \
        if(!yaml[#name])                                                    \
            config.settings.name = "";                                      \
        config.settings.name = yaml[#name].as<std::string>();               \
    }

// Default colors for every texture
// Use list of tuples to keep ordering
namespace DefaultColors {
    std::list<std::tuple<std::string, std::string>> heroColors = {
        {"Hair",        "FFEF10"},
        {"Skin",        "F7DB9C"},
        {"Mouth",       "F74963"},
        {"Eyes",        "10514A"},
        {"Sclera",      "FFFFFF"},
        // {"Hat",         "5AB24A"},
        {"Tunic",       "5AB24A"},
        {"Undershirt",  "ADE342"},
        {"Pants",       "FFFFFF"},
        {"Boots",       "944908"},
        {"Belt",        "633008"},
        {"Belt Buckle", "FFEF10"},
    };

    std::list<std::tuple<std::string, std::string>> casualColors = {
        {"Hair",         "FFEF10"},
        {"Skin",         "F7DB9C"},
        {"Mouth",        "F74963"},
        {"Eyes",         "10514A"},
        {"Sclera",       "FFFFFF"},
        {"Shirt",        "4A75AD"},
        {"Shirt Emblem", "FFFFD6"},
        {"Armbands",     "63696B"},
        {"Pants",        "FFA608"},
        {"Shoes",        "63696B"},
        {"Shoe Soles",   "D6BA39"},
    };

    std::unordered_map<std::string, std::string> getDefaultColorsMap(const bool& casualClothes) {
        std::unordered_map<std::string, std::string> map = {};

        auto colors = casualClothes ? casualColors : heroColors;

        for (auto& [name, color] : colors) {
            map[name] = color;
        }
        return map;
    }
} // namespace DefaultColors


ConfigError createDefaultConfig(const std::string& filePath) {
    Config conf;

    conf.gameBaseDir = "";
    conf.outputDir = "";
    conf.seed = "";
    conf.repack_for_console = false;
    conf.consoleOutputDir = "";

    conf.settings.progression_dungeons = ProgressionDungeons::Standard;
    conf.settings.progression_great_fairies = true;
    conf.settings.progression_puzzle_secret_caves = true;
    conf.settings.progression_combat_secret_caves = false;
    conf.settings.progression_short_sidequests = false;
    conf.settings.progression_long_sidequests = false;
    conf.settings.progression_spoils_trading = false;
    conf.settings.progression_minigames = false;
    conf.settings.progression_free_gifts = true;
    conf.settings.progression_mail = false;
    conf.settings.progression_platforms_rafts = false;
    conf.settings.progression_submarines = false;
    conf.settings.progression_eye_reef_chests = false;
    conf.settings.progression_big_octos_gunboats = false;
    conf.settings.progression_triforce_charts = false;
    conf.settings.progression_treasure_charts = false;
    conf.settings.progression_expensive_purchases = true;
    conf.settings.progression_misc = true;
    conf.settings.progression_tingle_chests = false;
    conf.settings.progression_battlesquid = false;
    conf.settings.progression_savage_labyrinth = false;
    conf.settings.progression_island_puzzles = false;
    conf.settings.progression_obscure = false;

    conf.settings.dungeon_small_keys = PlacementOption::OwnDungeon;
    conf.settings.dungeon_big_keys = PlacementOption::OwnDungeon;
    conf.settings.dungeon_maps_compasses = PlacementOption::OwnDungeon;
    conf.settings.randomize_charts = false;
    conf.settings.randomize_starting_island = false;
    conf.settings.randomize_dungeon_entrances = false;
    conf.settings.randomize_boss_entrances = false;
    conf.settings.randomize_cave_entrances = false;
    conf.settings.randomize_door_entrances = false;
    conf.settings.randomize_misc_entrances = false;
    conf.settings.mix_dungeons = false;
    conf.settings.mix_bosses = false;
    conf.settings.mix_caves = false;
    conf.settings.mix_doors = false;
    conf.settings.mix_misc = false;
    conf.settings.decouple_entrances = false;

    conf.settings.korl_hints = false;
    conf.settings.ho_ho_hints = false;
    conf.settings.path_hints = false;
    conf.settings.barren_hints = false;
    conf.settings.item_hints = false;
    conf.settings.location_hints = false;
    conf.settings.use_always_hints = false;
    conf.settings.clearer_hints = false;

    conf.settings.instant_text_boxes = true;
    conf.settings.fix_rng = false;
    conf.settings.reveal_full_sea_chart = true;
    conf.settings.num_starting_triforce_shards = 0;
    conf.settings.add_shortcut_warps_between_dungeons = false;
    conf.settings.do_not_generate_spoiler_log = false;
    conf.settings.sword_mode = SwordMode::StartWithSword;
    conf.settings.skip_rematch_bosses = true;
    conf.settings.invert_sea_compass_x_axis = false;
    conf.settings.num_required_dungeons = 0;
    conf.settings.damage_multiplier = 2.0f;
    conf.settings.chest_type_matches_contents = false;

    conf.settings.player_in_casual_clothes = false;
    conf.settings.pig_color = PigColor::RANDOM;

    conf.settings.starting_gear = {
        GameItem::ProgressiveShield,
        GameItem::BalladOfGales,
        GameItem::SongOfPassing,
        GameItem::ProgressiveMagicMeter,
        GameItem::ProgressiveSail
    };

    conf.settings.starting_pohs = 0;
    conf.settings.starting_hcs = 0;
    conf.settings.starting_joy_pendants = 0;
    conf.settings.starting_skull_necklaces = 0;
    conf.settings.starting_boko_baba_seeds = 0;
    conf.settings.starting_golden_feathers = 0;
    conf.settings.starting_knights_crests = 0;
    conf.settings.starting_red_chu_jellys = 0;
    conf.settings.starting_green_chu_jellys = 0;
    conf.settings.starting_blue_chu_jellys = 0;
    conf.settings.remove_music = false;

    conf.settings.do_not_generate_spoiler_log = false;
    conf.settings.start_with_random_item = false;
    conf.settings.plandomizer = false;
    conf.settings.plandomizerFile = "";

    conf.settings.target_type = TargetTypePreference::Hold;
    conf.settings.camera = CameraPreference::Standard;
    conf.settings.first_person_camera = FirstPersonCameraPreference::Standard;
    conf.settings.gyroscope = GyroscopePreference::On;
    conf.settings.ui_display = UIDisplayPreference::On;

    for (auto& [name, color] : DefaultColors::heroColors) {
        conf.settings.custom_colors[name] = color;
    }

    LOG_AND_RETURN_IF_ERR(writeToFile(filePath, conf))

    return ConfigError::NONE;
}

ConfigError loadFromFile(const std::string& filePath, Config& out, bool ignoreErrors /*= false*/) {
    //Check if we can open the file before parsing because exceptions won't work on console
    std::ifstream file(filePath);
    if(!file.is_open()) LOG_ERR_AND_RETURN(ConfigError::COULD_NOT_OPEN);
    file.close();
    
    YAML::Node root = YAML::LoadFile(filePath);

    std::string rando_version, file_version;
    GET_FIELD(root, program_version, rando_version)
    GET_FIELD(root, file_version, file_version)

    if(std::string(CONFIG_VERSION) != file_version && !ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::DIFFERENT_FILE_VERSION);

    //hardcode paths for console, otherwise use config
    #ifdef DEVKITPRO
        //these are now loaded elsewhere
        /* out.gameBaseDir = "storage_mlc01:/usr/title/00050000/10143500"; */
        /* out.outputDir = "storage_mlc01:/usr/title/00050000/10143599"; */

        out.settings.plandomizerFile = APP_SAVE_PATH "plandomizer.yaml";
    #else
        SET_FIELD_EMPTY_STR_IF_FAIL(root, out, gameBaseDir)
        SET_FIELD_EMPTY_STR_IF_FAIL(root, out, outputDir)
        SET_CONFIG_BOOL_FIELD(root, out, repack_for_console)
        SET_FIELD_EMPTY_STR_IF_FAIL(root, out, consoleOutputDir)
        SET_STR_FIELD_EMPTY_STR_IF_FAIL(root, out, plandomizerFile)
    #endif

    out.configSet = true;

    SET_FIELD_EMPTY_STR_IF_FAIL(root, out, seed)

    if(!root["progression_dungeons"]) {
      if (!ignoreErrors) return ConfigError::MISSING_KEY;
    } else {
      out.settings.progression_dungeons = nameToProgressionDungeons(root["progression_dungeons"].as<std::string>());
      if (out.settings.progression_dungeons == ProgressionDungeons::INVALID) {
        if (!ignoreErrors) return ConfigError::INVALID_VALUE;
        else out.settings.progression_dungeons = ProgressionDungeons::Standard;
      }
    }
    SET_BOOL_FIELD(root, out, progression_great_fairies)
    SET_BOOL_FIELD(root, out, progression_puzzle_secret_caves)
    SET_BOOL_FIELD(root, out, progression_combat_secret_caves)
    SET_BOOL_FIELD(root, out, progression_short_sidequests)
    SET_BOOL_FIELD(root, out, progression_long_sidequests)
    SET_BOOL_FIELD(root, out, progression_spoils_trading)
    SET_BOOL_FIELD(root, out, progression_minigames)
    SET_BOOL_FIELD(root, out, progression_free_gifts)
    SET_BOOL_FIELD(root, out, progression_mail)
    SET_BOOL_FIELD(root, out, progression_platforms_rafts)
    SET_BOOL_FIELD(root, out, progression_submarines)
    SET_BOOL_FIELD(root, out, progression_eye_reef_chests)
    SET_BOOL_FIELD(root, out, progression_big_octos_gunboats)
    SET_BOOL_FIELD(root, out, progression_triforce_charts)
    SET_BOOL_FIELD(root, out, progression_treasure_charts)
    SET_BOOL_FIELD(root, out, progression_expensive_purchases)
    SET_BOOL_FIELD(root, out, progression_misc)
    SET_BOOL_FIELD(root, out, progression_tingle_chests)
    SET_BOOL_FIELD(root, out, progression_battlesquid)
    SET_BOOL_FIELD(root, out, progression_savage_labyrinth)
    SET_BOOL_FIELD(root, out, progression_island_puzzles)
    SET_BOOL_FIELD(root, out, progression_obscure)

    SET_BOOL_FIELD(root, out, randomize_charts)
    SET_BOOL_FIELD(root, out, randomize_starting_island)
    SET_BOOL_FIELD(root, out, randomize_dungeon_entrances)
    SET_BOOL_FIELD(root, out, randomize_boss_entrances)
    SET_BOOL_FIELD(root, out, randomize_cave_entrances)
    SET_BOOL_FIELD(root, out, randomize_door_entrances)
    SET_BOOL_FIELD(root, out, randomize_misc_entrances)
    SET_BOOL_FIELD(root, out, mix_dungeons)
    SET_BOOL_FIELD(root, out, mix_bosses)
    SET_BOOL_FIELD(root, out, mix_caves)
    SET_BOOL_FIELD(root, out, mix_doors)
    SET_BOOL_FIELD(root, out, mix_misc)
    SET_BOOL_FIELD(root, out, decouple_entrances)

    SET_BOOL_FIELD(root, out, ho_ho_hints)
    SET_BOOL_FIELD(root, out, korl_hints)
    SET_BOOL_FIELD(root, out, clearer_hints)
    SET_BOOL_FIELD(root, out, use_always_hints)
    SET_INT_FIELD(root, out, path_hints)
    SET_INT_FIELD(root, out, barren_hints)
    SET_INT_FIELD(root, out, item_hints)
    SET_INT_FIELD(root, out, location_hints)

    SET_BOOL_FIELD(root, out, instant_text_boxes)
    SET_BOOL_FIELD(root, out, fix_rng)
    SET_BOOL_FIELD(root, out, reveal_full_sea_chart)
    SET_INT_FIELD(root, out, num_starting_triforce_shards)
    SET_BOOL_FIELD(root, out, add_shortcut_warps_between_dungeons)
    SET_BOOL_FIELD(root, out, skip_rematch_bosses)
    SET_BOOL_FIELD(root, out, invert_sea_compass_x_axis)
    SET_INT_FIELD(root, out, num_required_dungeons)
    if(!root["damage_multiplier"]) {
      Utility::platformLog("\"damage_multiplier\" not found in config.yaml\n");
      if (!ignoreErrors) return ConfigError::MISSING_KEY;
    }
    out.settings.damage_multiplier = root["damage_multiplier"].as<float>();
    SET_BOOL_FIELD(root, out, chest_type_matches_contents)

    SET_BOOL_FIELD(root, out, player_in_casual_clothes)

    SET_INT_FIELD(root, out, starting_pohs)
    SET_INT_FIELD(root, out, starting_hcs)
    SET_INT_FIELD(root, out, starting_joy_pendants)
    SET_INT_FIELD(root, out, starting_skull_necklaces)
    SET_INT_FIELD(root, out, starting_boko_baba_seeds)
    SET_INT_FIELD(root, out, starting_golden_feathers)
    SET_INT_FIELD(root, out, starting_knights_crests)
    SET_INT_FIELD(root, out, starting_red_chu_jellys)
    SET_INT_FIELD(root, out, starting_green_chu_jellys)
    SET_INT_FIELD(root, out, starting_blue_chu_jellys)
    SET_BOOL_FIELD(root, out, remove_music)

    SET_BOOL_FIELD(root, out, do_not_generate_spoiler_log)
    SET_BOOL_FIELD(root, out, start_with_random_item)
    SET_BOOL_FIELD(root, out, plandomizer)

    if(!root["sword_mode"]) {
      if (!ignoreErrors) return ConfigError::MISSING_KEY;
    } else {
      out.settings.sword_mode = nameToSwordMode(root["sword_mode"].as<std::string>());
      if (out.settings.sword_mode == SwordMode::INVALID) {
        if (!ignoreErrors) return ConfigError::INVALID_VALUE;
        else out.settings.sword_mode = SwordMode::StartWithSword;
      }
    }

    if(!root["pig_color"])  {
      if (!ignoreErrors) return ConfigError::MISSING_KEY;
    } else {
      out.settings.pig_color = nameToPigColor(root["pig_color"].as<std::string>());
      if(out.settings.pig_color == PigColor::INVALID) {
        if (!ignoreErrors) return ConfigError::INVALID_VALUE;
        else out.settings.pig_color = PigColor::RANDOM;
      }
    }

    if(!root["dungeon_small_keys"]) {
      if (!ignoreErrors) return ConfigError::MISSING_KEY;
    } else {
      out.settings.dungeon_small_keys = nameToPlacementOption(root["dungeon_small_keys"].as<std::string>());
      if (out.settings.dungeon_small_keys == PlacementOption::INVALID) {
        if (!ignoreErrors) return ConfigError::INVALID_VALUE;
        else out.settings.dungeon_small_keys = PlacementOption::Vanilla;
      }
    }

    if(!root["dungeon_big_keys"]) {
      if (!ignoreErrors) return ConfigError::MISSING_KEY;
    } else {
      out.settings.dungeon_big_keys = nameToPlacementOption(root["dungeon_big_keys"].as<std::string>());
      if (out.settings.dungeon_big_keys == PlacementOption::INVALID) {
        if (!ignoreErrors) return ConfigError::INVALID_VALUE;
        else out.settings.dungeon_big_keys = PlacementOption::Vanilla;
      }
    }

    if(!root["dungeon_maps_compasses"]) {
      if (!ignoreErrors) return ConfigError::MISSING_KEY;
    } else {
      out.settings.dungeon_maps_compasses = nameToPlacementOption(root["dungeon_maps_compasses"].as<std::string>());
      if (out.settings.dungeon_maps_compasses == PlacementOption::INVALID) {
        if (!ignoreErrors) return ConfigError::INVALID_VALUE;
        else out.settings.dungeon_maps_compasses = PlacementOption::Vanilla;
      }
    }

    if(!root["target_type"]) {
      if (!ignoreErrors) return ConfigError::MISSING_KEY;
    } else {
      out.settings.target_type = nameToTargetTypePreference(root["target_type"].as<std::string>());
      if (out.settings.target_type == TargetTypePreference::INVALID) {
        if (!ignoreErrors) return ConfigError::INVALID_VALUE;
        else out.settings.target_type = TargetTypePreference::Hold;
      }
    }

    if(!root["camera"]) {
      if (!ignoreErrors) return ConfigError::MISSING_KEY;
    } else {
      out.settings.camera = nameToCameraPreference(root["camera"].as<std::string>());
      if (out.settings.camera == CameraPreference::INVALID) {
        if (!ignoreErrors) return ConfigError::INVALID_VALUE;
        else out.settings.camera = CameraPreference::Standard;
      }
    }

    if(!root["first_person_camera"]) {
      if (!ignoreErrors) return ConfigError::MISSING_KEY;
    } else {
      out.settings.first_person_camera = nameToFirstPersonCameraPreference(root["first_person_camera"].as<std::string>());
      if (out.settings.first_person_camera == FirstPersonCameraPreference::INVALID) {
        if (!ignoreErrors) return ConfigError::INVALID_VALUE;
        else out.settings.first_person_camera = FirstPersonCameraPreference::Standard;
      }
    }

    if(!root["gyroscope"]) {
      if (!ignoreErrors) return ConfigError::MISSING_KEY;
    } else {
      out.settings.gyroscope = nameToGyroscopePreference(root["gyroscope"].as<std::string>());
      if (out.settings.gyroscope == GyroscopePreference::INVALID) {
        if (!ignoreErrors) return ConfigError::INVALID_VALUE;
        else out.settings.gyroscope = GyroscopePreference::On;
      }
    }

    if(!root["ui_display"]) {
      if (!ignoreErrors) return ConfigError::MISSING_KEY;
    } else {
      out.settings.ui_display = nameToUIDisplayPreference(root["ui_display"].as<std::string>());
      if (out.settings.ui_display == UIDisplayPreference::INVALID) {
        if (!ignoreErrors) return ConfigError::INVALID_VALUE;
        else out.settings.ui_display = UIDisplayPreference::On;
      }
    }

    if(!root["starting_gear"] && !ignoreErrors) return ConfigError::MISSING_KEY;
    if(!root["starting_gear"].IsSequence()) {
      if (!ignoreErrors && root["starting_gear"].as<std::string>() != "None") return ConfigError::INVALID_VALUE;
    } else {
      std::unordered_multiset<GameItem> valid_items = getSupportedStartingItems();

      // Erase swords if Swordless mode is on, or remove one sword if we're starting with one
      if(out.settings.sword_mode == SwordMode::NoSword) valid_items.erase(GameItem::ProgressiveSword);
      else if (out.settings.sword_mode == SwordMode::StartWithSword) valid_items.erase(valid_items.find(GameItem::ProgressiveSword));

      out.settings.starting_gear.clear();
      for (const auto& itemObject : root["starting_gear"]) {
        const std::string itemName = itemObject.as<std::string>();
        const GameItem item = nameToGameItem(itemName);

        if (valid_items.count(item) == 0) {
            ErrorLog::getInstance().log(itemName + " cannot be added to starting inventory");
            return ConfigError::INVALID_VALUE;
        }
        out.settings.starting_gear.push_back(item);
        valid_items.erase(valid_items.find(item)); //remove the item from the set to catch duplicates or too many progressive items
      }
    }

    if (!root["custom_colors"] && !ignoreErrors) return ConfigError::MISSING_KEY;
    if (root["custom_colors"].IsMap()) {
      for (const auto& colorObject : root["custom_colors"]) {
        
        auto texture = colorObject.first.as<std::string>();
        auto color = colorObject.second.as<std::string>();

        // Only accept the color if it's valid
        if (isValidHexColor(color)) {
          out.settings.custom_colors[texture] = color;
        } else if (!ignoreErrors) {
          Utility::platformLog(color + " is not a valid hex color");
          return ConfigError::INVALID_VALUE;
        }

      }
    } else {
      return ConfigError::INVALID_VALUE;
    }

    // Add in defaults for any missing colors
    auto& colors = out.settings.player_in_casual_clothes ? DefaultColors::casualColors : DefaultColors::heroColors;
    for (auto& [name, color] : colors) {
      if (!out.settings.custom_colors.contains(name)) {
        out.settings.custom_colors[name] = color;
      }
    }

    // Clamp starting spoils
    out.settings.starting_joy_pendants = std::clamp(out.settings.starting_joy_pendants, uint16_t(0), uint16_t(MAXIMUM_STARTING_JOY_PENDANTS));
    out.settings.starting_skull_necklaces = std::clamp(out.settings.starting_skull_necklaces, uint16_t(0), uint16_t(MAXIMUM_STARTING_SKULL_NECKLACES));
    out.settings.starting_boko_baba_seeds = std::clamp(out.settings.starting_boko_baba_seeds, uint16_t(0), uint16_t(MAXIMUM_STARTING_BOKO_BABA_SEEDS));
    out.settings.starting_golden_feathers = std::clamp(out.settings.starting_golden_feathers, uint16_t(0), uint16_t(MAXIMUM_STARTING_GOLDEN_FEATHERS));
    out.settings.starting_knights_crests = std::clamp(out.settings.starting_knights_crests, uint16_t(0), uint16_t(MAXIMUM_STARTING_KNIGHTS_CRESTS));
    out.settings.starting_red_chu_jellys = std::clamp(out.settings.starting_red_chu_jellys, uint16_t(0), uint16_t(MAXIMUM_STARTING_RED_CHU_JELLYS));
    out.settings.starting_green_chu_jellys = std::clamp(out.settings.starting_green_chu_jellys, uint16_t(0), uint16_t(MAXIMUM_STARTING_GREEN_CHU_JELLYS));
    out.settings.starting_blue_chu_jellys = std::clamp(out.settings.starting_blue_chu_jellys, uint16_t(0), uint16_t(MAXIMUM_STARTING_BLUE_CHU_JELLYS));

    //can still parse file with different rando versions, but will give different item placements
    //return error after parsing so it can warn the user
    if(std::string(RANDOMIZER_VERSION) != rando_version && !ignoreErrors) return ConfigError::DIFFERENT_RANDO_VERSION;
    return ConfigError::NONE;
}



#define WRITE_CONFIG_FIELD(yaml, config, name) {   \
        yaml[#name] = config.name;                 \
    }

#define WRITE_CONFIG_BOOL_FIELD(yaml, config, name) {   \
        yaml[#name] = config.name ? "true" : "false";   \
    }


#define WRITE_SETTING_BOOL_FIELD(yaml, config, name) {           \
        yaml[#name] = config.settings.name ? "true" : "false";   \
    }


#define WRITE_NUM_FIELD(yaml, config, name) {                 \
        yaml[#name] = std::to_string(config.settings.name);   \
    }

#define WRITE_STR_FIELD(yaml, config, name) {          \
        yaml[#name] = config.settings.name;            \
    }

ConfigError writeToFile(const std::string& filePath, const Config& config) {
    YAML::Node root;

    root["program_version"] = RANDOMIZER_VERSION; //Keep track of rando version to give warning (different versions will have different item placements)
    root["file_version"] = CONFIG_VERSION; //Keep track of file version so it can avoid incompatible ones

    root["gameBaseDir"] = config.gameBaseDir.string();
    root["outputDir"] = config.outputDir.string();
    WRITE_CONFIG_FIELD(root, config, seed)
    WRITE_CONFIG_BOOL_FIELD(root, config, repack_for_console)
    WRITE_CONFIG_FIELD(root, config, consoleOutputDir)

    root["progression_dungeons"] = ProgressionDungeonsToName(config.settings.progression_dungeons);
    WRITE_SETTING_BOOL_FIELD(root, config, progression_great_fairies)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_puzzle_secret_caves)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_combat_secret_caves)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_short_sidequests)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_long_sidequests)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_spoils_trading)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_minigames)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_free_gifts)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_mail)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_platforms_rafts)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_submarines)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_eye_reef_chests)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_big_octos_gunboats)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_triforce_charts)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_treasure_charts)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_expensive_purchases)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_misc)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_tingle_chests)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_battlesquid)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_savage_labyrinth)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_island_puzzles)
    WRITE_SETTING_BOOL_FIELD(root, config, progression_obscure)

    WRITE_SETTING_BOOL_FIELD(root, config, randomize_charts)
    WRITE_SETTING_BOOL_FIELD(root, config, randomize_starting_island)
    WRITE_SETTING_BOOL_FIELD(root, config, randomize_dungeon_entrances)
    WRITE_SETTING_BOOL_FIELD(root, config, randomize_boss_entrances)
    WRITE_SETTING_BOOL_FIELD(root, config, randomize_cave_entrances)
    WRITE_SETTING_BOOL_FIELD(root, config, randomize_door_entrances)
    WRITE_SETTING_BOOL_FIELD(root, config, randomize_misc_entrances)
    WRITE_SETTING_BOOL_FIELD(root, config, mix_dungeons)
    WRITE_SETTING_BOOL_FIELD(root, config, mix_bosses)
    WRITE_SETTING_BOOL_FIELD(root, config, mix_caves)
    WRITE_SETTING_BOOL_FIELD(root, config, mix_doors)
    WRITE_SETTING_BOOL_FIELD(root, config, mix_misc)
    WRITE_SETTING_BOOL_FIELD(root, config, decouple_entrances)

    WRITE_SETTING_BOOL_FIELD(root, config, ho_ho_hints)
    WRITE_SETTING_BOOL_FIELD(root, config, korl_hints)
    WRITE_SETTING_BOOL_FIELD(root, config, clearer_hints)
    WRITE_SETTING_BOOL_FIELD(root, config, use_always_hints)
    WRITE_NUM_FIELD(root, config, path_hints)
    WRITE_NUM_FIELD(root, config, barren_hints)
    WRITE_NUM_FIELD(root, config, item_hints)
    WRITE_NUM_FIELD(root, config, location_hints)

    WRITE_SETTING_BOOL_FIELD(root, config, instant_text_boxes)
    WRITE_SETTING_BOOL_FIELD(root, config, fix_rng)
    WRITE_SETTING_BOOL_FIELD(root, config, reveal_full_sea_chart)
    WRITE_NUM_FIELD(root, config, num_starting_triforce_shards)
    WRITE_SETTING_BOOL_FIELD(root, config, add_shortcut_warps_between_dungeons)
    WRITE_SETTING_BOOL_FIELD(root, config, skip_rematch_bosses)
    WRITE_SETTING_BOOL_FIELD(root, config, invert_sea_compass_x_axis)
    WRITE_NUM_FIELD(root, config, num_required_dungeons)
    WRITE_NUM_FIELD(root, config, damage_multiplier)
    WRITE_SETTING_BOOL_FIELD(root, config, chest_type_matches_contents)

    WRITE_SETTING_BOOL_FIELD(root, config, player_in_casual_clothes)

    WRITE_NUM_FIELD(root, config, starting_pohs)
    WRITE_NUM_FIELD(root, config, starting_hcs)
    WRITE_NUM_FIELD(root, config, starting_joy_pendants)
    WRITE_NUM_FIELD(root, config, starting_skull_necklaces)
    WRITE_NUM_FIELD(root, config, starting_boko_baba_seeds)
    WRITE_NUM_FIELD(root, config, starting_golden_feathers)
    WRITE_NUM_FIELD(root, config, starting_knights_crests)
    WRITE_NUM_FIELD(root, config, starting_red_chu_jellys)
    WRITE_NUM_FIELD(root, config, starting_green_chu_jellys)
    WRITE_NUM_FIELD(root, config, starting_blue_chu_jellys)
    WRITE_SETTING_BOOL_FIELD(root, config, remove_music)

    WRITE_SETTING_BOOL_FIELD(root, config, do_not_generate_spoiler_log)
    WRITE_SETTING_BOOL_FIELD(root, config, start_with_random_item)
    WRITE_SETTING_BOOL_FIELD(root, config, plandomizer)
    WRITE_STR_FIELD(root, config, plandomizerFile)

    root["sword_mode"] = SwordModeToName(config.settings.sword_mode);
    root["pig_color"] = PigColorToName(config.settings.pig_color);
    root["dungeon_small_keys"] = PlacementOptionToName(config.settings.dungeon_small_keys);
    root["dungeon_big_keys"] = PlacementOptionToName(config.settings.dungeon_big_keys);
    root["dungeon_maps_compasses"] = PlacementOptionToName(config.settings.dungeon_maps_compasses);
    root["target_type"] = TargetTypePreferenceToName(config.settings.target_type);
    root["camera"] = CameraPreferenceToName(config.settings.camera);
    root["first_person_camera"] = FirstPersonCameraPreferenceToName(config.settings.first_person_camera);
    root["gyroscope"] = GyroscopePreferenceToName(config.settings.gyroscope);
    root["ui_display"] = UIDisplayPreferenceToName(config.settings.ui_display);

    for (const auto& item : config.settings.starting_gear) {
      root["starting_gear"].push_back(gameItemToName(item));
    }

    if (root["starting_gear"].size() == 0) {
      root["starting_gear"] = "None";
    }

    for (const auto& [texture, color] : config.settings.custom_colors) {
      root["custom_colors"][texture] = color;
    }

    std::ofstream f(filePath);
    if (f.is_open() == false)
    {
        ErrorLog::getInstance().log("Could not open config at " + filePath);
        return ConfigError::COULD_NOT_OPEN;
    }
    f << root;

    return ConfigError::NONE;
}

std::string errorToName(ConfigError err) {
    switch (err)
    {
        case ConfigError::NONE:
            return "NONE";
        case ConfigError::COULD_NOT_OPEN:
            return "COULD_NOT_OPEN";
        case ConfigError::MISSING_KEY:
            return "MISSING_KEY";
        case ConfigError::DIFFERENT_FILE_VERSION:
            return "DIFFERENT_FILE_VERSION";
        case ConfigError::DIFFERENT_RANDO_VERSION:
            return "DIFFERENT_RANDO_VERSION";
        case ConfigError::INVALID_VALUE:
            return "INVALID_VALUE";
        case ConfigError::TOO_MANY_OF_ITEM:
            return "TOO_MANY_OF_ITEM";
        default:
            return "UNKNOWN";
    }
}
