#pragma once

#include <vector>
#include <set>

#include <logic/GameItem.hpp>
#include <customizer/model.hpp>

constexpr uint16_t MAXIMUM_STARTING_HC = 9;
constexpr uint16_t MAXIMUM_STARTING_HP = 44;
constexpr uint16_t MAXIMUM_STARTING_JOY_PENDANTS = 40;
constexpr uint16_t MAXIMUM_STARTING_SKULL_NECKLACES = 23;
constexpr uint16_t MAXIMUM_STARTING_BOKO_BABA_SEEDS = 10;
constexpr uint16_t MAXIMUM_STARTING_GOLDEN_FEATHERS = 20;
constexpr uint16_t MAXIMUM_STARTING_KNIGHTS_CRESTS = 10;
constexpr uint16_t MAXIMUM_STARTING_RED_CHU_JELLYS = 15;
constexpr uint16_t MAXIMUM_STARTING_GREEN_CHU_JELLYS = 15;
constexpr uint16_t MAXIMUM_STARTING_BLUE_CHU_JELLYS = 15;

constexpr uint8_t MAXIMUM_NUM_DUNGEONS = 6;
constexpr float MAXIMUM_DAMAGE_MULTIPLIER = 80.0f;

constexpr uint8_t MAXIMUM_PATH_HINT_COUNT = 7;
constexpr uint8_t MAXIMUM_BARREN_HINT_COUNT = 7;
constexpr uint8_t MAXIMUM_ITEM_HINT_COUNT = 7;
constexpr uint8_t MAXIMUM_LOCATION_HINT_COUNT = 7;

enum struct PigColor : uint8_t {
    Black = 0,
    Pink,
    Spotted,
    Random,
    INVALID
};

enum struct PlacementOption {
    Vanilla = 0,
    OwnDungeon,
    AnyDungeon,
    Overworld,
    Keysanity,
    INVALID
};

enum struct ProgressionDungeons {
    Disabled = 0,
    Standard,
    RaceMode,
    INVALID
};

enum struct ShuffleCaveEntrances {
    Disabled = 0,
    Caves,
    CavesFairies,
    INVALID
};

enum struct TargetTypePreference {
    Hold = 0,
    Switch,
    INVALID,
};

enum struct CameraPreference {
    Standard = 0,
    ReverseLeftRight,
    INVALID,
};

enum struct FirstPersonCameraPreference {
    Standard = 0,
    ReverseUpDown,
    INVALID,
};

enum struct GyroscopePreference {
    Off = 0,
    On,
    INVALID,
};

enum struct UIDisplayPreference {
    On = 0,
    Off,
    INVALID,
};

// internal for now, here so Crain can do what he wants with it
enum struct GameVersion {
    HD = 0,
    SD,
    INVALID
};

enum struct Option {
    INVALID = 0,

    // Internal
    GameVersion,

    // Progression
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
    ProgressDungeonSecrets,
    ProgressObscure,

    // Additional Randomization Options
    RemoveSwords,
    NumRequiredDungeons,
    RandomCharts,
    CTMC,

    // Dungeon Randomization Options
    DungeonSmallKeys,
    DungeonBigKeys,
    DungeonMapsAndCompasses,

    // Convenince Tweaks
    InvertCompass,
    InstantText,
    QuietSwiftSail,
    FixRNG,
    Performance,
    RevealSeaChart,
    SkipRefights,
    AddShortcutWarps,
    RemoveMusic,

    // Starting Gear
    StartingGear,
    StartingHP,
    StartingHC,
    StartingJoyPendants,
    StartingSkullNecklaces,
    StartingBokoBabaSeeds,
    StartingGoldenFeathers,
    StartingKnightsCrests,
    StartingRedChuJellys,
    StartingGreenChuJellys,
    StartingBlueChuJellys,

    // Excluded Locations
    ExcludedLocations,

    // Advanced Options
    NoSpoilerLog,
    StartWithRandomItem,
    RandomItemSlideItem,
    ClassicMode,
    Plandomizer,
    PlandomizerFile,

    // Hints
    HoHoHints,
    KorlHints,
    ClearerHints,
    UseAlwaysHints,
    HintImportance,
    PathHints,
    BarrenHints,
    ItemHints,
    LocationHints,

    // Entrance Randomizer
    RandomizeDungeonEntrances,
    RandomizeBossEntrances,
    RandomizeMinibossEntrances,
    RandomizeCaveEntrances,
    RandomizeDoorEntrances,
    RandomizeMiscEntrances,
    MixDungeons,
    MixBosses,
    MixMinibosses,
    MixCaves,
    MixDoors,
    MixMisc,
    DecoupleEntrances,
    RandomStartIsland,

    // Cosmetics
    PigColor,


    DamageMultiplier,

    // In game preference
    TargetType,
    Camera,
    FirstPersonCamera,
    Gyroscope,
    UIDisplay,

    COUNT
};

class Settings {
public:
    GameVersion game_version;

    ProgressionDungeons progression_dungeons;
    bool progression_great_fairies;
    bool progression_puzzle_secret_caves;
    bool progression_combat_secret_caves;
    bool progression_short_sidequests;
    bool progression_long_sidequests;
    bool progression_spoils_trading;
    bool progression_minigames;
    bool progression_free_gifts;
    bool progression_mail;
    bool progression_platforms_rafts;
    bool progression_submarines;
    bool progression_eye_reef_chests;
    bool progression_big_octos_gunboats;
    bool progression_triforce_charts;
    bool progression_treasure_charts;
    bool progression_expensive_purchases;
    bool progression_misc;
    bool progression_tingle_chests;
    bool progression_battlesquid;
    bool progression_savage_labyrinth;
    bool progression_island_puzzles;
    bool progression_dungeon_secrets;
    bool progression_obscure;

    PlacementOption dungeon_small_keys;
    PlacementOption dungeon_big_keys;
    PlacementOption dungeon_maps_compasses;
    bool randomize_charts;
    bool randomize_starting_island;
    bool randomize_dungeon_entrances;
    bool randomize_boss_entrances;
    bool randomize_miniboss_entrances;
    ShuffleCaveEntrances randomize_cave_entrances;
    bool randomize_door_entrances;
    bool randomize_misc_entrances;
    bool mix_dungeons;
    bool mix_bosses;
    bool mix_minibosses;
    bool mix_caves;
    bool mix_doors;
    bool mix_misc;
    bool decouple_entrances;

    bool ho_ho_hints;
    bool korl_hints;
    bool clearer_hints;
    bool use_always_hints;
    bool hint_importance;
    uint8_t path_hints;
    uint8_t barren_hints;
    uint8_t item_hints;
    uint8_t location_hints;

    bool instant_text_boxes;
    bool quiet_swift_sail;
    bool fix_rng;
    bool performance;
    bool reveal_full_sea_chart;
    bool add_shortcut_warps_between_dungeons;
    bool do_not_generate_spoiler_log;
    bool remove_swords;
    bool skip_rematch_bosses;
    bool invert_sea_compass_x_axis;
    uint8_t num_required_dungeons;
    float damage_multiplier;
    bool chest_type_matches_contents;

    PigColor pig_color;

    std::vector<GameItem> starting_gear;
    std::set<std::string> excluded_locations;
    uint16_t starting_pohs;
    uint16_t starting_hcs;
    uint16_t starting_joy_pendants;
    uint16_t starting_skull_necklaces;
    uint16_t starting_boko_baba_seeds;
    uint16_t starting_golden_feathers;
    uint16_t starting_knights_crests;
    uint16_t starting_red_chu_jellys;
    uint16_t starting_green_chu_jellys;
    uint16_t starting_blue_chu_jellys;
    bool remove_music;

    bool start_with_random_item;
    bool random_item_slide_item;
    bool classic_mode;
    bool plandomizer;
    fspath plandomizerFile;

    TargetTypePreference target_type;
    CameraPreference camera;
    FirstPersonCameraPreference first_person_camera;
    GyroscopePreference gyroscope;
    UIDisplayPreference ui_display;

    CustomModel selectedModel;

    Settings();
    void resetDefaultSettings();
    void resetDefaultPreferences(const bool& paths = false);

    uint8_t getSetting(const Option& option) const;
    void setSetting(const Option& option, const size_t& value);
    int evaluateOption(const std::string& optionStr) const;

    bool anyEntrancesShuffled() const;
};

GameVersion nameToGameVersion(const std::string& name);
std::string GameVersionToName(const GameVersion& version);

PigColor nameToPigColor(const std::string& name);
std::string PigColorToName(const PigColor& color);

PlacementOption nameToPlacementOption(const std::string& name);
std::string PlacementOptionToName(const PlacementOption& option);

ProgressionDungeons nameToProgressionDungeons(const std::string& name);
std::string ProgressionDungeonsToName(const ProgressionDungeons& option);

ShuffleCaveEntrances nameToShuffleCaveEntrances(const std::string& name);
std::string ShuffleCaveEntrancesToName(const ShuffleCaveEntrances& option);

TargetTypePreference nameToTargetTypePreference(const std::string& name);
std::string TargetTypePreferenceToName(const TargetTypePreference& preference);

CameraPreference nameToCameraPreference(const std::string& name);
std::string CameraPreferenceToName(const CameraPreference& preference);

FirstPersonCameraPreference nameToFirstPersonCameraPreference(const std::string& name);
std::string FirstPersonCameraPreferenceToName(const FirstPersonCameraPreference& preference);

GyroscopePreference nameToGyroscopePreference(const std::string& name);
std::string GyroscopePreferenceToName(const GyroscopePreference& preference);

UIDisplayPreference nameToUIDisplayPreference(const std::string& name);
std::string UIDisplayPreferenceToName(const UIDisplayPreference& preference);

int nameToSettingInt(const std::string& name);
Option nameToSetting(const std::string& name);
std::string settingToName(const Option& setting);

std::set<std::string> getDefaultExcludedLocations();
const std::set<std::string>& getAllLocationsNames();
