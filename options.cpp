#include "options.hpp"

SwordMode nameToSwordMode(const std::string& name) {
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

EntranceRando nameToEntranceRando(const std::string& name) {
    static std::unordered_map<std::string, EntranceRando> nameEntranceRandoMap = {
        {"None", EntranceRando::None},
        {"Dungeons", EntranceRando::Dungeons},
        {"Caves", EntranceRando::Caves},
        {"DungeonsAndCaves", EntranceRando::DungeonsAndCaves},
        {"DungeonsWithCaves", EntranceRando::DungeonsWithCaves},
    };

    if (nameEntranceRandoMap.count(name) == 0)
    {
        return EntranceRando::INVALID;
    }

    return nameEntranceRandoMap.at(name);
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
        {"RandomEntrances", Option::RandomEntrances},
        {"RandomCharts", Option::RandomCharts},
        {"RandomStartIsland", Option::RandomStartIsland},
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
        {"CasualClothes", Option::CasualClothes},
        {"PigColor", Option::PigColor},
        {"StartingGear", Option::StartingGear},
        {"StartingHP", Option::StartingHP},
        {"StartingHC", Option::StartingHC},
        {"RemoveMusic", Option::RemoveMusic}
    };

    if (optionNameMap.count(name) == 0)
    {
        return Option::INVALID;
    }

    return optionNameMap.at(name);
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
    case Option::RandomEntrances:
        return static_cast<std::underlying_type_t<EntranceRando>>(settings.randomize_entrances);
    case Option::RandomCharts:
        return settings.randomize_charts;
    case Option::RandomStartIsland:
        return settings.randomize_starting_island;
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
    case Option::CasualClothes:
        return settings.player_in_casual_clothes;
    case Option::PigColor:
        return static_cast<std::underlying_type_t<PigColor>>(settings.pigColor);
    case Option::StartingGear: //cant return this like everything else, just here as placeholder
        return 0;
    case Option::StartingHP:
        return settings.starting_pohs;
    case Option::StartingHC:
        return settings.starting_hcs;
    case Option::RemoveMusic:
        return settings.remove_music;
    default:
        return 0;
	}

}

int evaluateOption(const Settings& settings, const std::string& optionStr) {
    if (nameToSwordMode(optionStr) != SwordMode::INVALID)
    {
        return getSetting(settings, Option::SwordMode) == static_cast<int>(nameToSwordMode(optionStr));
    }
    else if (nameToEntranceRando(optionStr) != EntranceRando::INVALID)
    {
        return getSetting(settings, Option::RandomEntrances) == static_cast<int>(nameToEntranceRando(optionStr));
    }
    else if (nameToSetting(optionStr) != Option::INVALID)
    {
        return getSetting(settings, nameToSetting(optionStr));
    }

    // -1 means that the setting doesn't exist
    return -1;
}
