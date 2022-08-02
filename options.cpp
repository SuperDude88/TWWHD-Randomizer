#include "options.hpp"



static SwordMode nameToSwordMode(const std::string& name) {
    static std::unordered_map<std::string, SwordMode> nameSwordModeMap = {
        {"StartWithSword", SwordMode::StartWithSword},
        {"RandomSword", SwordMode::RandomSword},
        {"NoSword", SwordMode::NoSword},
    };

    if (nameSwordModeMap.count(name) == 0)
    {
        return SwordMode::INVALID;
    }

    return nameSwordModeMap.at(name);
}

Option nameToSetting(const std::string& name) {
    static std::unordered_map<std::string, Option> optionNameMap = {
        {"ProgressDungeons", Option::ProgressDungeons},
        {"ProgressGreatFairies", Option::ProgressGreatFairies},
        {"ProgressPuzzleCaves", Option::ProgressPuzzleCaves},
        {"ProgressCombatCaves", Option::ProgressCombatCaves},
        {"ProgressShortSidequests", Option::ProgressShortSidequests},
        {"ProgressLongSidequests", Option::ProgressLongSidequests},
        {"ProgressSpoilsTrading", Option::ProgressSpoilsTrading},
        {"ProgressMinigames", Option::ProgressMinigames},
        {"ProgressFreeGifts", Option::ProgressFreeGifts},
        {"ProgressMail", Option::ProgressMail},
        {"ProgressPlatformsRafts", Option::ProgressPlatformsRafts},
        {"ProgressSubmarines", Option::ProgressSubmarines},
        {"ProgressEyeReefs", Option::ProgressEyeReefs},
        {"ProgressOctosGunboats", Option::ProgressOctosGunboats},
        {"ProgressTriforceCharts", Option::ProgressTriforceCharts},
        {"ProgressTreasureCharts", Option::ProgressTreasureCharts},
        {"ProgressExpPurchases", Option::ProgressExpPurchases},
        {"ProgressMisc", Option::ProgressMisc},
        {"ProgressTingleChests", Option::ProgressTingleChests},
        {"ProgressBattlesquid", Option::ProgressBattlesquid},
        {"ProgressSavageLabyrinth", Option::ProgressSavageLabyrinth},
        {"ProgressIslandPuzzles", Option::ProgressIslandPuzzles},
        {"ProgressObscure", Option::ProgressObscure},
        {"Keylunacy", Option::Keylunacy},
        {"RandomCharts", Option::RandomCharts},
        {"RandomStartIsland", Option::RandomStartIsland},
        {"RandomizeDungeonEntrances", Option::RandomizeDungeonEntrances},
        {"RandomizeCaveEntrances", Option::RandomizeCaveEntrances},
        {"RandomizeDoorEntrances", Option::RandomizeDoorEntrances},
        {"RandomizeMiscEntrances", Option::RandomizeMiscEntrances},
        {"MixEntrancePools", Option::MixEntrancePools},
        {"DecoupleEntrances", Option::DecoupleEntrances},
        {"HoHoHints", Option::HoHoHints},
        {"KorlHints", Option::KorlHints},
        {"ClearerHints", Option::ClearerHints},
        {"UseAlwaysHints", Option::UseAlwaysHints},
        {"PathHints", Option::PathHints},
        {"BarrenHints", Option::BarrenHints},
        {"ItemHints", Option::ItemHints},
        {"LocationHints", Option::LocationHints},
        {"InstantText", Option::InstantText},
        {"RevealSeaChart", Option::RevealSeaChart},
        {"NumShards", Option::NumShards},
        {"AddShortcutWarps", Option::AddShortcutWarps},
        {"NoSpoilerLog", Option::NoSpoilerLog},
        {"SwordMode", Option::SwordMode},
        {"SkipRefights", Option::SkipRefights},
        {"InvertCompass", Option::InvertCompass},
        {"RaceMode", Option::RaceMode},
        {"NumRaceModeDungeons", Option::NumRaceModeDungeons},
        {"DamageMultiplier", Option::DamageMultiplier},
        {"CTMC", Option::CTMC},
        {"CasualClothes", Option::CasualClothes},
        {"PigColor", Option::PigColor},
        {"StartingGear", Option::StartingGear},
        {"StartingHP", Option::StartingHP},
        {"StartingHC", Option::StartingHC},
        {"RemoveMusic", Option::RemoveMusic},
        {"Plandomizer", Option::Plandomizer},
    };

    if (optionNameMap.count(name) == 0)
    {
        return Option::INVALID;
    }

    return optionNameMap.at(name);
}

std::string settingToName(const Option& setting) {
    static std::unordered_map<Option, std::string> optionNameMap = {
        {Option::ProgressDungeons, "ProgressDungeons"},
        {Option::ProgressGreatFairies, "ProgressGreatFairies"},
        {Option::ProgressPuzzleCaves, "ProgressPuzzleCaves"},
        {Option::ProgressCombatCaves, "ProgressCombatCaves"},
        {Option::ProgressShortSidequests, "ProgressShortSidequests"},
        {Option::ProgressLongSidequests, "ProgressLongSidequests"},
        {Option::ProgressSpoilsTrading, "ProgressSpoilsTrading"},
        {Option::ProgressMinigames, "ProgressMinigames"},
        {Option::ProgressFreeGifts, "ProgressFreeGifts"},
        {Option::ProgressMail, "ProgressMail"},
        {Option::ProgressPlatformsRafts, "ProgressPlatformsRafts"},
        {Option::ProgressSubmarines, "ProgressSubmarines"},
        {Option::ProgressEyeReefs, "ProgressEyeReefs"},
        {Option::ProgressOctosGunboats, "ProgressOctosGunboats"},
        {Option::ProgressTriforceCharts, "ProgressTriforceCharts"},
        {Option::ProgressTreasureCharts, "ProgressTreasureCharts"},
        {Option::ProgressExpPurchases, "ProgressExpPurchases"},
        {Option::ProgressMisc, "ProgressMisc"},
        {Option::ProgressTingleChests, "ProgressTingleChests"},
        {Option::ProgressBattlesquid, "ProgressBattlesquid"},
        {Option::ProgressSavageLabyrinth, "ProgressSavageLabyrinth"},
        {Option::ProgressIslandPuzzles, "ProgressIslandPuzzles"},
        {Option::ProgressObscure, "ProgressObscure"},
        {Option::Keylunacy, "Keylunacy"},
        {Option::RandomCharts, "RandomCharts"},
        {Option::RandomStartIsland, "RandomStartIsland"},
        {Option::RandomizeDungeonEntrances, "RandomizeDungeonEntrances"},
        {Option::RandomizeCaveEntrances, "RandomizeCaveEntrances"},
        {Option::RandomizeDoorEntrances, "RandomizeDoorEntrances"},
        {Option::RandomizeMiscEntrances, "RandomizeMiscEntrances"},
        {Option::MixEntrancePools, "MixEntrancePools"},
        {Option::DecoupleEntrances, "DecoupleEntrances"},
        {Option::HoHoHints, "HoHoHints"},
        {Option::KorlHints, "KorlHints"},
        {Option::ClearerHints, "ClearerHints"},
        {Option::UseAlwaysHints, "UseAlwaysHints"},
        {Option::PathHints, "PathHints"},
        {Option::BarrenHints, "BarrenHints"},
        {Option::ItemHints, "ItemHints"},
        {Option::LocationHints, "LocationHints"},
        {Option::InstantText, "InstantText"},
        {Option::RevealSeaChart, "RevealSeaChart"},
        {Option::NumShards, "NumShards"},
        {Option::AddShortcutWarps, "AddShortcutWarps"},
        {Option::NoSpoilerLog, "NoSpoilerLog"},
        {Option::SwordMode, "SwordMode"},
        {Option::SkipRefights, "SkipRefights"},
        {Option::InvertCompass, "InvertCompass"},
        {Option::RaceMode, "RaceMode"},
        {Option::NumRaceModeDungeons, "NumRaceModeDungeons"},
        {Option::DamageMultiplier, "DamageMultiplier"},
        {Option::CTMC, "CTMC"},
        {Option::CasualClothes, "CasualClothes"},
        {Option::PigColor, "PigColor"},
        {Option::StartingGear, "StartingGear"},
        {Option::StartingHP, "StartingHP"},
        {Option::StartingHC, "StartingHC"},
        {Option::RemoveMusic, "RemoveMusic"},
        {Option::Plandomizer, "Plandomizer"},
    };

    if (optionNameMap.count(setting) == 0)
    {
        return "Invalid Option";
    }

    return optionNameMap.at(setting);
}

int getSetting(const Settings& settings, const Option& option) {

	switch (option) {
    case Option::ProgressDungeons:
        return settings.progression_dungeons;
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
    case Option::ProgressObscure:
        return settings.progression_obscure;
    case Option::Keylunacy:
        return settings.keylunacy;
    case Option::RandomCharts:
        return settings.randomize_charts;
    case Option::RandomStartIsland:
        return settings.randomize_starting_island;
    case Option::RandomizeDungeonEntrances:
        return settings.randomize_dungeon_entrances;
    case Option::RandomizeCaveEntrances:
        return settings.randomize_cave_entrances;
    case Option::RandomizeDoorEntrances:
        return settings.randomize_door_entrances;
    case Option::RandomizeMiscEntrances:
        return settings.randomize_misc_entrances;
    case Option::MixEntrancePools:
        return settings.mix_entrance_pools;
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
    case Option::RaceMode:
        return settings.race_mode;
    case Option::NumRaceModeDungeons:
        return settings.num_race_mode_dungeons;
    case Option::DamageMultiplier:
        return settings.damage_multiplier;
    case Option::CTMC:
        return settings.chest_type_matches_contents;
    case Option::CasualClothes:
        return settings.player_in_casual_clothes;
    case Option::PigColor:
        return static_cast<std::underlying_type_t<PigColor>>(settings.pig_color);
    case Option::StartingGear: //cant return this like everything else, just here as placeholder
        return 0;
    case Option::StartingHP:
        return settings.starting_pohs;
    case Option::StartingHC:
        return settings.starting_hcs;
    case Option::RemoveMusic:
        return settings.remove_music;
    case Option::Plandomizer:
        return settings.plandomizer;
    default:
        return 0;
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
