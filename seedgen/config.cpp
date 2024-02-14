#include "config.hpp"

#include <fstream>
#include <unordered_set>
#include <filesystem>

#include <version.hpp>
#include <libs/yaml.hpp>
#include <logic/GameItem.hpp>
#include <seedgen/random.hpp>
#include <utility/color.hpp>
#include <utility/platform.hpp>
#include <command/Log.hpp>



// falls back to current value if the key is not present + errors are ignored
#define GET_FIELD(yaml, key, out)	                         			\
    if(!yaml[key]) {                                                  	\
        Utility::platformLog("\"" key "\" not found in config.yaml\n"); \
        if (!ignoreErrors) {											\
            return ConfigError::MISSING_KEY;							\
        }																\
        else {															\
            converted = true;											\
        }																\
    }            														\
    out = yaml[key].as<decltype(out)>(out);

#define GET_FIELD_NO_FAIL(yaml, key, out) out = yaml[key].as<decltype(out)>(out);

#define SET_FIELD(yaml, key, value) yaml[key] = value;

Config::Config() {
    resetDefaults();

    // paths and stuff that reset doesn't cover
    gameBaseDir = "";
    outputDir = "";
    seed = "";
    settings.plandomizerFile = "";
    
    #ifdef DEVKITPRO
        // these are managed by the title system on console
        /* out.gameBaseDir = "storage_mlc01:/usr/title/00050000/10143500"; */
        /* out.outputDir = "storage_mlc01:/usr/title/00050000/10143599"; */

        settings.plandomizerFile = APP_SAVE_PATH "plandomizer.yaml";
    #endif
}

void Config::resetDefaults() {
    //gameBaseDir = "";
    //outputDir = "";
    //seed = "";

    settings.resetDefaults();

    return;
}

ConfigError Config::loadFromFile(const std::string& filePath, const std::string& preferencesPath, bool ignoreErrors /*= false*/) {
    // check if we can open the file before parsing because exceptions won't work on console
    std::ifstream file(filePath);
    std::ifstream preferences(preferencesPath);
    if(!file.is_open() || !preferences.is_open()) LOG_ERR_AND_RETURN(ConfigError::COULD_NOT_OPEN);
    file.close();
    preferences.close();
    
    YAML::Node root = YAML::LoadFile(filePath);
    YAML::Node preferencesRoot = YAML::LoadFile(preferencesPath);

    std::string rando_version, file_version;
    GET_FIELD(root, "program_version", rando_version)
    GET_FIELD(root, "file_version", file_version)

    if(std::string(CONFIG_VERSION) != file_version) {
        converted = true;

        Utility::platformLog("Attempted to load config version " + file_version + ", current version is " CONFIG_VERSION "\n");
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::DIFFERENT_FILE_VERSION);
    }

    // hardcode paths for console, otherwise use config
    #ifdef DEVKITPRO
        // these are now loaded elsewhere
        /* out.gameBaseDir = "storage_mlc01:/usr/title/00050000/10143500"; */
        /* out.outputDir = "storage_mlc01:/usr/title/00050000/10143599"; */

        settings.plandomizerFile = APP_SAVE_PATH "plandomizer.yaml";
    #else
        std::string baseTemp, outTemp;
        GET_FIELD_NO_FAIL(preferencesRoot, "gameBaseDir", baseTemp)
        gameBaseDir = baseTemp;
        GET_FIELD_NO_FAIL(preferencesRoot, "outputDir", outTemp)
        outputDir = outTemp;
        GET_FIELD_NO_FAIL(preferencesRoot, "plandomizerFile", settings.plandomizerFile)
    #endif

    GET_FIELD_NO_FAIL(root, "seed", seed)

    if(!root["progression_dungeons"]) {
        if(!ignoreErrors) return ConfigError::MISSING_KEY;
    }
    else {
        settings.progression_dungeons = nameToProgressionDungeons(root["progression_dungeons"].as<std::string>("INVALID"));
        if (settings.progression_dungeons == ProgressionDungeons::INVALID) {
            if(!ignoreErrors) {
                return ConfigError::INVALID_VALUE;
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
    GET_FIELD(root, "randomize_cave_entrances", settings.randomize_cave_entrances)
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
        if(!ignoreErrors) return ConfigError::MISSING_KEY;
    }
    else {
        settings.pig_color = nameToPigColor(preferencesRoot["pig_color"].as<std::string>("INVALID"));
        if(settings.pig_color == PigColor::INVALID) {
            if (!ignoreErrors) {
                return ConfigError::INVALID_VALUE;
            }
            else {
                settings.pig_color = PigColor::Random;
            }
        }
    }

    if(!root["dungeon_small_keys"]) {
        if(!ignoreErrors) return ConfigError::MISSING_KEY;
    }
    else {
        settings.dungeon_small_keys = nameToPlacementOption(root["dungeon_small_keys"].as<std::string>("INVALID"));
        if(settings.dungeon_small_keys == PlacementOption::INVALID) {
            if (!ignoreErrors) {
                return ConfigError::INVALID_VALUE;
            }
            else {
                settings.dungeon_small_keys = PlacementOption::Vanilla;
            }
        }
    }

    if(!root["dungeon_big_keys"]) {
        if(!ignoreErrors) return ConfigError::MISSING_KEY;
    }
    else {
        settings.dungeon_big_keys = nameToPlacementOption(root["dungeon_big_keys"].as<std::string>("INVALID"));
        if(settings.dungeon_big_keys == PlacementOption::INVALID) {
            if (!ignoreErrors) {
                return ConfigError::INVALID_VALUE;
            }
            else {
                settings.dungeon_big_keys = PlacementOption::Vanilla;
            }
        }
    }

    if(!root["dungeon_maps_compasses"]) {
        if (!ignoreErrors) return ConfigError::MISSING_KEY;
    }
    else {
        settings.dungeon_maps_compasses = nameToPlacementOption(root["dungeon_maps_compasses"].as<std::string>("INVALID"));
        if(settings.dungeon_maps_compasses == PlacementOption::INVALID) {
            if(!ignoreErrors) {
                return ConfigError::INVALID_VALUE;
            }
            else {
                settings.dungeon_maps_compasses = PlacementOption::Vanilla;
            }
          }
    }

    if(!preferencesRoot["target_type"]) {
        if (!ignoreErrors) return ConfigError::MISSING_KEY;
    }
    else {
        settings.target_type = nameToTargetTypePreference(preferencesRoot["target_type"].as<std::string>("INVALID"));
        if(settings.target_type == TargetTypePreference::INVALID) {
            if(!ignoreErrors) {
                return ConfigError::INVALID_VALUE;
            }
            else {
                settings.target_type = TargetTypePreference::Hold;
            }
          }
    }

    if(!preferencesRoot["camera"]) {
        if (!ignoreErrors) return ConfigError::MISSING_KEY;
    }
    else {
        settings.camera = nameToCameraPreference(preferencesRoot["camera"].as<std::string>("INVALID"));
        if(settings.camera == CameraPreference::INVALID) {
            if(!ignoreErrors) {
                return ConfigError::INVALID_VALUE;
            }
            else {
                settings.camera = CameraPreference::Standard;
            }
          }
    }

    if(!preferencesRoot["first_person_camera"]) {
        if (!ignoreErrors) return ConfigError::MISSING_KEY;
    }
    else {
        settings.first_person_camera = nameToFirstPersonCameraPreference(preferencesRoot["first_person_camera"].as<std::string>("INVALID"));
        if(settings.first_person_camera == FirstPersonCameraPreference::INVALID) {
            if(!ignoreErrors) {
                return ConfigError::INVALID_VALUE;
            }
            else {
                settings.first_person_camera = FirstPersonCameraPreference::Standard;
            }
          }
    }

    if(!preferencesRoot["gyroscope"]) {
        if (!ignoreErrors) return ConfigError::MISSING_KEY;
    }
    else {
        settings.gyroscope = nameToGyroscopePreference(preferencesRoot["gyroscope"].as<std::string>("INVALID"));
        if(settings.gyroscope == GyroscopePreference::INVALID) {
            if(!ignoreErrors) {
                return ConfigError::INVALID_VALUE;
            }
            else {
                settings.gyroscope = GyroscopePreference::On;
            }
          }
    }

    if(!preferencesRoot["ui_display"]) {
        if(!ignoreErrors) return ConfigError::MISSING_KEY;
    }
    else {
        settings.ui_display = nameToUIDisplayPreference(preferencesRoot["ui_display"].as<std::string>("INVALID"));
        if(settings.ui_display == UIDisplayPreference::INVALID) {
            if(!ignoreErrors) {
                return ConfigError::INVALID_VALUE;
            }
            else {
                settings.ui_display = UIDisplayPreference::On;
            }
          }
    }

    if(!root["starting_gear"] || (!root["starting_gear"].IsSequence() && root["starting_gear"].as<std::string>() != "None")) {
        if(!ignoreErrors) return ConfigError::MISSING_KEY;
    }
    else {
        std::unordered_multiset<GameItem> valid_items = getSupportedStartingItems();

        // erase swords if they are not being placed
        if(settings.remove_swords) {
            valid_items.erase(GameItem::ProgressiveSword);
        }

        settings.starting_gear.clear();
        for(const auto& itemObject : root["starting_gear"]) {
            const std::string itemName = itemObject.as<std::string>();
            const GameItem item = nameToGameItem(itemName);

            if(!valid_items.contains(item)) {
                ErrorLog::getInstance().log(itemName + " cannot be added to starting inventory");
                return ConfigError::INVALID_VALUE;
            }

            settings.starting_gear.push_back(item);
            valid_items.erase(valid_items.find(item)); // remove the item from the set to catch duplicates or too many progressive items
        }
    }


    GET_FIELD(preferencesRoot, "player_in_casual_clothes", settings.selectedModel.casual)
    GET_FIELD(preferencesRoot, "custom_player_model", settings.selectedModel.modelName)
    // only non-default colors are written
    if(preferencesRoot["custom_colors"].IsMap()) {
        for(const auto& colorObject : preferencesRoot["custom_colors"]) {
            const std::string texture = colorObject.first.as<std::string>();
            const std::string color = colorObject.second.as<std::string>();

            // only accept the color if it's valid
            if(!isValidHexColor(color)) {
                Utility::platformLog(color + " is not a valid hex color");
                if(!ignoreErrors) return ConfigError::INVALID_VALUE;
            }
            else {
                settings.selectedModel.setColor(texture, color);
            }
        }
    }

    if(settings.selectedModel.loadFromFolder() != ModelError::NONE) {
        if(!ignoreErrors) return ConfigError::MODEL_ERROR;
    }

    // clamp numerical settings
    // health
    settings.starting_hcs = std::clamp<uint16_t>(settings.starting_hcs, 0, MAXIMUM_STARTING_HC);
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
    if(std::string(RANDOMIZER_VERSION) != rando_version) {
        updated = true;

        Utility::platformLog("Warning: config was made using a different randomizer version\n");
        Utility::platformLog("Item placement may be different than expected\n");
        if(!ignoreErrors) LOG_ERR_AND_RETURN(ConfigError::DIFFERENT_RANDO_VERSION);
    }

    return ConfigError::NONE;
}

YAML::Node Config::settingsToYaml() {
    YAML::Node root;

    SET_FIELD(root, "program_version", RANDOMIZER_VERSION) //Keep track of rando version to give warning (different versions will have different item placements)
    SET_FIELD(root, "file_version", CONFIG_VERSION) //Keep track of file version so it can avoid incompatible ones

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
    SET_FIELD(root, "randomize_cave_entrances", settings.randomize_cave_entrances)
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

    return root;
}

YAML::Node Config::preferencesToYaml() {
    YAML::Node preferencesRoot;
    SET_FIELD(preferencesRoot, "gameBaseDir", gameBaseDir.string())
    SET_FIELD(preferencesRoot, "outputDir", outputDir.string())
    SET_FIELD(preferencesRoot, "plandomizerFile", settings.plandomizerFile)

    SET_FIELD(preferencesRoot, "pig_color", PigColorToName(settings.pig_color))

    SET_FIELD(preferencesRoot, "target_type", TargetTypePreferenceToName(settings.target_type))
    SET_FIELD(preferencesRoot, "camera", CameraPreferenceToName(settings.camera))
    SET_FIELD(preferencesRoot, "first_person_camera", FirstPersonCameraPreferenceToName(settings.first_person_camera))
    SET_FIELD(preferencesRoot, "gyroscope", GyroscopePreferenceToName(settings.gyroscope))
    SET_FIELD(preferencesRoot, "ui_display", UIDisplayPreferenceToName(settings.ui_display))

    SET_FIELD(preferencesRoot, "player_in_casual_clothes", settings.selectedModel.casual)
    SET_FIELD(preferencesRoot, "custom_player_model", settings.selectedModel.modelName)
    for (const auto& [texture, color] : settings.selectedModel.getSetColorsMap()) {
        preferencesRoot["custom_colors"][texture] = color;
    }

    return preferencesRoot;
}

ConfigError Config::writeSettings(const std::string& filePath) {

    YAML::Node root = settingsToYaml();
    std::ofstream f(filePath);
    if (f.is_open() == false)
    {
        ErrorLog::getInstance().log("Could not open config at " + filePath);
        return ConfigError::COULD_NOT_OPEN;
    }
    f << root;
    f.close();

    return ConfigError::NONE;
}

ConfigError Config::writePreferences(const std::string& preferencesPath) {

    YAML::Node preferences = preferencesToYaml();
    std::ofstream pref(preferencesPath);
    if (pref.is_open() == false)
    {
        ErrorLog::getInstance().log("Could not open preferences at " + preferencesPath);
        return ConfigError::COULD_NOT_OPEN;
    }
    pref << preferences;
    pref.close();

    return ConfigError::NONE;
}

ConfigError Config::writeToFile(const std::string& filePath, const std::string& preferencesPath) {
    LOG_AND_RETURN_IF_ERR(writeSettings(filePath))
    LOG_AND_RETURN_IF_ERR(writePreferences(preferencesPath))
    return ConfigError::NONE;
}

ConfigError Config::writeDefault(const std::string& filePath, const std::string& preferencesPath) {
    Config conf;

    // Writes defaults if they don't exist
    std::ifstream file(filePath);
    std::ifstream pref(preferencesPath);

    if (file.is_open() == false) {
        Utility::platformLog("Creating default config\n");
        LOG_AND_RETURN_IF_ERR(conf.writeSettings(filePath))
    }

    if (pref.is_open() == false) {
        Utility::platformLog("Creating default preferences\n");
        LOG_AND_RETURN_IF_ERR(conf.writePreferences(preferencesPath))
    }

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
        case ConfigError::MODEL_ERROR:
            return "MODEL_ERROR";
        default:
            return "UNKNOWN";
    }
}
