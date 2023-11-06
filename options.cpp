#include "options.hpp"

static const std::unordered_map<std::string, SwordMode> nameSwordModeMap = {
    {"Start With Sword", SwordMode::StartWithSword},
    {"Random Sword", SwordMode::RandomSword},
    {"No Sword", SwordMode::NoSword},
};

static const std::unordered_map<SwordMode, std::string> swordModeNameMap = {
    {SwordMode::StartWithSword, "Start With Sword"},
    {SwordMode::RandomSword, "Random Sword"},
    {SwordMode::NoSword, "No Sword"}
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


SwordMode nameToSwordMode(const std::string& name) {

    if (nameSwordModeMap.count(name) == 0)
    {
        return SwordMode::INVALID;
    }

    return nameSwordModeMap.at(name);
}

std::string SwordModeToName(const SwordMode& mode) {

    if (swordModeNameMap.count(mode) == 0)
    {
        return "INVALID";
    }

    return swordModeNameMap.at(mode);
}

PigColor nameToPigColor(const std::string& name) {

    if (namePigColorMap.count(name) == 0)
    {
        return PigColor::INVALID;
    }

    return namePigColorMap.at(name);
}

std::string PigColorToName(const PigColor& name) {

    if (pigColorNameMap.count(name) == 0)
    {
        return "INVALID";
    }

    return pigColorNameMap.at(name);
}

PlacementOption nameToPlacementOption(const std::string& name) {
    if (namePlacementOptionMap.count(name) == 0)
    {
        return PlacementOption::INVALID;
    }

    return namePlacementOptionMap.at(name);
}

std::string PlacementOptionToName(const PlacementOption& option) {
    if (placementOptionNameMap.count(option) == 0)
    {
        return "INVALID";
    }

    return placementOptionNameMap.at(option);
}

ProgressionDungeons nameToProgressionDungeons(const std::string& name) {
    if (nameProgressionDungeonsMap.count(name) == 0)
    {
        return ProgressionDungeons::INVALID;
    }

    return nameProgressionDungeonsMap.at(name);
}

std::string ProgressionDungeonsToName(const ProgressionDungeons& option) {
    if (progressionDungeonsNameMap.count(option) == 0)
    {
        return "INVALID";
    }

    return progressionDungeonsNameMap.at(option);
}

TargetTypePreference nameToTargetTypePreference(const std::string& name)
{
    if (nameTargetTypePreferenceMap.count(name) == 0)
    {
        return TargetTypePreference::INVALID;
    }

    return nameTargetTypePreferenceMap.at(name);
}

std::string TargetTypePreferenceToName(const TargetTypePreference& preference)
{
    if (targetTypePreferenceNameMap.count(preference) == 0)
    {
        return "INVALID";
    }

    return targetTypePreferenceNameMap.at(preference);
}

CameraPreference nameToCameraPreference(const std::string& name)
{
    if (nameCameraPreferenceMap.count(name) == 0)
    {
        return CameraPreference::INVALID;
    }

    return nameCameraPreferenceMap.at(name);
}

std::string CameraPreferenceToName(const CameraPreference& preference)
{
    if (cameraPreferenceNameMap.count(preference) == 0)
    {
        return "INVALID";
    }

    return cameraPreferenceNameMap.at(preference);
}

FirstPersonCameraPreference nameToFirstPersonCameraPreference(const std::string& name)
{
    if (nameFirstPersonCameraPreferenceMap.count(name) == 0)
    {
        return FirstPersonCameraPreference::INVALID;
    }

    return nameFirstPersonCameraPreferenceMap.at(name);
}

std::string FirstPersonCameraPreferenceToName(const FirstPersonCameraPreference& preference)
{
    if (firstPersonCameraPreferenceNameMap.count(preference) == 0)
    {
        return "INVALID";
    }

    return firstPersonCameraPreferenceNameMap.at(preference);
}

GyroscopePreference nameToGyroscopePreference(const std::string& name)
{
    if (nameGyroscopePreferenceMap.count(name) == 0)
    {
        return GyroscopePreference::INVALID;
    }

    return nameGyroscopePreferenceMap.at(name);
}

std::string GyroscopePreferenceToName(const GyroscopePreference& preference)
{
    if (gyroscopePreferenceNameMap.count(preference) == 0)
    {
        return "INVALID";
    }

    return gyroscopePreferenceNameMap.at(preference);
}

UIDisplayPreference nameToUIDisplayPreference(const std::string& name)
{
    if (nameUIDisplayPreferenceMap.count(name) == 0)
    {
        return UIDisplayPreference::INVALID;
    }

    return nameUIDisplayPreferenceMap.at(name);
}

std::string UIDisplayPreferenceToName(const UIDisplayPreference& preference)
{
    if (uiDisplayPreferenceNameMap.count(preference) == 0)
    {
        return "INVALID";
    }

    return uiDisplayPreferenceNameMap.at(preference);
}

// Make sure there aren't any naming conflicts when adding future settings
int nameToSettingInt(const std::string& name) {
    if (nameSwordModeMap.count(name) > 0)
    {
        return static_cast<std::underlying_type_t<SwordMode>>(nameSwordModeMap.at(name));
    }
    if (namePigColorMap.count(name) > 0)
    {
        return static_cast<std::underlying_type_t<PigColor>>(namePigColorMap.at(name));
    }
    if (nameTargetTypePreferenceMap.count(name) > 0)
    {
        return static_cast<std::underlying_type_t<TargetTypePreference>>(nameTargetTypePreferenceMap.at(name));
    }
    if (nameCameraPreferenceMap.count(name) > 0)
    {
        return static_cast<std::underlying_type_t<CameraPreference>>(nameCameraPreferenceMap.at(name));
    }
    if (nameFirstPersonCameraPreferenceMap.count(name) > 0)
    {
        return static_cast<std::underlying_type_t<FirstPersonCameraPreference>>(nameFirstPersonCameraPreferenceMap.at(name));
    }
    if (nameGyroscopePreferenceMap.count(name) > 0)
    {
        return static_cast<std::underlying_type_t<GyroscopePreference>>(nameGyroscopePreferenceMap.at(name));
    }
    if (nameUIDisplayPreferenceMap.count(name) > 0)
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
        {"Path ints", Option::PathHints},
        {"Barren Hints", Option::BarrenHints},
        {"Item Hints", Option::ItemHints},
        {"Location Hints", Option::LocationHints},
        {"Instant Text", Option::InstantText},
        {"Fix RNG", Option::FixRNG},
        {"Performance", Option::Performance},
        {"Reveal Sea Chart", Option::RevealSeaChart},
        {"Num Shards", Option::NumShards},
        {"Add Shortcut Warps", Option::AddShortcutWarps},
        {"No Spoiler Log", Option::NoSpoilerLog},
        {"Sword Mode", Option::SwordMode},
        {"Skip Refights", Option::SkipRefights},
        {"Invert Compass", Option::InvertCompass},
        {"Num Required Dungeons", Option::NumRequiredDungeons},
        {"Damage Multiplier", Option::DamageMultiplier},
        {"CTMC", Option::CTMC},
        {"Pig Color", Option::PigColor},
        {"Starting Gear", Option::StartingGear},
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
        {"Plandomizer", Option::Plandomizer},
        {"Plandomizer File", Option::PlandomizerFile},
        {"Target Type", Option::TargetType},
        {"Camera", Option::Camera},
        {"First-Person Camera", Option::FirstPersonCamera},
        {"Gyroscope", Option::Gyroscope},
        {"UI Display", Option::UIDisplay},
    };

    if (optionNameMap.count(name) == 0)
    {
        return Option::INVALID;
    }

    return optionNameMap.at(name);
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
        {Option::PathHints, "Path Hints"},
        {Option::BarrenHints, "Barren Hints"},
        {Option::ItemHints, "Item Hints"},
        {Option::LocationHints, "Location Hints"},
        {Option::InstantText, "Instant Text"},
        {Option::FixRNG, "Fix RNG"},
        {Option::Performance, "Performance"},
        {Option::RevealSeaChart, "Reveal Sea Chart"},
        {Option::NumShards, "Num Shards"},
        {Option::AddShortcutWarps, "Add Shortcut Warps"},
        {Option::NoSpoilerLog, "No Spoiler Log"},
        {Option::SwordMode, "Sword Mode"},
        {Option::SkipRefights, "Skip Refights"},
        {Option::InvertCompass, "Invert Compass"},
        {Option::NumRequiredDungeons, "Num Required Dungeons"},
        {Option::DamageMultiplier, "Damage Multiplier"},
        {Option::CTMC, "CTMC"},
        {Option::PigColor, "PigColor"},
        {Option::StartingGear, "Starting Gear"},
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
        {Option::Plandomizer, "Plandomizer"},
        {Option::PlandomizerFile, "Plandomizer File"},
        {Option::TargetType, "Target Type"},
        {Option::Camera, "Camera"},
        {Option::FirstPersonCamera, "First-Person Camera"},
        {Option::Gyroscope, "Gyroscope"},
        {Option::UIDisplay, "UI Display"},
    };

    if (optionNameMap.count(setting) == 0)
    {
        return "Invalid Option";
    }

    return optionNameMap.at(setting);
}

uint8_t getSetting(const Settings& settings, const Option& option) {

    switch (option) {
        case Option::ProgressDungeons:
            return static_cast<std::underlying_type_t<ProgressionDungeons>>(settings.progression_dungeons);
        case Option::ProgressGreatFairies:
            return settings.progression_great_fairies;
        case Option::ProgressPuzzleCaves:
            return settings.progression_puzzle_secret_caves;
        case Option::ProgressCombatCaves:
            return settings.progression_combat_secret_caves;
        case Option::ProgressShortSidequests:
            return settings.progression_short_sidequests;
        case Option::ProgressLongSidequests:
            return settings.progression_long_sidequests;
        case Option::ProgressSpoilsTrading:
            return settings.progression_spoils_trading;
        case Option::ProgressMinigames:
            return settings.progression_minigames;
        case Option::ProgressFreeGifts:
            return settings.progression_free_gifts;
        case Option::ProgressMail:
            return settings.progression_mail;
        case Option::ProgressPlatformsRafts:
            return settings.progression_platforms_rafts;
        case Option::ProgressSubmarines:
            return settings.progression_submarines;
        case Option::ProgressEyeReefs:
            return settings.progression_eye_reef_chests;
        case Option::ProgressOctosGunboats:
            return settings.progression_big_octos_gunboats;
        case Option::ProgressTriforceCharts:
            return settings.progression_triforce_charts;
        case Option::ProgressTreasureCharts:
            return settings.progression_treasure_charts;
        case Option::ProgressExpPurchases:
            return settings.progression_expensive_purchases;
        case Option::ProgressMisc:
            return settings.progression_misc;
        case Option::ProgressTingleChests:
            return settings.progression_tingle_chests;
        case Option::ProgressBattlesquid:
            return settings.progression_battlesquid;
        case Option::ProgressSavageLabyrinth:
            return settings.progression_savage_labyrinth;
        case Option::ProgressIslandPuzzles:
            return settings.progression_island_puzzles;
        case Option::ProgressDungeonSecrets:
            return settings.progression_dungeon_secrets;
        case Option::ProgressObscure:
            return settings.progression_obscure;
        case Option::DungeonSmallKeys:
            return static_cast<std::underlying_type_t<PlacementOption>>(settings.dungeon_small_keys);
        case Option::DungeonBigKeys:
            return static_cast<std::underlying_type_t<PlacementOption>>(settings.dungeon_big_keys);
        case Option::DungeonMapsAndCompasses:
            return static_cast<std::underlying_type_t<PlacementOption>>(settings.dungeon_maps_compasses);
        case Option::RandomCharts:
            return settings.randomize_charts;
        case Option::RandomStartIsland:
            return settings.randomize_starting_island;
        case Option::RandomizeDungeonEntrances:
            return settings.randomize_dungeon_entrances;
        case Option::RandomizeMinibossEntrances:
            return settings.randomize_miniboss_entrances;
        case Option::RandomizeBossEntrances:
            return settings.randomize_boss_entrances;
        case Option::RandomizeCaveEntrances:
            return settings.randomize_cave_entrances;
        case Option::RandomizeDoorEntrances:
            return settings.randomize_door_entrances;
        case Option::RandomizeMiscEntrances:
            return settings.randomize_misc_entrances;
        case Option::MixDungeons:
            return settings.mix_dungeons;
        case Option::MixBosses:
            return settings.mix_bosses;
        case Option::MixMinibosses:
            return settings.mix_minibosses;
        case Option::MixCaves:
            return settings.mix_caves;
        case Option::MixDoors:
            return settings.mix_doors;
        case Option::MixMisc:
            return settings.mix_misc;
        case Option::DecoupleEntrances:
            return settings.decouple_entrances;
        case Option::HoHoHints:
            return settings.ho_ho_hints;
        case Option::KorlHints:
            return settings.korl_hints;
        case Option::ClearerHints:
            return settings.clearer_hints;
        case Option::UseAlwaysHints:
            return settings.use_always_hints;
        case Option::PathHints:
            return settings.path_hints;
        case Option::BarrenHints:
            return settings.barren_hints;
        case Option::ItemHints:
            return settings.item_hints;
        case Option::LocationHints:
            return settings.location_hints;
        case Option::InstantText:
            return settings.instant_text_boxes;
        case Option::FixRNG:
            return settings.fix_rng;
        case Option::Performance:
            return settings.performance;
        case Option::RevealSeaChart:
            return settings.reveal_full_sea_chart;
        case Option::NumShards:
            return settings.num_starting_triforce_shards;
        case Option::AddShortcutWarps:
            return settings.add_shortcut_warps_between_dungeons;
        case Option::NoSpoilerLog:
            return settings.do_not_generate_spoiler_log;
        case Option::SwordMode:
            return static_cast<std::underlying_type_t<SwordMode>>(settings.sword_mode);
        case Option::SkipRefights:
            return settings.skip_rematch_bosses;
        case Option::InvertCompass:
            return settings.invert_sea_compass_x_axis;
        case Option::NumRequiredDungeons:
            return settings.num_required_dungeons;
        case Option::DamageMultiplier:
            return static_cast<uint8_t>(settings.damage_multiplier);
        case Option::CTMC:
            return settings.chest_type_matches_contents;
        case Option::PigColor:
            return static_cast<std::underlying_type_t<PigColor>>(settings.pig_color);
        case Option::StartingGear: //cant return this like everything else, just here as placeholder
            return 0;
        case Option::StartingHP:
            return settings.starting_pohs;
        case Option::StartingHC:
            return settings.starting_hcs;
        case Option::StartingJoyPendants:
            return settings.starting_joy_pendants;
        case Option::StartingSkullNecklaces:
            return settings.starting_skull_necklaces;
        case Option::StartingBokoBabaSeeds:
            return settings.starting_boko_baba_seeds;
        case Option::StartingGoldenFeathers:
            return settings.starting_golden_feathers;
        case Option::StartingKnightsCrests:
            return settings.starting_knights_crests;
        case Option::StartingRedChuJellys:
            return settings.starting_red_chu_jellys;
        case Option::StartingGreenChuJellys:
            return settings.starting_green_chu_jellys;
        case Option::StartingBlueChuJellys:
            return settings.starting_blue_chu_jellys;
        case Option::RemoveMusic:
            return settings.remove_music;
        case Option::StartWithRandomItem:
            return settings.start_with_random_item;
        case Option::Plandomizer:
            return settings.plandomizer;
        case Option::PlandomizerFile: //cant return this like everything else, just here as placeholder
            return 0;
        case Option::TargetType:
            return static_cast<std::underlying_type_t<TargetTypePreference>>(settings.target_type);   
        case Option::Camera:
            return static_cast<std::underlying_type_t<CameraPreference>>(settings.camera);
        case Option::FirstPersonCamera:
            return static_cast<std::underlying_type_t<FirstPersonCameraPreference>>(settings.first_person_camera);
        case Option::Gyroscope:
            return static_cast<std::underlying_type_t<GyroscopePreference>>(settings.gyroscope);
        case Option::UIDisplay:
            return static_cast<std::underlying_type_t<UIDisplayPreference>>(settings.ui_display);
        default:
            return 0;
    }

}

void setSetting(Settings& settings, const Option& option, const size_t& value)
{
    switch (option) {
        case Option::ProgressDungeons:
            settings.progression_dungeons = static_cast<ProgressionDungeons>(value); return;
        case Option::ProgressGreatFairies:
            settings.progression_great_fairies = value; return;
        case Option::ProgressPuzzleCaves:
            settings.progression_puzzle_secret_caves = value; return;
        case Option::ProgressCombatCaves:
            settings.progression_combat_secret_caves = value; return;
        case Option::ProgressShortSidequests:
            settings.progression_short_sidequests = value; return;
        case Option::ProgressLongSidequests:
            settings.progression_long_sidequests = value; return;
        case Option::ProgressSpoilsTrading:
            settings.progression_spoils_trading = value; return;
        case Option::ProgressMinigames:
            settings.progression_minigames = value; return;
        case Option::ProgressFreeGifts:
            settings.progression_free_gifts = value; return;
        case Option::ProgressMail:
            settings.progression_mail = value; return;
        case Option::ProgressPlatformsRafts:
            settings.progression_platforms_rafts = value; return;
        case Option::ProgressSubmarines:
            settings.progression_submarines = value; return;
        case Option::ProgressEyeReefs:
            settings.progression_eye_reef_chests = value; return;
        case Option::ProgressOctosGunboats:
            settings.progression_big_octos_gunboats = value; return;
        case Option::ProgressTriforceCharts:
            settings.progression_triforce_charts = value; return;
        case Option::ProgressTreasureCharts:
            settings.progression_treasure_charts = value; return;
        case Option::ProgressExpPurchases:
            settings.progression_expensive_purchases = value; return;
        case Option::ProgressMisc:
            settings.progression_misc = value; return;
        case Option::ProgressTingleChests:
            settings.progression_tingle_chests = value; return;
        case Option::ProgressBattlesquid:
            settings.progression_battlesquid = value; return;
        case Option::ProgressSavageLabyrinth:
            settings.progression_savage_labyrinth = value; return;
        case Option::ProgressIslandPuzzles:
            settings.progression_island_puzzles = value; return;
        case Option::ProgressDungeonSecrets:
            settings.progression_dungeon_secrets = value; return;
        case Option::ProgressObscure:
            settings.progression_obscure = value; return;
        case Option::DungeonSmallKeys:
            settings.dungeon_small_keys = static_cast<PlacementOption>(value); return;
        case Option::DungeonBigKeys:
            settings.dungeon_big_keys = static_cast<PlacementOption>(value); return;
        case Option::DungeonMapsAndCompasses:
            settings.dungeon_maps_compasses = static_cast<PlacementOption>(value); return;
        case Option::RandomCharts:
            settings.randomize_charts = value; return;
        case Option::RandomStartIsland:
            settings.randomize_starting_island = value; return;
        case Option::RandomizeDungeonEntrances:
            settings.randomize_dungeon_entrances = value; return;
        case Option::RandomizeBossEntrances:
            settings.randomize_boss_entrances = value; return;
        case Option::RandomizeMinibossEntrances:
            settings.randomize_miniboss_entrances = value; return;
        case Option::RandomizeCaveEntrances:
            settings.randomize_cave_entrances = value; return;
        case Option::RandomizeDoorEntrances:
            settings.randomize_door_entrances = value; return;
        case Option::RandomizeMiscEntrances:
            settings.randomize_misc_entrances = value; return;
        case Option::MixDungeons:
            settings.mix_dungeons = value; return;
        case Option::MixBosses:
            settings.mix_bosses = value; return;
        case Option::MixMinibosses:
            settings.mix_minibosses = value; return;
        case Option::MixCaves:
            settings.mix_caves = value; return;
        case Option::MixDoors:
            settings.mix_doors = value; return;
        case Option::MixMisc:
            settings.mix_misc = value; return;
        case Option::DecoupleEntrances:
            settings.decouple_entrances = value; return;
        case Option::HoHoHints:
            settings.ho_ho_hints = value; return;
        case Option::KorlHints:
            settings.korl_hints = value; return;
        case Option::ClearerHints:
            settings.clearer_hints = value; return;
        case Option::UseAlwaysHints:
            settings.use_always_hints = value; return;
        case Option::PathHints:
            settings.path_hints = value; return;
        case Option::BarrenHints:
            settings.barren_hints = value; return;
        case Option::ItemHints:
            settings.item_hints = value; return;
        case Option::LocationHints:
            settings.location_hints = value; return;
        case Option::InstantText:
            settings.instant_text_boxes = value; return;
        case Option::FixRNG:
            settings.fix_rng = value; return;
        case Option::Performance:
            settings.performance = value; return;
        case Option::RevealSeaChart:
            settings.reveal_full_sea_chart = value; return;
        case Option::NumShards:
            settings.num_starting_triforce_shards = value; return;
        case Option::AddShortcutWarps:
            settings.add_shortcut_warps_between_dungeons = value; return;
        case Option::NoSpoilerLog:
            settings.do_not_generate_spoiler_log = value; return;
        case Option::SwordMode:
            settings.sword_mode = static_cast<SwordMode>(value); return;
        case Option::SkipRefights:
            settings.skip_rematch_bosses = value; return;
        case Option::InvertCompass:
            settings.invert_sea_compass_x_axis = value; return;
        case Option::NumRequiredDungeons:
            settings.num_required_dungeons = value; return;
        case Option::DamageMultiplier:
            settings.damage_multiplier = value; return;
        case Option::CTMC:
            settings.chest_type_matches_contents = value; return;
        case Option::PigColor:
            settings.pig_color = static_cast<PigColor>(value); return;
        case Option::StartingGear: //cant set this like everything else, just here as placeholder
            return;
        case Option::StartingHP:
            settings.starting_pohs = value; return;
        case Option::StartingHC:
            settings.starting_hcs = value; return;
        case Option::StartingJoyPendants:
            settings.starting_joy_pendants = value; return;
        case Option::StartingSkullNecklaces:
            settings.starting_skull_necklaces = value; return;
        case Option::StartingBokoBabaSeeds:
            settings.starting_boko_baba_seeds = value; return;
        case Option::StartingGoldenFeathers:
            settings.starting_golden_feathers = value; return;
        case Option::StartingKnightsCrests:
            settings.starting_knights_crests = value; return;
        case Option::StartingRedChuJellys:
            settings.starting_red_chu_jellys = value; return;
        case Option::StartingGreenChuJellys:
            settings.starting_green_chu_jellys = value; return;
        case Option::StartingBlueChuJellys:
            settings.starting_blue_chu_jellys = value; return;
        case Option::RemoveMusic:
            settings.remove_music = value; return;
        case Option::StartWithRandomItem:
            settings.start_with_random_item = value; return;
        case Option::Plandomizer:
            settings.plandomizer = value; return;
        case Option::PlandomizerFile: //cant set this like everything else, just here as placeholder
            return;
        case Option::TargetType:
            settings.target_type = static_cast<TargetTypePreference>(value); return;
        case Option::Camera:
            settings.camera = static_cast<CameraPreference>(value); return;
        case Option::FirstPersonCamera:
            settings.first_person_camera = static_cast<FirstPersonCameraPreference>(value); return;
        case Option::Gyroscope:
            settings.gyroscope = static_cast<GyroscopePreference>(value); return;
        case Option::UIDisplay:
            settings.ui_display = static_cast<UIDisplayPreference>(value); return;
        default:
            return;
    }
}

int evaluateOption(const Settings& settings, const std::string& optionStr) {
    if (nameToSwordMode(optionStr) != SwordMode::INVALID)
    {
        return settings.sword_mode == nameToSwordMode(optionStr);
    }
    else if (nameToSetting(optionStr) != Option::INVALID)
    {
        return getSetting(settings, nameToSetting(optionStr));
    }

    // -1 means that the setting doesn't exist
    return -1;
}
