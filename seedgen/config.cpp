#include "config.hpp"

#include <fstream>
#include <set>

#include <libs/yaml.hpp>
#include <libs/base64pp.hpp>

#include <version.hpp>
#include <options.hpp>
#include <keys/keys.hpp>
#include <logic/GameItem.hpp>
#include <seedgen/seed.hpp>
#include <seedgen/packed_bits.hpp>
#include <utility/color.hpp>
#include <utility/platform.hpp>
#include <utility/string.hpp>
#include <command/Log.hpp>



// falls back to current value if the key is not present + errors are ignored
#define GET_FIELD(yaml, key, out)                                         \
    if(!yaml[key]) {                                                      \
        if (!ignoreErrors) {                                              \
            ErrorLog::getInstance().log("\"" key "\" not found in yaml"); \
            LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);                 \
        }                                                                 \
        else {                                                            \
            Utility::platformLog("\"" key "\" not found in yaml");        \
            converted = true;                                             \
        }                                                                 \
    }                                                                     \
    out = yaml[key].as<decltype(out)>(out);

#define GET_FIELD_NO_FAIL(yaml, key, out) out = yaml[key].as<decltype(out)>(out);

#define SET_FIELD(yaml, key, value) yaml[key] = value;

Config::Config() {
    resetDefaultSettings();
    resetDefaultPreferences(true);
}

void Config::resetDefaultSettings() {
    settings.resetDefaultSettings();

    return;
}

void Config::resetDefaultPreferences(const bool& paths) {
    settings.resetDefaultPreferences(paths);

    if(paths) {
        // paths and stuff that settings don't cover
        #ifdef DEVKITPRO
            // these are managed by the title system on console
            /* gameBaseDir = "storage_mlc01:/usr/title/00050000/10143500"; */
            /* outputDir = "storage_mlc01:/usr/title/00050000/10143599"; */
        #else
            gameBaseDir.clear();
            outputDir.clear();
        #endif
    }

    return;
}

ConfigError Config::loadFromFile(const fspath& filePath, const fspath& preferencesPath, bool ignoreErrors /*= false*/) {
    // check if we can open the file before parsing because exceptions won't work on console
    std::ifstream file(filePath);
    std::ifstream preferences(preferencesPath);
    if(!file.is_open() || !preferences.is_open()) LOG_ERR_AND_RETURN(ConfigError::COULD_NOT_OPEN);
    file.close();
    preferences.close();
    
    YAML::Node root = LoadYAML(filePath);
    YAML::Node preferencesRoot = LoadYAML(preferencesPath);

    std::string rando_version, file_version;
    GET_FIELD(root, "program_version", rando_version)
    GET_FIELD(root, "file_version", file_version)

    if(file_version != CONFIG_VERSION) {
        converted = true;

        Utility::platformLog("Attempted to load config version " + file_version + ", current version is " CONFIG_VERSION);
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::DIFFERENT_FILE_VERSION);
    }

    // hardcode paths for console, otherwise use config
    #ifdef DEVKITPRO
        // these are now loaded elsewhere
        /* out.gameBaseDir = "storage_mlc01:/usr/title/00050000/10143500"; */
        /* out.outputDir = "storage_mlc01:/usr/title/00050000/10143599"; */

        settings.plandomizerFile = Utility::get_app_save_path() / "plandomizer.yaml";
    #else
        std::string baseTemp, outTemp, plandoTemp;
        GET_FIELD_NO_FAIL(preferencesRoot, "gameBaseDir", baseTemp)
        GET_FIELD_NO_FAIL(preferencesRoot, "outputDir", outTemp)
        GET_FIELD_NO_FAIL(preferencesRoot, "plandomizerFile", plandoTemp)
        // convert these in case they have special characters, prevents special character issues on Windows
        gameBaseDir = Utility::Str::toUTF16(baseTemp);
        outputDir = Utility::Str::toUTF16(outTemp);
        settings.plandomizerFile = Utility::Str::toUTF16(plandoTemp);
    #endif

    if(!root["game_version"]) {
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        settings.game_version = nameToGameVersion(root["game_version"].as<std::string>("INVALID"));
        if (settings.game_version == GameVersion::INVALID) {
            if(!ignoreErrors) {
                LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }
            else {
                settings.game_version = GameVersion::HD;
            }
        }
    }

    GET_FIELD_NO_FAIL(root, "seed", seed)

    if(!root["progression_dungeons"]) {
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        settings.progression_dungeons = nameToProgressionDungeons(root["progression_dungeons"].as<std::string>("INVALID"));
        if (settings.progression_dungeons == ProgressionDungeons::INVALID) {
            if(!ignoreErrors) {
                LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }
            else {
                settings.progression_dungeons = ProgressionDungeons::Standard;
            }
        }
    }
    GET_FIELD(root, "progression_great_fairies", settings.progression_great_fairies)
    GET_FIELD(root, "progression_puzzle_secret_caves", settings.progression_puzzle_secret_caves)
    GET_FIELD(root, "progression_combat_secret_caves", settings.progression_combat_secret_caves)
    GET_FIELD(root, "progression_short_sidequests", settings.progression_short_sidequests)
    GET_FIELD(root, "progression_long_sidequests", settings.progression_long_sidequests)
    GET_FIELD(root, "progression_spoils_trading", settings.progression_spoils_trading)
    GET_FIELD(root, "progression_minigames", settings.progression_minigames)
    GET_FIELD(root, "progression_free_gifts", settings.progression_free_gifts)
    GET_FIELD(root, "progression_mail", settings.progression_mail)
    GET_FIELD(root, "progression_platforms_rafts", settings.progression_platforms_rafts)
    GET_FIELD(root, "progression_submarines", settings.progression_submarines)
    GET_FIELD(root, "progression_eye_reef_chests", settings.progression_eye_reef_chests)
    GET_FIELD(root, "progression_big_octos_gunboats", settings.progression_big_octos_gunboats)
    GET_FIELD(root, "progression_triforce_charts", settings.progression_triforce_charts)
    GET_FIELD(root, "progression_treasure_charts", settings.progression_treasure_charts)
    GET_FIELD(root, "progression_expensive_purchases", settings.progression_expensive_purchases)
    GET_FIELD(root, "progression_misc", settings.progression_misc)
    GET_FIELD(root, "progression_tingle_chests", settings.progression_tingle_chests)
    GET_FIELD(root, "progression_battlesquid", settings.progression_battlesquid)
    GET_FIELD(root, "progression_savage_labyrinth", settings.progression_savage_labyrinth)
    GET_FIELD(root, "progression_island_puzzles", settings.progression_island_puzzles)
    GET_FIELD(root, "progression_dungeon_secrets", settings.progression_dungeon_secrets)
    GET_FIELD(root, "progression_obscure", settings.progression_obscure)

    GET_FIELD(root, "randomize_charts", settings.randomize_charts)
    GET_FIELD(root, "randomize_starting_island", settings.randomize_starting_island)
    GET_FIELD(root, "randomize_dungeon_entrances", settings.randomize_dungeon_entrances)
    GET_FIELD(root, "randomize_boss_entrances", settings.randomize_boss_entrances)
    GET_FIELD(root, "randomize_miniboss_entrances", settings.randomize_miniboss_entrances)
    if(!root["randomize_cave_entrances"]) {
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        settings.randomize_cave_entrances = nameToShuffleCaveEntrances(root["randomize_cave_entrances"].as<std::string>("INVALID"));
        if (settings.randomize_cave_entrances == ShuffleCaveEntrances::INVALID) {
            if(!ignoreErrors) {
                LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }
            else {
                settings.randomize_cave_entrances = ShuffleCaveEntrances::Disabled;
            }
        }
    }
    GET_FIELD(root, "randomize_door_entrances", settings.randomize_door_entrances)
    GET_FIELD(root, "randomize_misc_entrances", settings.randomize_misc_entrances)
    GET_FIELD(root, "mix_dungeons", settings.mix_dungeons)
    GET_FIELD(root, "mix_bosses", settings.mix_bosses)
    GET_FIELD(root, "mix_minibosses", settings.mix_minibosses)
    GET_FIELD(root, "mix_caves", settings.mix_caves)
    GET_FIELD(root, "mix_doors", settings.mix_doors)
    GET_FIELD(root, "mix_misc", settings.mix_misc)
    GET_FIELD(root, "decouple_entrances", settings.decouple_entrances)

    GET_FIELD(root, "ho_ho_hints", settings.ho_ho_hints)
    GET_FIELD(root, "korl_hints", settings.korl_hints)
    GET_FIELD(root, "clearer_hints", settings.clearer_hints)
    GET_FIELD(root, "use_always_hints", settings.use_always_hints)
    GET_FIELD(root, "hint_importance", settings.hint_importance)
    GET_FIELD(root, "path_hints", settings.path_hints)
    GET_FIELD(root, "barren_hints", settings.barren_hints)
    GET_FIELD(root, "item_hints", settings.item_hints)
    GET_FIELD(root, "location_hints", settings.location_hints)

    GET_FIELD(root, "instant_text_boxes", settings.instant_text_boxes)
    GET_FIELD(root, "fix_rng", settings.fix_rng)
    GET_FIELD(root, "performance", settings.performance)
    GET_FIELD(root, "reveal_full_sea_chart", settings.reveal_full_sea_chart)
    GET_FIELD(root, "add_shortcut_warps_between_dungeons", settings.add_shortcut_warps_between_dungeons)
    GET_FIELD(root, "skip_rematch_bosses", settings.skip_rematch_bosses)
    GET_FIELD(root, "invert_sea_compass_x_axis", settings.invert_sea_compass_x_axis)
    GET_FIELD(root, "num_required_dungeons", settings.num_required_dungeons)
    GET_FIELD(root, "damage_multiplier", settings.damage_multiplier);
    GET_FIELD(root, "chest_type_matches_contents", settings.chest_type_matches_contents)
    GET_FIELD(root, "remove_swords", settings.remove_swords)

    GET_FIELD(root, "starting_pohs", settings.starting_pohs)
    GET_FIELD(root, "starting_hcs", settings.starting_hcs)
    GET_FIELD(root, "starting_joy_pendants", settings.starting_joy_pendants)
    GET_FIELD(root, "starting_skull_necklaces", settings.starting_skull_necklaces)
    GET_FIELD(root, "starting_boko_baba_seeds", settings.starting_boko_baba_seeds)
    GET_FIELD(root, "starting_golden_feathers", settings.starting_golden_feathers)
    GET_FIELD(root, "starting_knights_crests", settings.starting_knights_crests)
    GET_FIELD(root, "starting_red_chu_jellys", settings.starting_red_chu_jellys)
    GET_FIELD(root, "starting_green_chu_jellys", settings.starting_green_chu_jellys)
    GET_FIELD(root, "starting_blue_chu_jellys", settings.starting_blue_chu_jellys)
    GET_FIELD(root, "remove_music", settings.remove_music)

    GET_FIELD(root, "do_not_generate_spoiler_log", settings.do_not_generate_spoiler_log)
    GET_FIELD(root, "start_with_random_item", settings.start_with_random_item)
    GET_FIELD(root, "random_item_slide_item", settings.random_item_slide_item)
    GET_FIELD(root, "classic_mode", settings.classic_mode)
    GET_FIELD(root, "plandomizer", settings.plandomizer)

    if(!preferencesRoot["pig_color"])  {
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        settings.pig_color = nameToPigColor(preferencesRoot["pig_color"].as<std::string>("INVALID"));
        if(settings.pig_color == PigColor::INVALID) {
            if (!ignoreErrors) {
                LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }
            else {
                settings.pig_color = PigColor::Random;
            }
        }
    }

    if(!root["dungeon_small_keys"]) {
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        settings.dungeon_small_keys = nameToPlacementOption(root["dungeon_small_keys"].as<std::string>("INVALID"));
        if(settings.dungeon_small_keys == PlacementOption::INVALID) {
            if (!ignoreErrors) {
                LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }
            else {
                settings.dungeon_small_keys = PlacementOption::Vanilla;
            }
        }
    }

    if(!root["dungeon_big_keys"]) {
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        settings.dungeon_big_keys = nameToPlacementOption(root["dungeon_big_keys"].as<std::string>("INVALID"));
        if(settings.dungeon_big_keys == PlacementOption::INVALID) {
            if (!ignoreErrors) {
                LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }
            else {
                settings.dungeon_big_keys = PlacementOption::Vanilla;
            }
        }
    }

    if(!root["dungeon_maps_compasses"]) {
        if (!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        settings.dungeon_maps_compasses = nameToPlacementOption(root["dungeon_maps_compasses"].as<std::string>("INVALID"));
        if(settings.dungeon_maps_compasses == PlacementOption::INVALID) {
            if(!ignoreErrors) {
                LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }
            else {
                settings.dungeon_maps_compasses = PlacementOption::Vanilla;
            }
          }
    }

    if(!preferencesRoot["target_type"]) {
        if (!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        settings.target_type = nameToTargetTypePreference(preferencesRoot["target_type"].as<std::string>("INVALID"));
        if(settings.target_type == TargetTypePreference::INVALID) {
            if(!ignoreErrors) {
                LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }
            else {
                settings.target_type = TargetTypePreference::Hold;
            }
          }
    }

    if(!preferencesRoot["camera"]) {
        if (!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        settings.camera = nameToCameraPreference(preferencesRoot["camera"].as<std::string>("INVALID"));
        if(settings.camera == CameraPreference::INVALID) {
            if(!ignoreErrors) {
                LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }
            else {
                settings.camera = CameraPreference::Standard;
            }
          }
    }

    if(!preferencesRoot["first_person_camera"]) {
        if (!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        settings.first_person_camera = nameToFirstPersonCameraPreference(preferencesRoot["first_person_camera"].as<std::string>("INVALID"));
        if(settings.first_person_camera == FirstPersonCameraPreference::INVALID) {
            if(!ignoreErrors) {
                LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }
            else {
                settings.first_person_camera = FirstPersonCameraPreference::Standard;
            }
          }
    }

    if(!preferencesRoot["gyroscope"]) {
        if (!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        settings.gyroscope = nameToGyroscopePreference(preferencesRoot["gyroscope"].as<std::string>("INVALID"));
        if(settings.gyroscope == GyroscopePreference::INVALID) {
            if(!ignoreErrors) {
                LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }
            else {
                settings.gyroscope = GyroscopePreference::On;
            }
          }
    }

    if(!preferencesRoot["ui_display"]) {
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        settings.ui_display = nameToUIDisplayPreference(preferencesRoot["ui_display"].as<std::string>("INVALID"));
        if(settings.ui_display == UIDisplayPreference::INVALID) {
            if(!ignoreErrors) {
                LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }
            else {
                settings.ui_display = UIDisplayPreference::On;
            }
          }
    }

    if(!root["starting_gear"] || (!root["starting_gear"].IsSequence() && root["starting_gear"].as<std::string>() != "None")) {
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        std::unordered_multiset<GameItem> valid_items = getSupportedStartingItems();

        // erase swords if they are not being placed
        if(settings.remove_swords) {
            valid_items.erase(GameItem::ProgressiveSword);
            valid_items.erase(GameItem::HurricaneSpin);
        }

        settings.starting_gear.clear();
        for(const auto& itemObject : root["starting_gear"]) {
            const std::string itemName = itemObject.as<std::string>();
            const GameItem item = nameToGameItem(itemName);

            if(!valid_items.contains(item)) {
                ErrorLog::getInstance().log(itemName + " cannot be added to starting inventory");
                LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }

            settings.starting_gear.push_back(item);
            valid_items.erase(valid_items.find(item)); // remove the item from the set to catch duplicates or too many progressive items
        }
    }

    if(!root["excluded_locations"] || (!root["excluded_locations"].IsSequence() && root["excluded_locations"].as<std::string>() != "None")) {
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MISSING_KEY);
    }
    else {
        settings.excluded_locations.clear();
        for (const auto& locObject : root["excluded_locations"]) {
            const auto locName = locObject.as<std::string>();
            settings.excluded_locations.insert(locName);
        }
    }

    GET_FIELD(preferencesRoot, "quiet_swift_sail", settings.quiet_swift_sail)

    GET_FIELD(preferencesRoot, "custom_player_model", settings.selectedModel.modelName)
    if(settings.selectedModel.loadFromFolder() != ModelError::NONE) {
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::MODEL_ERROR);
    }
    GET_FIELD(preferencesRoot, "player_in_casual_clothes", settings.selectedModel.casual)
    // only non-default colors are written
    if(preferencesRoot["custom_colors"].IsMap()) {
        for(const auto& colorObject : preferencesRoot["custom_colors"]) {
            const std::string texture = colorObject.first.as<std::string>();
            const std::string color = colorObject.second.as<std::string>();

            // only accept the color if it's valid
            if(!isValidHexColor(color)) {
                Utility::platformLog(color + " is not a valid hex color");
                if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::INVALID_VALUE);
            }
            else {
                settings.selectedModel.setColor(texture, color);
            }
        }
    }

    // clamp numerical settings
    // health
    settings.starting_hcs = std::clamp<uint16_t>(settings.starting_hcs, 1, MAXIMUM_STARTING_HC);
    settings.starting_pohs = std::clamp<uint16_t>(settings.starting_pohs, 0, MAXIMUM_STARTING_HP);
    settings.damage_multiplier = std::clamp<float>(settings.damage_multiplier, 0, MAXIMUM_DAMAGE_MULTIPLIER);

    // spoils
    settings.starting_joy_pendants = std::clamp<uint16_t>(settings.starting_joy_pendants, 0, MAXIMUM_STARTING_JOY_PENDANTS);
    settings.starting_joy_pendants = std::clamp<uint16_t>(settings.starting_joy_pendants, 0, MAXIMUM_STARTING_JOY_PENDANTS);
    settings.starting_skull_necklaces = std::clamp<uint16_t>(settings.starting_skull_necklaces, 0, MAXIMUM_STARTING_SKULL_NECKLACES);
    settings.starting_boko_baba_seeds = std::clamp<uint16_t>(settings.starting_boko_baba_seeds, 0, MAXIMUM_STARTING_BOKO_BABA_SEEDS);
    settings.starting_golden_feathers = std::clamp<uint16_t>(settings.starting_golden_feathers, 0, MAXIMUM_STARTING_GOLDEN_FEATHERS);
    settings.starting_knights_crests = std::clamp<uint16_t>(settings.starting_knights_crests, 0, MAXIMUM_STARTING_KNIGHTS_CRESTS);
    settings.starting_red_chu_jellys = std::clamp<uint16_t>(settings.starting_red_chu_jellys, 0, MAXIMUM_STARTING_RED_CHU_JELLYS);
    settings.starting_green_chu_jellys = std::clamp<uint16_t>(settings.starting_green_chu_jellys, 0, MAXIMUM_STARTING_GREEN_CHU_JELLYS);
    settings.starting_blue_chu_jellys = std::clamp<uint16_t>(settings.starting_blue_chu_jellys, 0, MAXIMUM_STARTING_BLUE_CHU_JELLYS);

    // hints
    settings.path_hints = std::clamp<uint8_t>(settings.path_hints, 0, MAXIMUM_PATH_HINT_COUNT);
    settings.barren_hints = std::clamp<uint8_t>(settings.barren_hints, 0, MAXIMUM_BARREN_HINT_COUNT);
    settings.item_hints = std::clamp<uint8_t>(settings.item_hints, 0, MAXIMUM_ITEM_HINT_COUNT);
    settings.location_hints = std::clamp<uint8_t>(settings.location_hints, 0, MAXIMUM_LOCATION_HINT_COUNT);
    
    settings.num_required_dungeons = std::clamp<uint8_t>(settings.num_required_dungeons, 0, MAXIMUM_NUM_DUNGEONS);
    
    configSet = true;

    // can still use a file from a different rando version but it will give different item placements
    // handle this at the end so it can warn the user but everything still loads
    if(rando_version != RANDOMIZER_VERSION) {
        updated = true;

        Utility::platformLog("Warning: config was made using a different randomizer version");
        Utility::platformLog("Item placement may be different than expected");
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::DIFFERENT_RANDO_VERSION);
    }

    return ConfigError::NONE;
}

YAML::Node Config::settingsToYaml() const {
    YAML::Node root;

    SET_FIELD(root, "program_version", RANDOMIZER_VERSION) //Keep track of rando version to give warning (different versions will have different item placements)
    SET_FIELD(root, "file_version", CONFIG_VERSION) //Keep track of file version so it can avoid incompatible ones

    SET_FIELD(root, "game_version", GameVersionToName(settings.game_version))

    SET_FIELD(root, "seed", seed)

    SET_FIELD(root, "progression_dungeons", ProgressionDungeonsToName(settings.progression_dungeons))
    SET_FIELD(root, "progression_great_fairies", settings.progression_great_fairies)
    SET_FIELD(root, "progression_puzzle_secret_caves", settings.progression_puzzle_secret_caves)
    SET_FIELD(root, "progression_combat_secret_caves", settings.progression_combat_secret_caves)
    SET_FIELD(root, "progression_short_sidequests", settings.progression_short_sidequests)
    SET_FIELD(root, "progression_long_sidequests", settings.progression_long_sidequests)
    SET_FIELD(root, "progression_spoils_trading", settings.progression_spoils_trading)
    SET_FIELD(root, "progression_minigames", settings.progression_minigames)
    SET_FIELD(root, "progression_free_gifts", settings.progression_free_gifts)
    SET_FIELD(root, "progression_mail", settings.progression_mail)
    SET_FIELD(root, "progression_platforms_rafts", settings.progression_platforms_rafts)
    SET_FIELD(root, "progression_submarines", settings.progression_submarines)
    SET_FIELD(root, "progression_eye_reef_chests", settings.progression_eye_reef_chests)
    SET_FIELD(root, "progression_big_octos_gunboats", settings.progression_big_octos_gunboats)
    SET_FIELD(root, "progression_triforce_charts", settings.progression_triforce_charts)
    SET_FIELD(root, "progression_treasure_charts", settings.progression_treasure_charts)
    SET_FIELD(root, "progression_expensive_purchases", settings.progression_expensive_purchases)
    SET_FIELD(root, "progression_misc", settings.progression_misc)
    SET_FIELD(root, "progression_tingle_chests", settings.progression_tingle_chests)
    SET_FIELD(root, "progression_battlesquid", settings.progression_battlesquid)
    SET_FIELD(root, "progression_savage_labyrinth", settings.progression_savage_labyrinth)
    SET_FIELD(root, "progression_island_puzzles", settings.progression_island_puzzles)
    SET_FIELD(root, "progression_dungeon_secrets", settings.progression_dungeon_secrets)
    SET_FIELD(root, "progression_obscure", settings.progression_obscure)

    SET_FIELD(root, "randomize_charts", settings.randomize_charts)
    SET_FIELD(root, "randomize_starting_island", settings.randomize_starting_island)
    SET_FIELD(root, "randomize_dungeon_entrances", settings.randomize_dungeon_entrances)
    SET_FIELD(root, "randomize_boss_entrances", settings.randomize_boss_entrances)
    SET_FIELD(root, "randomize_miniboss_entrances", settings.randomize_miniboss_entrances)
    SET_FIELD(root, "randomize_cave_entrances", ShuffleCaveEntrancesToName(settings.randomize_cave_entrances))
    SET_FIELD(root, "randomize_door_entrances", settings.randomize_door_entrances)
    SET_FIELD(root, "randomize_misc_entrances", settings.randomize_misc_entrances)
    SET_FIELD(root, "mix_dungeons", settings.mix_dungeons)
    SET_FIELD(root, "mix_bosses", settings.mix_bosses)
    SET_FIELD(root, "mix_minibosses", settings.mix_minibosses)
    SET_FIELD(root, "mix_caves", settings.mix_caves)
    SET_FIELD(root, "mix_doors", settings.mix_doors)
    SET_FIELD(root, "mix_misc", settings.mix_misc)
    SET_FIELD(root, "decouple_entrances", settings.decouple_entrances)

    SET_FIELD(root, "ho_ho_hints", settings.ho_ho_hints)
    SET_FIELD(root, "korl_hints", settings.korl_hints)
    SET_FIELD(root, "clearer_hints", settings.clearer_hints)
    SET_FIELD(root, "use_always_hints", settings.use_always_hints)
    SET_FIELD(root, "hint_importance", settings.hint_importance)
    SET_FIELD(root, "path_hints", static_cast<uint32_t>(settings.path_hints)) // cast because uint8_t has a weird yaml output
    SET_FIELD(root, "barren_hints", static_cast<uint32_t>(settings.barren_hints)) // cast because uint8_t has a weird yaml output
    SET_FIELD(root, "item_hints", static_cast<uint32_t>(settings.item_hints)) // cast because uint8_t has a weird yaml output
    SET_FIELD(root, "location_hints", static_cast<uint32_t>(settings.location_hints)) // cast because uint8_t has a weird yaml output

    SET_FIELD(root, "instant_text_boxes", settings.instant_text_boxes)
    SET_FIELD(root, "fix_rng", settings.fix_rng)
    SET_FIELD(root, "performance", settings.performance)
    SET_FIELD(root, "reveal_full_sea_chart", settings.reveal_full_sea_chart)
    SET_FIELD(root, "add_shortcut_warps_between_dungeons", settings.add_shortcut_warps_between_dungeons)
    SET_FIELD(root, "skip_rematch_bosses", settings.skip_rematch_bosses)
    SET_FIELD(root, "invert_sea_compass_x_axis", settings.invert_sea_compass_x_axis)
    SET_FIELD(root, "num_required_dungeons", static_cast<uint32_t>(settings.num_required_dungeons)) // cast because uint8_t has a weird yaml output
    SET_FIELD(root, "damage_multiplier", settings.damage_multiplier)
    SET_FIELD(root, "chest_type_matches_contents", settings.chest_type_matches_contents)
    SET_FIELD(root, "remove_swords", settings.remove_swords)

    SET_FIELD(root, "starting_pohs", settings.starting_pohs)
    SET_FIELD(root, "starting_hcs", settings.starting_hcs)
    SET_FIELD(root, "starting_joy_pendants", settings.starting_joy_pendants)
    SET_FIELD(root, "starting_skull_necklaces", settings.starting_skull_necklaces)
    SET_FIELD(root, "starting_boko_baba_seeds", settings.starting_boko_baba_seeds)
    SET_FIELD(root, "starting_golden_feathers", settings.starting_golden_feathers)
    SET_FIELD(root, "starting_knights_crests", settings.starting_knights_crests)
    SET_FIELD(root, "starting_red_chu_jellys", settings.starting_red_chu_jellys)
    SET_FIELD(root, "starting_green_chu_jellys", settings.starting_green_chu_jellys)
    SET_FIELD(root, "starting_blue_chu_jellys", settings.starting_blue_chu_jellys)
    SET_FIELD(root, "remove_music", settings.remove_music)

    SET_FIELD(root, "do_not_generate_spoiler_log", settings.do_not_generate_spoiler_log)
    SET_FIELD(root, "start_with_random_item", settings.start_with_random_item)
    SET_FIELD(root, "random_item_slide_item", settings.random_item_slide_item)
    SET_FIELD(root, "classic_mode", settings.classic_mode)
    SET_FIELD(root, "plandomizer", settings.plandomizer)

    SET_FIELD(root, "dungeon_small_keys", PlacementOptionToName(settings.dungeon_small_keys))
    SET_FIELD(root, "dungeon_big_keys", PlacementOptionToName(settings.dungeon_big_keys))
    SET_FIELD(root, "dungeon_maps_compasses", PlacementOptionToName(settings.dungeon_maps_compasses))

    for (const auto& item : settings.starting_gear) {
        root["starting_gear"].push_back(gameItemToName(item));
    }

    if (root["starting_gear"].size() == 0) {
        root["starting_gear"] = "None";
    }

    for (const auto& locName : settings.excluded_locations) {
        root["excluded_locations"].push_back(locName);
    }

    if (root["excluded_locations"].size() == 0) {
        root["excluded_locations"] = "None";
    }

    return root;
}

YAML::Node Config::preferencesToYaml() const {
    YAML::Node preferencesRoot;
    SET_FIELD(preferencesRoot, "gameBaseDir", Utility::toUtf8String(gameBaseDir))
    SET_FIELD(preferencesRoot, "outputDir", Utility::toUtf8String(outputDir))
    SET_FIELD(preferencesRoot, "plandomizerFile", Utility::toUtf8String(settings.plandomizerFile))

    SET_FIELD(preferencesRoot, "pig_color", PigColorToName(settings.pig_color))

    SET_FIELD(preferencesRoot, "target_type", TargetTypePreferenceToName(settings.target_type))
    SET_FIELD(preferencesRoot, "camera", CameraPreferenceToName(settings.camera))
    SET_FIELD(preferencesRoot, "first_person_camera", FirstPersonCameraPreferenceToName(settings.first_person_camera))
    SET_FIELD(preferencesRoot, "gyroscope", GyroscopePreferenceToName(settings.gyroscope))
    SET_FIELD(preferencesRoot, "ui_display", UIDisplayPreferenceToName(settings.ui_display))

    SET_FIELD(preferencesRoot, "quiet_swift_sail", settings.quiet_swift_sail)

    SET_FIELD(preferencesRoot, "player_in_casual_clothes", settings.selectedModel.casual)
    SET_FIELD(preferencesRoot, "custom_player_model", settings.selectedModel.modelName)
    for (const auto& [texture, color] : settings.selectedModel.getSetColorsMap()) {
        preferencesRoot["custom_colors"][texture] = color;
    }

    return preferencesRoot;
}

ConfigError Config::writeSettings(const fspath& filePath) const {

    YAML::Node root = settingsToYaml();
    std::ofstream f(filePath);
    if (f.is_open() == false)
    {
        ErrorLog::getInstance().log("Could not open config at " + Utility::toUtf8String(filePath));
        return ConfigError::COULD_NOT_OPEN;
    }
    f << root;
    f.close();

    return ConfigError::NONE;
}

ConfigError Config::writePreferences(const fspath& preferencesPath) const {

    YAML::Node preferences = preferencesToYaml();
    std::ofstream pref(preferencesPath);
    if (pref.is_open() == false)
    {
        ErrorLog::getInstance().log("Could not open preferences at " + Utility::toUtf8String(preferencesPath));
        return ConfigError::COULD_NOT_OPEN;
    }
    pref << preferences;
    pref.close();

    return ConfigError::NONE;
}

ConfigError Config::writeToFile(const fspath& filePath, const fspath& preferencesPath) const {
    LOG_AND_RETURN_IF_ERR(writeSettings(filePath))
    LOG_AND_RETURN_IF_ERR(writePreferences(preferencesPath))
    return ConfigError::NONE;
}

static const std::vector<GameItem> REGULAR_ITEMS = {
    GameItem::BaitBag,
    GameItem::BalladOfGales,
    GameItem::Bombs,
    GameItem::Boomerang,
    GameItem::CabanaDeed,
    GameItem::CommandMelody,
    GameItem::DekuLeaf,
    GameItem::DeliveryBag,
    GameItem::DinsPearl,
    GameItem::EarthGodsLyric,
    GameItem::EmptyBottle,
    GameItem::FaroresPearl,
    GameItem::GhostShipChart,
    GameItem::GrapplingHook,
    GameItem::HerosCharm,
    GameItem::Hookshot,
    GameItem::HurricaneSpin,
    GameItem::IronBoots,
    GameItem::MaggiesLetter,
    GameItem::MagicArmor,
    GameItem::MoblinsLetter,
    GameItem::NayrusPearl,
    GameItem::NoteToMom,
    GameItem::PowerBracelets,
    GameItem::SkullHammer,
    GameItem::SongOfPassing,
    GameItem::SpoilsBag,
    GameItem::Telescope,
    GameItem::TingleBottle,
    GameItem::WindGodsAria,
    GameItem::DRCBigKey,
    GameItem::DRCCompass,
    GameItem::DRCDungeonMap,
    GameItem::FWBigKey,
    GameItem::FWCompass,
    GameItem::FWDungeonMap,
    GameItem::TotGBigKey,
    GameItem::TotGCompass,
    GameItem::TotGDungeonMap,
    GameItem::ETBigKey,
    GameItem::ETCompass,
    GameItem::ETDungeonMap,
    GameItem::WTBigKey,
    GameItem::WTCompass,
    GameItem::WTDungeonMap,
    GameItem::FFCompass,
    GameItem::FFDungeonMap,
    GameItem::DragonTingleStatue,
    GameItem::ForbiddenTingleStatue,
    GameItem::GoddessTingleStatue,
    GameItem::EarthTingleStatue,
    GameItem::WindTingleStatue,
    GameItem::TriforceShard1,
    GameItem::TriforceShard2,
    GameItem::TriforceShard3,
    GameItem::TriforceShard4,
    GameItem::TriforceShard5,
    GameItem::TriforceShard6,
    GameItem::TriforceShard7,
    GameItem::TriforceShard8,
};

static const std::vector<GameItem> PROGRESSIVE_ITEMS = {
    GameItem::ProgressiveBombBag,
    GameItem::ProgressiveBow,
    GameItem::ProgressiveMagicMeter,
    GameItem::ProgressivePictoBox,
    GameItem::ProgressiveQuiver,
    GameItem::ProgressiveShield,
    GameItem::ProgressiveSword,
    GameItem::ProgressiveSail,
    GameItem::ProgressiveWallet,
    GameItem::DRCSmallKey,
    GameItem::FWSmallKey,
    GameItem::TotGSmallKey,
    GameItem::ETSmallKey,
    GameItem::WTSmallKey,
};

// These are options that should affect seed generation even with the same seed
static const std::vector<Option> PERMALINK_OPTIONS {
    // Progression
    Option::ProgressDungeons,
    Option::ProgressGreatFairies,
    Option::ProgressPuzzleCaves,
    Option::ProgressCombatCaves,
    Option::ProgressShortSidequests,
    Option::ProgressLongSidequests,
    Option::ProgressSpoilsTrading,
    Option::ProgressMinigames,
    Option::ProgressFreeGifts,
    Option::ProgressMail,
    Option::ProgressPlatformsRafts,
    Option::ProgressSubmarines,
    Option::ProgressEyeReefs,
    Option::ProgressOctosGunboats,
    Option::ProgressTriforceCharts,
    Option::ProgressTreasureCharts,
    Option::ProgressExpPurchases,
    Option::ProgressMisc,
    Option::ProgressTingleChests,
    Option::ProgressBattlesquid,
    Option::ProgressSavageLabyrinth,
    Option::ProgressIslandPuzzles,
    Option::ProgressDungeonSecrets,
    Option::ProgressObscure,

    // Additional Randomization Options
    Option::RemoveSwords,
    Option::DungeonSmallKeys,
    Option::DungeonBigKeys,
    Option::DungeonMapsAndCompasses,
    Option::NumRequiredDungeons,
    Option::RandomCharts,
    Option::CTMC,
    Option::DamageMultiplier,

    // Convenience Tweaks
    Option::InstantText,
    Option::RevealSeaChart,
    Option::SkipRefights,
    Option::AddShortcutWarps,
    Option::RemoveMusic,

    // Starting Gear
    Option::StartingGear,
    Option::StartingHC,
    Option::StartingHP,
    Option::StartingJoyPendants,
    Option::StartingSkullNecklaces,
    Option::StartingBokoBabaSeeds,
    Option::StartingGoldenFeathers,
    Option::StartingKnightsCrests,
    Option::StartingRedChuJellys,
    Option::StartingGreenChuJellys,
    Option::StartingBlueChuJellys,

    // Excluded Locations
    Option::ExcludedLocations,

    // Advanced Options
    Option::NoSpoilerLog,
    Option::StartWithRandomItem,
    Option::RandomItemSlideItem,
    Option::ClassicMode,
    Option::Plandomizer,
    Option::FixRNG,
    Option::Performance,

    // Hints
    Option::HoHoHints,
    Option::KorlHints,
    Option::PathHints,
    Option::BarrenHints,
    Option::ItemHints,
    Option::LocationHints,
    Option::UseAlwaysHints,
    Option::ClearerHints,
    Option::HintImportance,

    // Entrance Randomizer
    Option::RandomizeDungeonEntrances,
    Option::RandomizeBossEntrances,
    Option::RandomizeMinibossEntrances,
    Option::RandomizeCaveEntrances,
    Option::RandomizeDoorEntrances,
    Option::RandomizeMiscEntrances,
    Option::MixDungeons,
    Option::MixBosses,
    Option::MixMinibosses,
    Option::MixCaves,
    Option::MixDoors,
    Option::MixMisc,
    Option::DecoupleEntrances,
    Option::RandomStartIsland,

};

static size_t getOptionBitCount(const Option& option) {
    switch(option) {
        case Option::DungeonSmallKeys:
        case Option::DungeonBigKeys:
        case Option::DungeonMapsAndCompasses:
        case Option::NumRequiredDungeons:
        case Option::DamageMultiplier:
            return 8; // ComboBox Options (and 8-bit SpinBox options)
        case Option::ProgressDungeons:
        case Option::RandomizeCaveEntrances:
        case Option::PathHints:
        case Option::BarrenHints:
        case Option::LocationHints:
        case Option::ItemHints:
        case Option::StartingHC:
            return 3; // 3-bit SpinBox options
        case Option::StartingHP:
        case Option::StartingJoyPendants:
        case Option::StartingSkullNecklaces:
        case Option::StartingBokoBabaSeeds:
        case Option::StartingGoldenFeathers:
        case Option::StartingKnightsCrests:
        case Option::StartingRedChuJellys:
        case Option::StartingGreenChuJellys:
        case Option::StartingBlueChuJellys:
            return 6; // 6-bit SpinBox options
        case Option::ProgressGreatFairies:
        case Option::ProgressPuzzleCaves:
        case Option::ProgressCombatCaves:
        case Option::ProgressShortSidequests:
        case Option::ProgressLongSidequests:
        case Option::ProgressSpoilsTrading:
        case Option::ProgressMinigames:
        case Option::ProgressFreeGifts:
        case Option::ProgressMail:
        case Option::ProgressPlatformsRafts:
        case Option::ProgressSubmarines:
        case Option::ProgressEyeReefs:
        case Option::ProgressOctosGunboats:
        case Option::ProgressTriforceCharts:
        case Option::ProgressTreasureCharts:
        case Option::ProgressExpPurchases:
        case Option::ProgressMisc:
        case Option::ProgressTingleChests:
        case Option::ProgressBattlesquid:
        case Option::ProgressSavageLabyrinth:
        case Option::ProgressIslandPuzzles:
        case Option::ProgressDungeonSecrets:
        case Option::ProgressObscure:
        case Option::RemoveSwords:
        case Option::RandomCharts:
        case Option::CTMC:
        case Option::InstantText:
        case Option::RevealSeaChart:
        case Option::SkipRefights:
        case Option::AddShortcutWarps:
        case Option::RemoveMusic:
        case Option::NoSpoilerLog:
        case Option::StartWithRandomItem:
        case Option::RandomItemSlideItem:
        case Option::ClassicMode:
        case Option::Plandomizer:
        case Option::FixRNG:
        case Option::Performance:
        case Option::HoHoHints:
        case Option::KorlHints:
        case Option::UseAlwaysHints:
        case Option::ClearerHints:
        case Option::HintImportance:
        case Option::RandomizeDungeonEntrances:
        case Option::RandomizeBossEntrances:
        case Option::RandomizeMinibossEntrances:
        case Option::RandomizeDoorEntrances:
        case Option::RandomizeMiscEntrances:
        case Option::MixDungeons:
        case Option::MixBosses:
        case Option::MixMinibosses:
        case Option::MixCaves:
        case Option::MixDoors:
        case Option::MixMisc:
        case Option::DecoupleEntrances:
        case Option::RandomStartIsland:
            return 1; // 1-bit Checkbox options
        case Option::StartingGear:
        case Option::ExcludedLocations:
        case Option::INVALID:
        default:
            return static_cast<size_t>(-1); // Invalid or needs special handling
    }
}

#define BYTES_EXIST_CHECK(value) if (value == static_cast<size_t>(-1)) LOG_ERR_AND_RETURN(PermalinkError::COULD_NOT_READ);

PermalinkError Config::loadPermalink(std::string b64permalink) {
    Config load = *this;

    load.converted = false;
    load.updated = false;
    load.configSet = false;

    // Strip trailing spaces
    std::erase_if(b64permalink, [](unsigned char ch){ return std::isspace(ch); });

    if (b64permalink.empty()) {
        LOG_ERR_AND_RETURN(PermalinkError::EMPTY);
    }

    std::string permalink = b64_decode(b64permalink);
    // Empty string gets returned if there was an error
    if (permalink == "") {
        LOG_ERR_AND_RETURN(PermalinkError::BAD_ENCODING);
    }

    // Split the string into 3 parts along the null terminator delimiter
    // 1st part - Version string
    // 2nd part - seed string
    // 3rd part - packed bits representing settings
    std::vector<std::string> permaParts = {};
    const char delimiter = '\0';
    size_t pos = permalink.find(delimiter);
    while (pos != std::string::npos) {
        if (permaParts.size() != 2) {
            permaParts.push_back(permalink.substr(0, pos));
            permalink.erase(0, pos + 1);
        }
        else {
            permaParts.push_back(permalink);
            break;
        }

        pos = permalink.find(delimiter);
    }

    if (permaParts.size() != 3) {
        std::string errorStr = "Bad permalink, parts: ";
        for (const std::string& part : permaParts) {
            errorStr += part + ", ";
        }
        ErrorLog::getInstance().log(errorStr);

        LOG_ERR_AND_RETURN(PermalinkError::MISSING_PARTS);
    }

    const std::string version = permaParts[0];
    load.seed = permaParts[1];
    const std::string optionsBytes = permaParts[2];

    if (version != RANDOMIZER_VERSION) {
        LOG_ERR_AND_RETURN(PermalinkError::INVALID_VERSION);
    }

    const std::vector<char> bytes(optionsBytes.begin(), optionsBytes.end());
    PackedBitsReader bitsReader(bytes);

    for(const Option& option : PERMALINK_OPTIONS) {
        if(option == Option::StartingGear) {
            load.settings.starting_gear.clear();
            for (size_t i = 0; i < REGULAR_ITEMS.size(); i++) {
                const size_t value = bitsReader.read(1);
                BYTES_EXIST_CHECK(value);
                if (value == 1) {
                    load.settings.starting_gear.push_back(REGULAR_ITEMS[i]);
                }
            }
            for (const GameItem& item : PROGRESSIVE_ITEMS) {
                const size_t value = bitsReader.read(3);
                BYTES_EXIST_CHECK(value);
                for (size_t i = 0; i < value; i++) {
                    load.settings.starting_gear.push_back(item);
                }
            }
        }
        else if(option == Option::ExcludedLocations) {
            load.settings.excluded_locations.clear();

            const auto& locations = getAllLocationsNames();
            if(locations.empty()) {
                LOG_ERR_AND_RETURN(PermalinkError::COULD_NOT_LOAD_LOCATIONS);
            }

            for (const auto& locName: locations) {
                const auto value = bitsReader.read(1);
                BYTES_EXIST_CHECK(value);
                if (value == 1){
                    load.settings.excluded_locations.insert(locName);
                }
            }
        }
        else {
            const size_t len = getOptionBitCount(option);
            if(len == static_cast<size_t>(-1)) {
                LOG_ERR_AND_RETURN(PermalinkError::UNHANDLED_OPTION);
            }

            const size_t value = bitsReader.read(len);
            BYTES_EXIST_CHECK(value);
            load.settings.setSetting(option, value);
        }
    }

    if (bitsReader.current_byte_index != bitsReader.bytes.size() - 1) {
        LOG_ERR_AND_RETURN(PermalinkError::INCORRECT_LENGTH);
    }

    // Only overwrite our current config if there were no errors
    load.configSet = true;
    *this = load;

    return PermalinkError::NONE;
}


std::string Config::getPermalink(const bool& internal /* = false */) const {
    std::string permalink = "";

    permalink += RANDOMIZER_VERSION;
    permalink += '\0';
    permalink += seed;
    permalink += '\0';

    // Pack the settings up
    PackedBitsWriter bitsWriter;
    for(const Option& option : PERMALINK_OPTIONS) {
        if(option == Option::StartingGear) {
            const std::multiset<GameItem> startingGear(settings.starting_gear.begin(), settings.starting_gear.end());

            for (size_t i = 0; i < REGULAR_ITEMS.size(); i++)
            {
                const size_t bit = startingGear.contains(REGULAR_ITEMS[i]);
                bitsWriter.write(bit, 1);
            }
            for (const GameItem& item : PROGRESSIVE_ITEMS)
            {
                bitsWriter.write(startingGear.count(item), 3);
            }
        }
        else if(option == Option::ExcludedLocations) {
            const auto& locations = getAllLocationsNames();
            if(locations.empty()) {
                ErrorLog::getInstance().log("Could not load location names for permalink");
                return "";
            }

            for (const auto& locName : locations) {
                const size_t bit = settings.excluded_locations.contains(locName);
                bitsWriter.write(bit, 1);
            }
        }
        else {
            const size_t len = getOptionBitCount(option);
            if(len == static_cast<size_t>(-1)) {
                ErrorLog::getInstance().log("Unhandled option " + settingToName(option));
                return "";
            }

            bitsWriter.write(settings.getSetting(option), len);
        }
    }

    // Add the packed bits to the permalink
    bitsWriter.flush();
    for (const auto& byte : bitsWriter.bytes) {
        permalink += byte;
    }

    permalink = b64_encode(permalink);
    
    if(internal) {
        if(settings.do_not_generate_spoiler_log) permalink += SEED_KEY;

        // Add the plandomizer file contents to the permalink when plandomzier is enabled
        if (settings.plandomizer) {
            std::string plandoContents;
            if (Utility::getFileContents(settings.plandomizerFile, plandoContents) != 0) {
                ErrorLog::getInstance().log("Could not find plandomizer file at " + Utility::toUtf8String(settings.plandomizerFile));
                return "";
            }
            permalink += plandoContents;
        }
    }

    return permalink;
}

ConfigError Config::writeDefault(const fspath& filePath, const fspath& preferencesPath) {
    Config conf;

    // Writes defaults if they don't exist
    std::ifstream file(filePath);
    std::ifstream pref(preferencesPath);

    if (file.is_open() == false) {
        Utility::platformLog("Creating default config");

        conf.seed = generate_seed();
        LOG_AND_RETURN_IF_ERR(conf.writeSettings(filePath))
    }

    if (pref.is_open() == false) {
        Utility::platformLog("Creating default preferences");

        // Load in default link colors
        if (const ModelError err = conf.settings.selectedModel.loadFromFolder(); err != ModelError::NONE) {
            ErrorLog::getInstance().log("Failed to load default colors, error " + errorToName(err));
            LOG_ERR_AND_RETURN(ConfigError::MODEL_ERROR);
        }
        conf.settings.selectedModel.loadPreset(0); // Load default preset

        LOG_AND_RETURN_IF_ERR(conf.writePreferences(preferencesPath))
    }

    return ConfigError::NONE;
}

std::string ConfigErrorGetName(ConfigError err) {
    switch (err) {
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
        case ConfigError::BAD_PERMALINK:
            return "BAD_PERMALINK";
        case ConfigError::INVALID_VALUE:
            return "INVALID_VALUE";
        case ConfigError::MODEL_ERROR:
            return "MODEL_ERROR";
        default:
            return "UNKNOWN";
    }
}

std::string PermalinkErrorGetName(PermalinkError err) {
    switch (err) {
        case PermalinkError::NONE:
            return "NONE";
        case PermalinkError::EMPTY:
            return "EMPTY";
        case PermalinkError::BAD_ENCODING:
            return "BAD_ENCODING";
        case PermalinkError::MISSING_PARTS:
            return "MISSING_PARTS";
        case PermalinkError::INVALID_VERSION:
            return "INVALID_VERSION";
        case PermalinkError::INCORRECT_LENGTH:
            return "INCORRECT_LENGTH";
        case PermalinkError::COULD_NOT_READ:
            return "COULD_NOT_READ";
        case PermalinkError::UNHANDLED_OPTION:
            return "UNHANDLED_OPTION";
        case PermalinkError::COULD_NOT_LOAD_LOCATIONS:
            return "COULD_NOT_LOAD_LOCATIONS";
        default:
            return "UNKNOWN";
    }
}
