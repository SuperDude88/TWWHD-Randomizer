#pragma once

#include <vector>
#include <unordered_map>

#include "logic/GameItem.hpp"

enum struct PigColor : uint8_t {
    BLACK = 0,
    PINK,
    SPOTTED
};

enum struct SwordMode {
    StartWithSword = 0,
    RandomSword,
    NoSword,
    INVALID
};

enum struct Option {
    INVALID = 0,
    ProgressDungeons,
    ProgressGreatFairies,
    ProgressPuzzleCaves,
    ProgressCombatCaves,
    ProgressShortSidequests,
    ProgressLongSidequests,
    ProgressSpoilsTrading,
    ProgressMinigames,
    ProgressFreeGifts,
    ProgressMail,
    ProgressPlatformsRafts,
    ProgressSubmarines,
    ProgressEyeReefs,
    ProgressOctosGunboats,
    ProgressTriforceCharts,
    ProgressTreasureCharts,
    ProgressExpPurchases,
    ProgressMisc,
    ProgressTingleChests,
    ProgressBattlesquid,
    ProgressSavageLabyrinth,
    ProgressIslandPuzzles,
    ProgressObscure,

    Keylunacy,
    RandomCharts,
    RandomStartIsland,
    RandomizeDungeonEntrances,
    RandomizeCaveEntrances,
    MixEntrancePools,

    InstantText,
    RevealSeaChart,
    NumShards,
    AddShortcutWarps,
    NoSpoilerLog,
    SwordMode,
    SkipRefights,
    InvertCompass,
    RaceMode,
    NumRaceModeDungeons,
    DamageMultiplier,

    CasualClothes,
    PigColor,

    StartingGear,
    StartingHP,
    StartingHC,
    RemoveMusic,
    COUNT
};

struct Settings {
	  bool progression_dungeons = false;
	  bool progression_great_fairies = false;
	  bool progression_puzzle_secret_caves = false;
    bool progression_combat_secret_caves = false;
    bool progression_short_sidequests = false;
    bool progression_long_sidequests = false;
    bool progression_spoils_trading = false;
    bool progression_minigames = false;
    bool progression_free_gifts = false;
    bool progression_mail = false;
    bool progression_platforms_rafts = false;
    bool progression_submarines = false;
    bool progression_eye_reef_chests = false;
    bool progression_big_octos_gunboats = false;
    bool progression_triforce_charts = false;
    bool progression_treasure_charts = false;
    bool progression_expensive_purchases = false;
    bool progression_misc = false;
    bool progression_tingle_chests = false;
    bool progression_battlesquid = false;
    bool progression_savage_labyrinth = false;
    bool progression_island_puzzles = false;
    bool progression_obscure = false;

    bool keylunacy = false;
    bool randomize_charts = false;
    bool randomize_starting_island = false;
    bool randomize_dungeon_entrances = false;
    bool randomize_cave_entrances = false;
    bool mix_entrance_pools = false;

    bool instant_text_boxes = false;
    bool reveal_full_sea_chart = false;
    uint8_t num_starting_triforce_shards = false;
    bool add_shortcut_warps_between_dungeons = false;
    bool do_not_generate_spoiler_log = false;
    SwordMode sword_mode = SwordMode::StartWithSword;
    bool skip_rematch_bosses = false;
    bool invert_sea_compass_x_axis = false;
    bool race_mode = false;
    uint8_t num_race_mode_dungeons = false;
    float damage_multiplier = 1.0f;

    bool player_in_casual_clothes = false;
    PigColor pigColor = PigColor::BLACK;

    std::vector<GameItem> starting_gear;
    uint16_t starting_pohs = false;
    uint16_t starting_hcs = false;
    bool remove_music = false;
};

Option nameToSetting(const std::string& name);

std::string settingToName(const Option& setting);

int getSetting(const Settings& settings, const Option& option);

int evaluateOption(const Settings& settings, const std::string& optionStr);
