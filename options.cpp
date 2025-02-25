#include "options.hpp"

#include <unordered_map>

#include <libs/yaml.hpp>

#include <command/Log.hpp>
#include <utility/file.hpp>
#include <utility/platform.hpp>

Settings::Settings() {
    resetDefaultSettings();
    resetDefaultPreferences(true);
}

void Settings::resetDefaultSettings() {
    game_version = GameVersion::HD;

    progression_dungeons = ProgressionDungeons::Standard;
    progression_great_fairies = true;
    progression_puzzle_secret_caves = true;
    progression_combat_secret_caves = false;
    progression_short_sidequests = false;
    progression_long_sidequests = false;
    progression_spoils_trading = false;
    progression_minigames = false;
    progression_free_gifts = true;
    progression_mail = false;
    progression_platforms_rafts = false;
    progression_submarines = false;
    progression_eye_reef_chests = false;
    progression_big_octos_gunboats = false;
    progression_triforce_charts = false;
    progression_treasure_charts = false;
    progression_expensive_purchases = true;
    progression_misc = true;
    progression_tingle_chests = false;
    progression_battlesquid = false;
    progression_savage_labyrinth = false;
    progression_island_puzzles = false;
    progression_dungeon_secrets = false;
    progression_obscure = false;

    dungeon_small_keys = PlacementOption::OwnDungeon;
    dungeon_big_keys = PlacementOption::OwnDungeon;
    dungeon_maps_compasses = PlacementOption::OwnDungeon;
    randomize_charts = false;
    randomize_starting_island = false;
    randomize_dungeon_entrances = false;
    randomize_boss_entrances = false;
    randomize_miniboss_entrances = false;
    randomize_cave_entrances = ShuffleCaveEntrances::Disabled;
    randomize_door_entrances = false;
    randomize_misc_entrances = false;
    mix_dungeons = false;
    mix_bosses = false;
    mix_minibosses = false;
    mix_caves = false;
    mix_doors = false;
    mix_misc = false;
    decouple_entrances = false;

    korl_hints = false;
    ho_ho_hints = false;
    path_hints = false;
    barren_hints = false;
    item_hints = false;
    location_hints = false;
    use_always_hints = false;
    clearer_hints = false;
    hint_importance = false;

    instant_text_boxes = true;
    quiet_swift_sail = false;
    fix_rng = false;
    performance = false;
    reveal_full_sea_chart = true;
    add_shortcut_warps_between_dungeons = false;
    do_not_generate_spoiler_log = false;
    remove_swords = false;
    skip_rematch_bosses = true;
    invert_sea_compass_x_axis = false;
    num_required_dungeons = 0;
    damage_multiplier = 2.0f;
    chest_type_matches_contents = false;

    starting_gear = {
        GameItem::ProgressiveSword,
        GameItem::ProgressiveShield,
        GameItem::BalladOfGales,
        GameItem::SongOfPassing,
        GameItem::ProgressiveMagicMeter,
        GameItem::ProgressiveSail
    };

    excluded_locations = getDefaultExcludedLocations();

    starting_pohs = 0;
    starting_hcs = 3;
    starting_joy_pendants = 0;
    starting_skull_necklaces = 0;
    starting_boko_baba_seeds = 0;
    starting_golden_feathers = 0;
    starting_knights_crests = 0;
    starting_red_chu_jellys = 0;
    starting_green_chu_jellys = 0;
    starting_blue_chu_jellys = 0;
    remove_music = false;

    do_not_generate_spoiler_log = false;
    start_with_random_item = false;
    random_item_slide_item = false;
    classic_mode = false;
    plandomizer = false;

    return;
}

void Settings::resetDefaultPreferences(const bool& paths) {
    if(paths) {
        #ifdef DEVKITPRO
            plandomizerFile = Utility::get_app_save_path() / "plandomizer.yaml";
        #else
            plandomizerFile.clear();
        #endif
    }

    pig_color = PigColor::Random;

    target_type = TargetTypePreference::Hold;
    camera = CameraPreference::Standard;
    first_person_camera = FirstPersonCameraPreference::Standard;
    gyroscope = GyroscopePreference::On;
    ui_display = UIDisplayPreference::On;

    selectedModel.casual = false;
    selectedModel.modelName = "Link";
    selectedModel.resetColors();

    return;
}

uint8_t Settings::getSetting(const Option& option) const {
    switch (option) {
        case Option::GameVersion:
            return static_cast<std::underlying_type_t<GameVersion>>(game_version);
        case Option::ProgressDungeons:
            return static_cast<std::underlying_type_t<ProgressionDungeons>>(progression_dungeons);
        case Option::ProgressGreatFairies:
            return progression_great_fairies;
        case Option::ProgressPuzzleCaves:
            return progression_puzzle_secret_caves;
        case Option::ProgressCombatCaves:
            return progression_combat_secret_caves;
        case Option::ProgressShortSidequests:
            return progression_short_sidequests;
        case Option::ProgressLongSidequests:
            return progression_long_sidequests;
        case Option::ProgressSpoilsTrading:
            return progression_spoils_trading;
        case Option::ProgressMinigames:
            return progression_minigames;
        case Option::ProgressFreeGifts:
            return progression_free_gifts;
        case Option::ProgressMail:
            return progression_mail;
        case Option::ProgressPlatformsRafts:
            return progression_platforms_rafts;
        case Option::ProgressSubmarines:
            return progression_submarines;
        case Option::ProgressEyeReefs:
            return progression_eye_reef_chests;
        case Option::ProgressOctosGunboats:
            return progression_big_octos_gunboats;
        case Option::ProgressTriforceCharts:
            return progression_triforce_charts;
        case Option::ProgressTreasureCharts:
            return progression_treasure_charts;
        case Option::ProgressExpPurchases:
            return progression_expensive_purchases;
        case Option::ProgressMisc:
            return progression_misc;
        case Option::ProgressTingleChests:
            return progression_tingle_chests;
        case Option::ProgressBattlesquid:
            return progression_battlesquid;
        case Option::ProgressSavageLabyrinth:
            return progression_savage_labyrinth;
        case Option::ProgressIslandPuzzles:
            return progression_island_puzzles;
        case Option::ProgressDungeonSecrets:
            return progression_dungeon_secrets;
        case Option::ProgressObscure:
            return progression_obscure;
        case Option::DungeonSmallKeys:
            return static_cast<std::underlying_type_t<PlacementOption>>(dungeon_small_keys);
        case Option::DungeonBigKeys:
            return static_cast<std::underlying_type_t<PlacementOption>>(dungeon_big_keys);
        case Option::DungeonMapsAndCompasses:
            return static_cast<std::underlying_type_t<PlacementOption>>(dungeon_maps_compasses);
        case Option::RandomCharts:
            return randomize_charts;
        case Option::RandomStartIsland:
            return randomize_starting_island;
        case Option::RandomizeDungeonEntrances:
            return randomize_dungeon_entrances;
        case Option::RandomizeMinibossEntrances:
            return randomize_miniboss_entrances;
        case Option::RandomizeBossEntrances:
            return randomize_boss_entrances;
        case Option::RandomizeCaveEntrances:
            return static_cast<std::underlying_type_t<ShuffleCaveEntrances>>(randomize_cave_entrances);
        case Option::RandomizeDoorEntrances:
            return randomize_door_entrances;
        case Option::RandomizeMiscEntrances:
            return randomize_misc_entrances;
        case Option::MixDungeons:
            return mix_dungeons;
        case Option::MixBosses:
            return mix_bosses;
        case Option::MixMinibosses:
            return mix_minibosses;
        case Option::MixCaves:
            return mix_caves;
        case Option::MixDoors:
            return mix_doors;
        case Option::MixMisc:
            return mix_misc;
        case Option::DecoupleEntrances:
            return decouple_entrances;
        case Option::HoHoHints:
            return ho_ho_hints;
        case Option::KorlHints:
            return korl_hints;
        case Option::ClearerHints:
            return clearer_hints;
        case Option::UseAlwaysHints:
            return use_always_hints;
        case Option::HintImportance:
            return hint_importance;
        case Option::PathHints:
            return path_hints;
        case Option::BarrenHints:
            return barren_hints;
        case Option::ItemHints:
            return item_hints;
        case Option::LocationHints:
            return location_hints;
        case Option::InstantText:
            return instant_text_boxes;
        case Option::QuietSwiftSail:
            return quiet_swift_sail;
        case Option::FixRNG:
            return fix_rng;
        case Option::Performance:
            return performance;
        case Option::RevealSeaChart:
            return reveal_full_sea_chart;
        case Option::AddShortcutWarps:
            return add_shortcut_warps_between_dungeons;
        case Option::NoSpoilerLog:
            return do_not_generate_spoiler_log;
        case Option::RemoveSwords:
            return remove_swords;
        case Option::SkipRefights:
            return skip_rematch_bosses;
        case Option::InvertCompass:
            return invert_sea_compass_x_axis;
        case Option::NumRequiredDungeons:
            return num_required_dungeons;
        case Option::DamageMultiplier:
            return static_cast<uint8_t>(damage_multiplier);
        case Option::CTMC:
            return chest_type_matches_contents;
        case Option::PigColor:
            return static_cast<std::underlying_type_t<PigColor>>(pig_color);
        //cant return these like everything else, just here as placeholder
        case Option::StartingGear:
        case Option::ExcludedLocations:
            return 0;
        case Option::StartingHP:
            return starting_pohs;
        case Option::StartingHC:
            return starting_hcs;
        case Option::StartingJoyPendants:
            return starting_joy_pendants;
        case Option::StartingSkullNecklaces:
            return starting_skull_necklaces;
        case Option::StartingBokoBabaSeeds:
            return starting_boko_baba_seeds;
        case Option::StartingGoldenFeathers:
            return starting_golden_feathers;
        case Option::StartingKnightsCrests:
            return starting_knights_crests;
        case Option::StartingRedChuJellys:
            return starting_red_chu_jellys;
        case Option::StartingGreenChuJellys:
            return starting_green_chu_jellys;
        case Option::StartingBlueChuJellys:
            return starting_blue_chu_jellys;
        case Option::RemoveMusic:
            return remove_music;
        case Option::StartWithRandomItem:
            return start_with_random_item;
        case Option::RandomItemSlideItem:
            return random_item_slide_item;
        case Option::ClassicMode:
            return classic_mode;
        case Option::Plandomizer:
            return plandomizer;
        case Option::PlandomizerFile: //cant return this like everything else, just here as placeholder
            return 0;
        case Option::TargetType:
            return static_cast<std::underlying_type_t<TargetTypePreference>>(target_type);   
        case Option::Camera:
            return static_cast<std::underlying_type_t<CameraPreference>>(camera);
        case Option::FirstPersonCamera:
            return static_cast<std::underlying_type_t<FirstPersonCameraPreference>>(first_person_camera);
        case Option::Gyroscope:
            return static_cast<std::underlying_type_t<GyroscopePreference>>(gyroscope);
        case Option::UIDisplay:
            return static_cast<std::underlying_type_t<UIDisplayPreference>>(ui_display);
        default:
            return 0;
    }
}

void Settings::setSetting(const Option& option, const size_t& value) {
    switch (option) {
        case Option::GameVersion:
            game_version = static_cast<GameVersion>(value); return;
        case Option::ProgressDungeons:
            progression_dungeons = static_cast<ProgressionDungeons>(value); return;
        case Option::ProgressGreatFairies:
            progression_great_fairies = value; return;
        case Option::ProgressPuzzleCaves:
            progression_puzzle_secret_caves = value; return;
        case Option::ProgressCombatCaves:
            progression_combat_secret_caves = value; return;
        case Option::ProgressShortSidequests:
            progression_short_sidequests = value; return;
        case Option::ProgressLongSidequests:
            progression_long_sidequests = value; return;
        case Option::ProgressSpoilsTrading:
            progression_spoils_trading = value; return;
        case Option::ProgressMinigames:
            progression_minigames = value; return;
        case Option::ProgressFreeGifts:
            progression_free_gifts = value; return;
        case Option::ProgressMail:
            progression_mail = value; return;
        case Option::ProgressPlatformsRafts:
            progression_platforms_rafts = value; return;
        case Option::ProgressSubmarines:
            progression_submarines = value; return;
        case Option::ProgressEyeReefs:
            progression_eye_reef_chests = value; return;
        case Option::ProgressOctosGunboats:
            progression_big_octos_gunboats = value; return;
        case Option::ProgressTriforceCharts:
            progression_triforce_charts = value; return;
        case Option::ProgressTreasureCharts:
            progression_treasure_charts = value; return;
        case Option::ProgressExpPurchases:
            progression_expensive_purchases = value; return;
        case Option::ProgressMisc:
            progression_misc = value; return;
        case Option::ProgressTingleChests:
            progression_tingle_chests = value; return;
        case Option::ProgressBattlesquid:
            progression_battlesquid = value; return;
        case Option::ProgressSavageLabyrinth:
            progression_savage_labyrinth = value; return;
        case Option::ProgressIslandPuzzles:
            progression_island_puzzles = value; return;
        case Option::ProgressDungeonSecrets:
            progression_dungeon_secrets = value; return;
        case Option::ProgressObscure:
            progression_obscure = value; return;
        case Option::DungeonSmallKeys:
            dungeon_small_keys = static_cast<PlacementOption>(value); return;
        case Option::DungeonBigKeys:
            dungeon_big_keys = static_cast<PlacementOption>(value); return;
        case Option::DungeonMapsAndCompasses:
            dungeon_maps_compasses = static_cast<PlacementOption>(value); return;
        case Option::RandomCharts:
            randomize_charts = value; return;
        case Option::RandomStartIsland:
            randomize_starting_island = value; return;
        case Option::RandomizeDungeonEntrances:
            randomize_dungeon_entrances = value; return;
        case Option::RandomizeBossEntrances:
            randomize_boss_entrances = value; return;
        case Option::RandomizeMinibossEntrances:
            randomize_miniboss_entrances = value; return;
        case Option::RandomizeCaveEntrances:
            randomize_cave_entrances = static_cast<ShuffleCaveEntrances>(value); return;
        case Option::RandomizeDoorEntrances:
            randomize_door_entrances = value; return;
        case Option::RandomizeMiscEntrances:
            randomize_misc_entrances = value; return;
        case Option::MixDungeons:
            mix_dungeons = value; return;
        case Option::MixBosses:
            mix_bosses = value; return;
        case Option::MixMinibosses:
            mix_minibosses = value; return;
        case Option::MixCaves:
            mix_caves = value; return;
        case Option::MixDoors:
            mix_doors = value; return;
        case Option::MixMisc:
            mix_misc = value; return;
        case Option::DecoupleEntrances:
            decouple_entrances = value; return;
        case Option::HoHoHints:
            ho_ho_hints = value; return;
        case Option::KorlHints:
            korl_hints = value; return;
        case Option::ClearerHints:
            clearer_hints = value; return;
        case Option::UseAlwaysHints:
            use_always_hints = value; return;
        case Option::HintImportance:
            hint_importance = value; return;
        case Option::PathHints:
            path_hints = value; return;
        case Option::BarrenHints:
            barren_hints = value; return;
        case Option::ItemHints:
            item_hints = value; return;
        case Option::LocationHints:
            location_hints = value; return;
        case Option::InstantText:
            instant_text_boxes = value; return;
        case Option::QuietSwiftSail:
            quiet_swift_sail = value; return;
        case Option::FixRNG:
            fix_rng = value; return;
        case Option::Performance:
            performance = value; return;
        case Option::RevealSeaChart:
            reveal_full_sea_chart = value; return;
        case Option::AddShortcutWarps:
            add_shortcut_warps_between_dungeons = value; return;
        case Option::NoSpoilerLog:
            do_not_generate_spoiler_log = value; return;
        case Option::RemoveSwords:
            remove_swords = value; return;
        case Option::SkipRefights:
            skip_rematch_bosses = value; return;
        case Option::InvertCompass:
            invert_sea_compass_x_axis = value; return;
        case Option::NumRequiredDungeons:
            num_required_dungeons = value; return;
        case Option::DamageMultiplier:
            damage_multiplier = value; return;
        case Option::CTMC:
            chest_type_matches_contents = value; return;
        case Option::PigColor:
            pig_color = static_cast<PigColor>(value); return;
        //cant set these like everything else, just here as placeholder
        case Option::StartingGear: 
        case Option::ExcludedLocations:
            return;
        case Option::StartingHP:
            starting_pohs = value; return;
        case Option::StartingHC:
            starting_hcs = value; return;
        case Option::StartingJoyPendants:
            starting_joy_pendants = value; return;
        case Option::StartingSkullNecklaces:
            starting_skull_necklaces = value; return;
        case Option::StartingBokoBabaSeeds:
            starting_boko_baba_seeds = value; return;
        case Option::StartingGoldenFeathers:
            starting_golden_feathers = value; return;
        case Option::StartingKnightsCrests:
            starting_knights_crests = value; return;
        case Option::StartingRedChuJellys:
            starting_red_chu_jellys = value; return;
        case Option::StartingGreenChuJellys:
            starting_green_chu_jellys = value; return;
        case Option::StartingBlueChuJellys:
            starting_blue_chu_jellys = value; return;
        case Option::RemoveMusic:
            remove_music = value; return;
        case Option::StartWithRandomItem:
            start_with_random_item = value; return;
        case Option::RandomItemSlideItem:
            random_item_slide_item = value; return;
        case Option::ClassicMode:
            classic_mode = value; return;
        case Option::Plandomizer:
            plandomizer = value; return;
        case Option::PlandomizerFile: //cant set this like everything else, just here as placeholder
            return;
        case Option::TargetType:
            target_type = static_cast<TargetTypePreference>(value); return;
        case Option::Camera:
            camera = static_cast<CameraPreference>(value); return;
        case Option::FirstPersonCamera:
            first_person_camera = static_cast<FirstPersonCameraPreference>(value); return;
        case Option::Gyroscope:
            gyroscope = static_cast<GyroscopePreference>(value); return;
        case Option::UIDisplay:
            ui_display = static_cast<UIDisplayPreference>(value); return;
        default:
            return;
    }
}

int Settings::evaluateOption(const std::string& optionStr) const {
    if (nameToSetting(optionStr) != Option::INVALID)
    {
        return getSetting(nameToSetting(optionStr));
    }

    // -1 means that the setting doesn't exist
    return -1;
}

static const std::unordered_map<std::string, GameVersion> nameGameVersionMap = {
    {"HD", GameVersion::HD},
    {"SD", GameVersion::SD}
};

static const std::unordered_map<GameVersion, std::string> gameVersionNameMap = {
    {GameVersion::HD, "HD"},
    {GameVersion::SD, "SD"}
};

static const std::unordered_map<std::string, PigColor> namePigColorMap = {
    {"Black", PigColor::Black},
    {"Pink", PigColor::Pink},
    {"Spotted", PigColor::Spotted},
    {"Random", PigColor::Random}
};

static const std::unordered_map<PigColor, std::string> pigColorNameMap = {
    {PigColor::Black, "Black"},
    {PigColor::Pink, "Pink"},
    {PigColor::Spotted, "Spotted"},
    {PigColor::Random, "Random"}
};

static const std::unordered_map<PlacementOption, std::string> placementOptionNameMap = {
    {PlacementOption::Vanilla, "Vanilla"},
    {PlacementOption::OwnDungeon, "Own Dungeon"},
    {PlacementOption::AnyDungeon, "Any Dungeon"},
    {PlacementOption::Overworld, "Overworld"},
    {PlacementOption::Keysanity, "Keysanity"},
};

static const std::unordered_map<std::string, PlacementOption> namePlacementOptionMap = {
    {"Vanilla", PlacementOption::Vanilla},
    {"Own Dungeon", PlacementOption::OwnDungeon},
    {"Any Dungeon", PlacementOption::AnyDungeon},
    {"Overworld", PlacementOption::Overworld},
    {"Keysanity", PlacementOption::Keysanity},
};

static const std::unordered_map<ProgressionDungeons, std::string> progressionDungeonsNameMap = {
    {ProgressionDungeons::Disabled, "Disabled"},
    {ProgressionDungeons::Standard, "Standard"},
    {ProgressionDungeons::RaceMode, "Race Mode"},
};

static const std::unordered_map<std::string, ProgressionDungeons> nameProgressionDungeonsMap = {
    {"Disabled", ProgressionDungeons::Disabled},
    {"Standard", ProgressionDungeons::Standard},
    {"Race Mode", ProgressionDungeons::RaceMode},
};

static const std::unordered_map<ShuffleCaveEntrances, std::string> shuffleCaveEntrancesNameMap = {
    {ShuffleCaveEntrances::Disabled, "Disabled"},
    {ShuffleCaveEntrances::Caves, "Caves"},
    {ShuffleCaveEntrances::CavesFairies, "Caves and Fairies"},
};

static const std::unordered_map<std::string, ShuffleCaveEntrances> nameShuffleCaveEntrancesMap = {
    {"Disabled", ShuffleCaveEntrances::Disabled},
    {"Caves", ShuffleCaveEntrances::Caves},
    {"Caves and Fairies", ShuffleCaveEntrances::CavesFairies},
};

static const std::unordered_map<TargetTypePreference, std::string> targetTypePreferenceNameMap = {
    {TargetTypePreference::Hold, "Hold"},
    {TargetTypePreference::Switch, "Switch"},  
};

static const std::unordered_map<std::string, TargetTypePreference> nameTargetTypePreferenceMap = {
    {"Hold", TargetTypePreference::Hold},
    {"Switch", TargetTypePreference::Switch},
};

static const std::unordered_map<CameraPreference, std::string> cameraPreferenceNameMap = {
    {CameraPreference::Standard, "Standard"},
    {CameraPreference::ReverseLeftRight, "Reverse Left/Right"},  
};

static const std::unordered_map<std::string, CameraPreference> nameCameraPreferenceMap = {
    {"Standard", CameraPreference::Standard},
    {"Reverse Left/Right", CameraPreference::ReverseLeftRight},
};

static const std::unordered_map<FirstPersonCameraPreference, std::string> firstPersonCameraPreferenceNameMap = {
    {FirstPersonCameraPreference::Standard, "Standard"},
    {FirstPersonCameraPreference::ReverseUpDown, "Reverse Up/Down"},  
};

static const std::unordered_map<std::string, FirstPersonCameraPreference> nameFirstPersonCameraPreferenceMap = {
    {"Standard", FirstPersonCameraPreference::Standard},
    {"Reverse Up/Down", FirstPersonCameraPreference::ReverseUpDown},
};

static const std::unordered_map<GyroscopePreference, std::string> gyroscopePreferenceNameMap = {
    {GyroscopePreference::Off, "Off"},
    {GyroscopePreference::On, "On"},  
};

static const std::unordered_map<std::string, GyroscopePreference> nameGyroscopePreferenceMap = {
    {"Off", GyroscopePreference::Off},
    {"On", GyroscopePreference::On},
};

static const std::unordered_map<UIDisplayPreference, std::string> uiDisplayPreferenceNameMap = {
    {UIDisplayPreference::On, "On"},
    {UIDisplayPreference::Off, "Off"},  
};

static const std::unordered_map<std::string, UIDisplayPreference> nameUIDisplayPreferenceMap = {
    {"On", UIDisplayPreference::On},
    {"Off", UIDisplayPreference::Off},
};



GameVersion nameToGameVersion(const std::string& name) {
    if (nameGameVersionMap.contains(name))
    {
        return nameGameVersionMap.at(name);
    }

    return GameVersion::INVALID;
}

std::string GameVersionToName(const GameVersion& version) {
    if (gameVersionNameMap.contains(version))
    {
        return gameVersionNameMap.at(version);
    }

    return "INVALID";
}

PigColor nameToPigColor(const std::string& name) {
    if (namePigColorMap.contains(name))
    {
        return namePigColorMap.at(name);
    }

    return PigColor::INVALID;
}

std::string PigColorToName(const PigColor& color) {
    if (pigColorNameMap.contains(color))
    {
        return pigColorNameMap.at(color);
    }

    return "INVALID";
}

PlacementOption nameToPlacementOption(const std::string& name) {
    if (namePlacementOptionMap.contains(name))
    {
        return namePlacementOptionMap.at(name);
    }

    return PlacementOption::INVALID;
}

std::string PlacementOptionToName(const PlacementOption& option) {
    if (placementOptionNameMap.contains(option))
    {
        return placementOptionNameMap.at(option);
    }

    return "INVALID";
}

ProgressionDungeons nameToProgressionDungeons(const std::string& name) {
    if (nameProgressionDungeonsMap.contains(name))
    {
        return nameProgressionDungeonsMap.at(name);
    }

    return ProgressionDungeons::INVALID;
}

std::string ProgressionDungeonsToName(const ProgressionDungeons& option) {
    if (progressionDungeonsNameMap.contains(option))
    {
        return progressionDungeonsNameMap.at(option);
    }

    return "INVALID";
}

ShuffleCaveEntrances nameToShuffleCaveEntrances(const std::string& name) {
    if (nameShuffleCaveEntrancesMap.contains(name))
    {
        return nameShuffleCaveEntrancesMap.at(name);
    }

    return ShuffleCaveEntrances::INVALID;
}

std::string ShuffleCaveEntrancesToName(const ShuffleCaveEntrances& option) {
    if (shuffleCaveEntrancesNameMap.contains(option))
    {
        return shuffleCaveEntrancesNameMap.at(option);
    }

    return "INVALID";
}

TargetTypePreference nameToTargetTypePreference(const std::string& name)
{
    if (nameTargetTypePreferenceMap.contains(name))
    {
        return nameTargetTypePreferenceMap.at(name);
    }

    return TargetTypePreference::INVALID;
}

std::string TargetTypePreferenceToName(const TargetTypePreference& preference)
{
    if (targetTypePreferenceNameMap.contains(preference))
    {
        return targetTypePreferenceNameMap.at(preference);
    }

    return "INVALID";
}

CameraPreference nameToCameraPreference(const std::string& name)
{
    if (nameCameraPreferenceMap.contains(name))
    {
        return nameCameraPreferenceMap.at(name);
    }

    return CameraPreference::INVALID;
}

std::string CameraPreferenceToName(const CameraPreference& preference)
{
    if (cameraPreferenceNameMap.contains(preference))
    {
        return cameraPreferenceNameMap.at(preference);
    }

    return "INVALID";
}

FirstPersonCameraPreference nameToFirstPersonCameraPreference(const std::string& name)
{
    if (nameFirstPersonCameraPreferenceMap.contains(name))
    {
        return nameFirstPersonCameraPreferenceMap.at(name);
    }

    return FirstPersonCameraPreference::INVALID;
}

std::string FirstPersonCameraPreferenceToName(const FirstPersonCameraPreference& preference)
{
    if (firstPersonCameraPreferenceNameMap.contains(preference))
    {
        return firstPersonCameraPreferenceNameMap.at(preference);
    }

    return "INVALID";
}

GyroscopePreference nameToGyroscopePreference(const std::string& name)
{
    if (nameGyroscopePreferenceMap.contains(name))
    {
        return nameGyroscopePreferenceMap.at(name);
    }

    return GyroscopePreference::INVALID;
}

std::string GyroscopePreferenceToName(const GyroscopePreference& preference)
{
    if (gyroscopePreferenceNameMap.contains(preference))
    {
        return gyroscopePreferenceNameMap.at(preference);
    }

    return "INVALID";
}

UIDisplayPreference nameToUIDisplayPreference(const std::string& name)
{
    if (nameUIDisplayPreferenceMap.contains(name))
    {
        return nameUIDisplayPreferenceMap.at(name);
    }

    return UIDisplayPreference::INVALID;
}

std::string UIDisplayPreferenceToName(const UIDisplayPreference& preference)
{
    if (uiDisplayPreferenceNameMap.contains(preference))
    {
        return uiDisplayPreferenceNameMap.at(preference);
    }

    return "INVALID";
}

// Make sure there aren't any naming conflicts when adding future settings
int nameToSettingInt(const std::string& name) {
    if (namePigColorMap.contains(name))
    {
        return static_cast<std::underlying_type_t<PigColor>>(namePigColorMap.at(name));
    }
    if (nameTargetTypePreferenceMap.contains(name))
    {
        return static_cast<std::underlying_type_t<TargetTypePreference>>(nameTargetTypePreferenceMap.at(name));
    }
    if (nameCameraPreferenceMap.contains(name))
    {
        return static_cast<std::underlying_type_t<CameraPreference>>(nameCameraPreferenceMap.at(name));
    }
    if (nameFirstPersonCameraPreferenceMap.contains(name))
    {
        return static_cast<std::underlying_type_t<FirstPersonCameraPreference>>(nameFirstPersonCameraPreferenceMap.at(name));
    }
    if (nameGyroscopePreferenceMap.contains(name))
    {
        return static_cast<std::underlying_type_t<GyroscopePreference>>(nameGyroscopePreferenceMap.at(name));
    }
    if (nameUIDisplayPreferenceMap.contains(name))
    {
        return static_cast<std::underlying_type_t<UIDisplayPreference>>(nameUIDisplayPreferenceMap.at(name));
    }

    return 0;
}

Option nameToSetting(const std::string& name) {
    static std::unordered_map<std::string, Option> optionNameMap = {
        {"Progress Dungeons", Option::ProgressDungeons},
        {"Progress Great Fairies", Option::ProgressGreatFairies},
        {"Progress Puzzle Caves", Option::ProgressPuzzleCaves},
        {"Progress Combat Caves", Option::ProgressCombatCaves},
        {"Progress Short Sidequests", Option::ProgressShortSidequests},
        {"Progress Long Sidequests", Option::ProgressLongSidequests},
        {"Progress Spoils Trading", Option::ProgressSpoilsTrading},
        {"Progress Minigames", Option::ProgressMinigames},
        {"Progress Free Gifts", Option::ProgressFreeGifts},
        {"Progress Mail", Option::ProgressMail},
        {"Progress Platforms Rafts", Option::ProgressPlatformsRafts},
        {"Progress Submarines", Option::ProgressSubmarines},
        {"Progress Eye Reefs", Option::ProgressEyeReefs},
        {"Progress Octos Gunboats", Option::ProgressOctosGunboats},
        {"Progress Triforce Charts", Option::ProgressTriforceCharts},
        {"Progress Treasure Charts", Option::ProgressTreasureCharts},
        {"Progress Exp Purchases", Option::ProgressExpPurchases},
        {"Progress Misc", Option::ProgressMisc},
        {"Progress Tingle Chests", Option::ProgressTingleChests},
        {"Progress Battlesquid", Option::ProgressBattlesquid},
        {"Progress Savage Labyrinth", Option::ProgressSavageLabyrinth},
        {"Progress Island Puzzles", Option::ProgressIslandPuzzles},
        {"Progress Dungeon Secrets", Option::ProgressDungeonSecrets},
        {"Progress Obscure", Option::ProgressObscure},
        {"Dungeon Small Keys", Option::DungeonSmallKeys},
        {"Dungeon Big Keys", Option::DungeonBigKeys},
        {"Dungeon Maps And Compasses", Option::DungeonMapsAndCompasses},
        {"RandomCharts", Option::RandomCharts},
        {"Random Start Island", Option::RandomStartIsland},
        {"Randomize Dungeon Entrances", Option::RandomizeDungeonEntrances},
        {"Randomize Boss Entrances", Option::RandomizeBossEntrances},
        {"Randomize Miniboss Entrances", Option::RandomizeMinibossEntrances},
        {"Randomize Cave Entrances", Option::RandomizeCaveEntrances},
        {"Randomize Door Entrances", Option::RandomizeDoorEntrances},
        {"Randomize Misc Entrances", Option::RandomizeMiscEntrances},
        {"Mix Dungeons", Option::MixDungeons},
        {"Mix Bosses", Option::MixBosses},
        {"Mix Minibosses", Option::MixMinibosses},
        {"Mix Caves", Option::MixCaves},
        {"Mix Doors", Option::MixDoors},
        {"Mix Misc", Option::MixMisc},
        {"Decouple Entrances", Option::DecoupleEntrances},
        {"Ho Ho Hints", Option::HoHoHints},
        {"Korl Hints", Option::KorlHints},
        {"Clearer Hints", Option::ClearerHints},
        {"Use Always Hints", Option::UseAlwaysHints},
        {"Hint Importance", Option::HintImportance},
        {"Path ints", Option::PathHints},
        {"Barren Hints", Option::BarrenHints},
        {"Item Hints", Option::ItemHints},
        {"Location Hints", Option::LocationHints},
        {"Instant Text", Option::InstantText},
        {"Quiet Swift Sail", Option::QuietSwiftSail},
        {"Fix RNG", Option::FixRNG},
        {"Performance", Option::Performance},
        {"Reveal Sea Chart", Option::RevealSeaChart},
        {"Add Shortcut Warps", Option::AddShortcutWarps},
        {"No Spoiler Log", Option::NoSpoilerLog},
        {"Remove Swords", Option::RemoveSwords},
        {"Skip Refights", Option::SkipRefights},
        {"Invert Compass", Option::InvertCompass},
        {"Num Required Dungeons", Option::NumRequiredDungeons},
        {"Damage Multiplier", Option::DamageMultiplier},
        {"CTMC", Option::CTMC},
        {"Pig Color", Option::PigColor},
        {"Starting Gear", Option::StartingGear},
        {"Excluded Locations", Option::ExcludedLocations},
        {"Starting HP", Option::StartingHP},
        {"Starting HC", Option::StartingHC},
        {"Starting Joy Pendants", Option::StartingJoyPendants},
        {"Starting Skull Necklaces", Option::StartingSkullNecklaces},
        {"Starting Boko Baba Seeds", Option::StartingBokoBabaSeeds},
        {"Starting Golden Feathers", Option::StartingGoldenFeathers},
        {"Starting Knights Crests", Option::StartingKnightsCrests},
        {"Starting Red Chu Jellys", Option::StartingRedChuJellys},
        {"Starting Green Chu Jellys", Option::StartingGreenChuJellys},
        {"Starting Blue Chu Jellys", Option::StartingBlueChuJellys},
        {"Remove Music", Option::RemoveMusic},
        {"Start With Random Item", Option::StartWithRandomItem},
        {"Start With Random Item Sliding Item", Option::RandomItemSlideItem},
        {"Classic Mode", Option::ClassicMode},
        {"Plandomizer", Option::Plandomizer},
        {"Plandomizer File", Option::PlandomizerFile},
        {"Target Type", Option::TargetType},
        {"Camera", Option::Camera},
        {"First-Person Camera", Option::FirstPersonCamera},
        {"Gyroscope", Option::Gyroscope},
        {"UI Display", Option::UIDisplay},
    };

    if (optionNameMap.contains(name))
    {
        return optionNameMap.at(name);
    }

    return Option::INVALID;
}

std::string settingToName(const Option& setting) {
    static std::unordered_map<Option, std::string> optionNameMap = {
        {Option::ProgressDungeons, "Progress Dungeons"},
        {Option::ProgressGreatFairies, "Progress Great Fairies"},
        {Option::ProgressPuzzleCaves, "Progress Puzzle Caves"},
        {Option::ProgressCombatCaves, "Progress Combat Caves"},
        {Option::ProgressShortSidequests, "Progress Short Sidequests"},
        {Option::ProgressLongSidequests, "Progress Long Sidequests"},
        {Option::ProgressSpoilsTrading, "Progress Spoils Trading"},
        {Option::ProgressMinigames, "Progress Minigames"},
        {Option::ProgressFreeGifts, "Progress Free Gifts"},
        {Option::ProgressMail, "Progress Mail"},
        {Option::ProgressPlatformsRafts, "Progress Platforms Rafts"},
        {Option::ProgressSubmarines, "Progress Submarines"},
        {Option::ProgressEyeReefs, "Progress Eye Reefs"},
        {Option::ProgressOctosGunboats, "Progress Octos Gunboats"},
        {Option::ProgressTriforceCharts, "Progress Triforce Charts"},
        {Option::ProgressTreasureCharts, "Progress Treasure Charts"},
        {Option::ProgressExpPurchases, "Progress Exp Purchases"},
        {Option::ProgressMisc, "Progress Misc"},
        {Option::ProgressTingleChests, "Progress Tingle Chests"},
        {Option::ProgressBattlesquid, "Progress Battlesquid"},
        {Option::ProgressSavageLabyrinth, "Progress Savage Labyrinth"},
        {Option::ProgressIslandPuzzles, "Progress Island Puzzles"},
        {Option::ProgressDungeonSecrets, "Progress Dungeon Secrets"},
        {Option::ProgressObscure, "Progress Obscure"},
        {Option::DungeonSmallKeys, "Dungeon Small Keys"},
        {Option::DungeonBigKeys, "Dungeon Big Keys"},
        {Option::DungeonMapsAndCompasses, "Dungeon Maps And Compasses"},
        {Option::RandomCharts, "Random Charts"},
        {Option::RandomStartIsland, "Random Start Island"},
        {Option::RandomizeDungeonEntrances, "Randomize Dungeon Entrances"},
        {Option::RandomizeBossEntrances, "Randomize Boss Entrances"},
        {Option::RandomizeMinibossEntrances, "Randomize Miniboss Entrances"},
        {Option::RandomizeCaveEntrances, "Randomize Cave Entrances"},
        {Option::RandomizeDoorEntrances, "Randomize Door Entrances"},
        {Option::RandomizeMiscEntrances, "Randomize Misc Entrances"},
        {Option::MixDungeons, "Mix Dungeons"},
        {Option::MixBosses, "Mix Bosses"},
        {Option::MixMinibosses, "Mix Minibosses"},
        {Option::MixCaves, "Mix Caves"},
        {Option::MixDoors, "Mix Doors"},
        {Option::MixMisc, "Mix Misc"},
        {Option::DecoupleEntrances, "Decouple Entrances"},
        {Option::HoHoHints, "Ho Ho Hints"},
        {Option::KorlHints, "Korl Hints"},
        {Option::ClearerHints, "Clearer Hints"},
        {Option::UseAlwaysHints, "Use Always Hints"},
        {Option::HintImportance, "Hint Importance"},
        {Option::PathHints, "Path Hints"},
        {Option::BarrenHints, "Barren Hints"},
        {Option::ItemHints, "Item Hints"},
        {Option::LocationHints, "Location Hints"},
        {Option::InstantText, "Instant Text"},
        {Option::QuietSwiftSail, "Quiet Swift Sail"},
        {Option::FixRNG, "Fix RNG"},
        {Option::Performance, "Performance"},
        {Option::RevealSeaChart, "Reveal Sea Chart"},
        {Option::AddShortcutWarps, "Add Shortcut Warps"},
        {Option::NoSpoilerLog, "No Spoiler Log"},
        {Option::RemoveSwords, "Remove Swords"},
        {Option::SkipRefights, "Skip Refights"},
        {Option::InvertCompass, "Invert Compass"},
        {Option::NumRequiredDungeons, "Num Required Dungeons"},
        {Option::DamageMultiplier, "Damage Multiplier"},
        {Option::CTMC, "CTMC"},
        {Option::PigColor, "PigColor"},
        {Option::StartingGear, "Starting Gear"},
        {Option::ExcludedLocations, "Excluded Locations"},
        {Option::StartingHP, "Starting HP"},
        {Option::StartingHC, "Starting HC"},
        {Option::StartingJoyPendants, "Starting Joy Pendants"},
        {Option::StartingSkullNecklaces, "Starting Skull Necklaces"},
        {Option::StartingBokoBabaSeeds, "Starting Boko Baba Seeds"},
        {Option::StartingGoldenFeathers, "Starting Golden Feathers"},
        {Option::StartingKnightsCrests, "Starting Knights Crests"},
        {Option::StartingRedChuJellys, "Starting Red Chu Jellys"},
        {Option::StartingGreenChuJellys, "Starting Green Chu Jellys"},
        {Option::StartingBlueChuJellys, "Starting Blue Chu Jellys"},
        {Option::RemoveMusic, "Remove Music"},
        {Option::StartWithRandomItem, "Start With Random Item"},
        {Option::RandomItemSlideItem, "Start With Random Item Sliding Item"},
        {Option::ClassicMode, "Classic Mode"},
        {Option::Plandomizer, "Plandomizer"},
        {Option::PlandomizerFile, "Plandomizer File"},
        {Option::TargetType, "Target Type"},
        {Option::Camera, "Camera"},
        {Option::FirstPersonCamera, "First-Person Camera"},
        {Option::Gyroscope, "Gyroscope"},
        {Option::UIDisplay, "UI Display"},
    };

    if (optionNameMap.contains(setting))
    {
        return optionNameMap.at(setting);
    }

    return "Invalid Option";
}

std::set<std::string> getDefaultExcludedLocations()
{
    return {
        "Mailbox - Beedle's Gold Membership Reward",
        "Mailbox - Beedle's Silver Membership Reward",
        "Outset Island - Orca Hit 500 Times",
    };
}

const std::set<std::string>& getAllLocationsNames()
{
    static std::set<std::string> locNames = {};

    // Only parse the locations file on the first call
    // The data folder is read-only for most builds, and it shouldn't be changing anyways
    if(locNames.empty()) {
        std::string locationDataStr;
        if(Utility::getFileContents(Utility::get_data_path() / "logic/location_data.yaml", locationDataStr, true) != 0) {
            ErrorLog::getInstance().log("Failed to open location_data.yaml to load location names");
            return locNames;
        }

        YAML::Node locationDataTree = YAML::Load(locationDataStr);
        for (const auto& locationObject : locationDataTree)
        {
            if (locationObject["Names"] && locationObject["Names"]["English"])
            {
                locNames.insert(locationObject["Names"]["English"].as<std::string>());
            }
            else
            {
                ErrorLog::getInstance().log("Location object missing name in location_data.yaml");
                locNames.clear();
                return locNames;
            }
        }
    }

    return locNames;
}

bool Settings::anyEntrancesShuffled() const {
    return randomize_dungeon_entrances || randomize_boss_entrances ||
            randomize_miniboss_entrances || randomize_cave_entrances != ShuffleCaveEntrances::Disabled ||
            randomize_door_entrances || randomize_misc_entrances;
}
