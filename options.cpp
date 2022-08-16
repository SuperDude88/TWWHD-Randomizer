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
        {"PlandomizerFile", Option::PlandomizerFile},
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
        {Option::PlandomizerFile, "PlandomizerFile"},
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
    case Option::PlandomizerFile: //cant return this like everything else, just here as placeholder
        return 0;
    default:
        return 0;
	}

}

void setSetting(Settings& settings, const Option& option, const size_t& value)
{
  switch (option) {
    case Option::ProgressDungeons:
        settings.progression_dungeons = value; return;
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
    case Option::ProgressObscure:
        settings.progression_obscure = value; return;
    case Option::Keylunacy:
        settings.keylunacy = value; return;
    case Option::RandomCharts:
        settings.randomize_charts = value; return;
    case Option::RandomStartIsland:
        settings.randomize_starting_island = value; return;
    case Option::RandomizeDungeonEntrances:
        settings.randomize_dungeon_entrances = value; return;
    case Option::RandomizeCaveEntrances:
        settings.randomize_cave_entrances = value; return;
    case Option::RandomizeDoorEntrances:
        settings.randomize_door_entrances = value; return;
    case Option::RandomizeMiscEntrances:
        settings.randomize_misc_entrances = value; return;
    case Option::MixEntrancePools:
        settings.mix_entrance_pools = value; return;
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
    case Option::RaceMode:
        settings.race_mode = value; return;
    case Option::NumRaceModeDungeons:
        settings.num_race_mode_dungeons = value; return;
    case Option::DamageMultiplier:
        settings.damage_multiplier = value; return;
    case Option::CTMC:
        settings.chest_type_matches_contents = value; return;
    case Option::CasualClothes:
        settings.player_in_casual_clothes = value; return;
    case Option::PigColor:
        settings.pig_color = static_cast<PigColor>(value); return;
    case Option::StartingGear: //cant set this like everything else, just here as placeholder
        return;
    case Option::StartingHP:
        settings.starting_pohs = value; return;
    case Option::StartingHC:
        settings.starting_hcs = value; return;
    case Option::RemoveMusic:
        settings.remove_music = value; return;
    case Option::Plandomizer:
        settings.plandomizer = value; return;
    case Option::PlandomizerFile: //cant set this like everything else, just here as placeholder
        return;
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
