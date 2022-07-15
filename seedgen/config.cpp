#include "config.hpp"

#include <fstream>
#include <unordered_set>
#include <filesystem>
#include "../libs/Yaml.hpp"
#include "../logic/GameItem.hpp"
#include "../server/utility/platform.hpp"
#include "../server/command/Log.hpp"
#include <chrono>
#include <thread>



#define GET_FIELD(yaml, name, out) {                                \
        if(yaml[#name].IsNone()) return ConfigError::MISSING_KEY;   \
        out = yaml[#name].As<std::string>();                      \
    }

#define SET_FIELD(yaml, config, name) {                                \
        if(yaml[#name].IsNone()) return ConfigError::MISSING_KEY;   \
        config.name = yaml[#name].As<std::string>();                      \
    }

#define SET_BOOL_FIELD(yaml, config, name) {                                \
        if(yaml[#name].IsNone()) return ConfigError::MISSING_KEY;   \
        config.settings.name = yaml[#name].As<bool>();                      \
    }
    
#define SET_INT_FIELD(yaml, config, name) {                         \
        if(yaml[#name].IsNone()) return ConfigError::MISSING_KEY;   \
        config.settings.name = yaml[#name].As<int>(0);                       \
    }

namespace {
    SwordMode nameToSwordMode(const std::string& name) {
        static std::unordered_map<std::string, SwordMode> nameSwordModeMap = {
            {"StartWithSword", SwordMode::StartWithSword},
            {"RandomSword", SwordMode::RandomSword},
            {"NoSword", SwordMode::NoSword}
        };

        if (nameSwordModeMap.count(name) == 0)
        {
            return SwordMode::INVALID;
        }

        return nameSwordModeMap.at(name);
    }

    std::string SwordModeToName(const SwordMode& mode) {
        static std::unordered_map<SwordMode, std::string> swordModeNameMap = {
            {SwordMode::StartWithSword, "StartWithSword"},
            {SwordMode::RandomSword, "RandomSword"},
            {SwordMode::NoSword, "NoSword"}
        };

        if (swordModeNameMap.count(mode) == 0)
        {
            return "INVALID";
        }

        return swordModeNameMap.at(mode);
    }

    PigColor nameToPigColor(const std::string& name) {
        static std::unordered_map<std::string, PigColor> nameColorMap = {
            {"BLACK", PigColor::BLACK},
            {"PINK", PigColor::PINK},
            {"SPOTTED", PigColor::SPOTTED},
            {"RANDOM", PigColor::RANDOM}
        };

        if (nameColorMap.count(name) == 0)
        {
            return PigColor::INVALID;
        }

        return nameColorMap.at(name);
    }

    std::string PigColorToName(const PigColor& name) {
        static std::unordered_map<PigColor, std::string> colorNameMap = {
            {PigColor::BLACK, "BLACK"},
            {PigColor::PINK, "PINK"},
            {PigColor::SPOTTED, "SPOTTED"},
            {PigColor::RANDOM, "RANDOM"}
        };

        if (colorNameMap.count(name) == 0)
        {
            return "INVALID";
        }

        return colorNameMap.at(name);
    }
}

ConfigError loadFromFile(const std::string& filePath, Config& out) {
    //Check if we can open the file before parsing because exceptions won't work on console
    std::ifstream file(filePath);
    if(!file.is_open()) LOG_ERR_AND_RETURN(ConfigError::COULD_NOT_OPEN);

    Yaml::Node root;
    Yaml::Parse(root, filePath.c_str());

    std::string rando_version, file_version;
    GET_FIELD(root, program_version, rando_version)
    GET_FIELD(root, file_version, file_version)

    if(std::string(CONFIG_VERSION) != file_version) LOG_ERR_AND_RETURN(ConfigError::DIFFERENT_FILE_VERSION);

    //hardcode paths for console, otherwise use config
    #ifdef DEVKITPRO
        out.gameBaseDir = "./backup";
        out.workingDir = "./working";
        out.outputDir = "storage_mlc01:/usr/title/00050000/10143500";
    #else
        SET_FIELD(root, out, gameBaseDir)
        SET_FIELD(root, out, workingDir)
        SET_FIELD(root, out, outputDir)
    #endif

    SET_FIELD(root, out, seed)

    SET_BOOL_FIELD(root, out, progression_dungeons)
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

    SET_BOOL_FIELD(root, out, keylunacy)
    SET_BOOL_FIELD(root, out, randomize_charts)
    SET_BOOL_FIELD(root, out, randomize_starting_island)
    SET_BOOL_FIELD(root, out, randomize_dungeon_entrances)
    SET_BOOL_FIELD(root, out, randomize_cave_entrances)
    SET_BOOL_FIELD(root, out, randomize_door_entrances)
    SET_BOOL_FIELD(root, out, randomize_misc_entrances)
    SET_BOOL_FIELD(root, out, mix_entrance_pools)
    SET_BOOL_FIELD(root, out, decouple_entrances)

    SET_BOOL_FIELD(root, out, instant_text_boxes)
    SET_BOOL_FIELD(root, out, reveal_full_sea_chart)
    SET_INT_FIELD(root, out, num_starting_triforce_shards)
    SET_BOOL_FIELD(root, out, add_shortcut_warps_between_dungeons)
    //SET_FIELD(root, out, settings.sword_mode)
    SET_BOOL_FIELD(root, out, skip_rematch_bosses)
    SET_BOOL_FIELD(root, out, invert_sea_compass_x_axis)
    SET_BOOL_FIELD(root, out, race_mode)
    SET_INT_FIELD(root, out, num_race_mode_dungeons)
    SET_INT_FIELD(root, out, damage_multiplier)
    SET_BOOL_FIELD(root, out, chest_type_matches_contents)
    
    SET_BOOL_FIELD(root, out, player_in_casual_clothes)
    //SET_FIELD(root, out, settings.pig_color)

    //SET_FIELD(root, out, settings.starting_gear)
    SET_INT_FIELD(root, out, starting_pohs)
    SET_INT_FIELD(root, out, starting_hcs)
    SET_BOOL_FIELD(root, out, remove_music)

    SET_BOOL_FIELD(root, out, plandomizer)

    if(root["sword_mode"].IsNone()) return ConfigError::MISSING_KEY;
    out.settings.sword_mode = nameToSwordMode(root["sword_mode"].As<std::string>());
    if(out.settings.sword_mode == SwordMode::INVALID) return ConfigError::INVALID_VALUE;

    if(root["pig_color"].IsNone()) return ConfigError::MISSING_KEY;
    out.settings.pig_color = nameToPigColor(root["pig_color"].As<std::string>());
    if(out.settings.pig_color == PigColor::INVALID) return ConfigError::INVALID_VALUE;

    if(root["starting_gear"].IsNone()) return ConfigError::MISSING_KEY;
    if(!root["starting_gear"].IsSequence()) return ConfigError::INVALID_VALUE;

    static std::unordered_multiset<GameItem> valid_items = {
        /* not currently supported, may be later
        GameItem::DRCSmallKey,
        GameItem::DRCBigKey,
        GameItem::DRCCompass,
        GameItem::DRCDungeonMap,
        
        GameItem::FWSmallKey,
        GameItem::FWBigKey,
        GameItem::FWCompass,
        GameItem::FWDungeonMap,

        GameItem::TotGSmallKey,
        GameItem::TotGBigKey,
        GameItem::TotGCompass,
        GameItem::TotGDungeonMap,

        GameItem::ETSmallKey,
        GameItem::ETBigKey,
        GameItem::ETCompass,
        GameItem::ETDungeonMap,

        GameItem::WTSmallKey,
        GameItem::WTBigKey,
        GameItem::WTCompass,
        GameItem::WTDungeonMap,
        
        GameItem::FFCompass,
        GameItem::FFDungeonMap,
        */
        GameItem::Telescope,
        GameItem::MagicArmor,
        GameItem::HerosCharm,
        GameItem::TingleBottle,
        GameItem::WindWaker,
        GameItem::GrapplingHook,
        GameItem::PowerBracelets,
        GameItem::IronBoots,
        GameItem::Boomerang,
        GameItem::Hookshot,
        GameItem::Bombs,
        GameItem::SkullHammer,
        GameItem::DekuLeaf,
        GameItem::HurricaneSpin,
        GameItem::DinsPearl,
        GameItem::FaroresPearl,
        GameItem::NayrusPearl,
        GameItem::WindsRequiem,
        GameItem::SongOfPassing,
        GameItem::BalladOfGales,
        GameItem::CommandMelody,
        GameItem::EarthGodsLyric,
        GameItem::WindGodsAria,
        GameItem::SpoilsBag,
        GameItem::BaitBag,
        GameItem::DeliveryBag,
        GameItem::NoteToMom,
        GameItem::MaggiesLetter,
        GameItem::MoblinsLetter,
        GameItem::CabanaDeed,
        GameItem::GhostShipChart,
        GameItem::EmptyBottle,
        GameItem::ProgressiveMagicMeter,
        GameItem::ProgressiveMagicMeter,
        GameItem::ProgressiveBombBag,
        GameItem::ProgressiveBombBag,
        GameItem::ProgressiveBow,
        GameItem::ProgressiveBow,
        GameItem::ProgressiveBow,
        GameItem::ProgressiveQuiver,
        GameItem::ProgressiveQuiver,
        GameItem::ProgressiveWallet,
        GameItem::ProgressiveWallet,
        GameItem::ProgressivePictoBox,
        GameItem::ProgressivePictoBox,
        GameItem::ProgressiveSword,
        GameItem::ProgressiveSword,
        GameItem::ProgressiveSword,
        GameItem::ProgressiveShield,
        GameItem::ProgressiveShield,
        GameItem::ProgressiveSail,
        GameItem::ProgressiveSail
    };

    if(out.settings.sword_mode == SwordMode::NoSword) valid_items.erase(GameItem::ProgressiveSword);
    for (auto it = root["starting_gear"].Begin(); it != root["starting_gear"].End(); it++) {
            const Yaml::Node& itemNode = (*it).second;
            const std::string itemName = itemNode.As<std::string>();
            const GameItem item = nameToGameItem(itemName);

            if(valid_items.count(item) == 0) return ConfigError::INVALID_VALUE;
            out.settings.starting_gear.push_back(item);
            valid_items.erase(valid_items.find(item)); //remove the item from the set to catch duplicates or too many progressive items
    }

    //can still parse file with different rando versions, but will give different item placements
    //return error after parsing so it can warn the user
    if(std::string(RANDOMIZER_VERSION) != rando_version) return ConfigError::DIFFERENT_RANDO_VERSION;
    return ConfigError::NONE;
}



#define WRITE_FIELD(yaml, config, name) {   \
        yaml[#name] = config.name;          \
    }

#define WRITE_BOOL_FIELD(yaml, config, name) {          \
        yaml[#name] = config.settings.name ? "true" : "false";   \
    }

#define WRITE_NUM_FIELD(yaml, config, name) {          \
        yaml[#name] = std::to_string(config.settings.name);   \
    }

ConfigError writeToFile(const std::string& filePath, const Config& config) {
//Check if we can open the file before parsing because exceptions won't work on console
    std::ofstream file(filePath);
    if(!file.is_open()) return ConfigError::COULD_NOT_OPEN;

    Yaml::Node root;

    root["program_version"] = RANDOMIZER_VERSION; //Keep track of rando version to give warning (different versions will have different item placements)
    root["file_version"] = CONFIG_VERSION; //Keep track of file version so it can avoid incompatible ones

    WRITE_FIELD(root, config, gameBaseDir)
    WRITE_FIELD(root, config, workingDir)
    WRITE_FIELD(root, config, outputDir)
    WRITE_FIELD(root, config, seed)

    WRITE_BOOL_FIELD(root, config, progression_dungeons)
    WRITE_BOOL_FIELD(root, config, progression_great_fairies)
    WRITE_BOOL_FIELD(root, config, progression_puzzle_secret_caves)
    WRITE_BOOL_FIELD(root, config, progression_combat_secret_caves)
    WRITE_BOOL_FIELD(root, config, progression_short_sidequests)
    WRITE_BOOL_FIELD(root, config, progression_long_sidequests)
    WRITE_BOOL_FIELD(root, config, progression_spoils_trading)
    WRITE_BOOL_FIELD(root, config, progression_minigames)
    WRITE_BOOL_FIELD(root, config, progression_free_gifts)
    WRITE_BOOL_FIELD(root, config, progression_mail)
    WRITE_BOOL_FIELD(root, config, progression_platforms_rafts)
    WRITE_BOOL_FIELD(root, config, progression_submarines)
    WRITE_BOOL_FIELD(root, config, progression_eye_reef_chests)
    WRITE_BOOL_FIELD(root, config, progression_big_octos_gunboats)
    WRITE_BOOL_FIELD(root, config, progression_triforce_charts)
    WRITE_BOOL_FIELD(root, config, progression_treasure_charts)
    WRITE_BOOL_FIELD(root, config, progression_expensive_purchases)
    WRITE_BOOL_FIELD(root, config, progression_misc)
    WRITE_BOOL_FIELD(root, config, progression_tingle_chests)
    WRITE_BOOL_FIELD(root, config, progression_battlesquid)
    WRITE_BOOL_FIELD(root, config, progression_savage_labyrinth)
    WRITE_BOOL_FIELD(root, config, progression_island_puzzles)
    WRITE_BOOL_FIELD(root, config, progression_obscure)

    WRITE_BOOL_FIELD(root, config, keylunacy)
    WRITE_BOOL_FIELD(root, config, randomize_charts)
    WRITE_BOOL_FIELD(root, config, randomize_starting_island)
    WRITE_BOOL_FIELD(root, config, randomize_dungeon_entrances)
    WRITE_BOOL_FIELD(root, config, randomize_cave_entrances)
    WRITE_BOOL_FIELD(root, config, randomize_door_entrances)
    WRITE_BOOL_FIELD(root, config, randomize_misc_entrances)
    WRITE_BOOL_FIELD(root, config, mix_entrance_pools)
    WRITE_BOOL_FIELD(root, config, decouple_entrances)

    WRITE_BOOL_FIELD(root, config, instant_text_boxes)
    WRITE_BOOL_FIELD(root, config, reveal_full_sea_chart)
    WRITE_NUM_FIELD(root, config, num_starting_triforce_shards)
    WRITE_BOOL_FIELD(root, config, add_shortcut_warps_between_dungeons)
    //WRITE_BOOL_FIELD(root, config, sword_mode)
    WRITE_BOOL_FIELD(root, config, skip_rematch_bosses)
    WRITE_BOOL_FIELD(root, config, invert_sea_compass_x_axis)
    WRITE_BOOL_FIELD(root, config, race_mode)
    WRITE_NUM_FIELD(root, config, num_race_mode_dungeons)
    WRITE_NUM_FIELD(root, config, damage_multiplier)
    WRITE_BOOL_FIELD(root, config, chest_type_matches_contents)
    
    WRITE_BOOL_FIELD(root, config, player_in_casual_clothes)
    //WRITE_FIELD(root, config, pig_color)

    //WRITE_FIELD(root, config, starting_gear)
    WRITE_NUM_FIELD(root, config, starting_pohs)
    WRITE_NUM_FIELD(root, config, starting_hcs)
    WRITE_BOOL_FIELD(root, config, remove_music)

    WRITE_BOOL_FIELD(root, config, plandomizer)

    root["sword_mode"] = SwordModeToName(config.settings.sword_mode);
    root["pig_color"] = PigColorToName(config.settings.pig_color);

    root["starting_gear"] = {};
    for (const auto& item : config.settings.starting_gear) {
            Yaml::Node& node = root["starting_gear"].PushBack();
            node = gameItemToName(item);
    }

    Yaml::Serialize(root, filePath.c_str());

    return ConfigError::NONE;
}
