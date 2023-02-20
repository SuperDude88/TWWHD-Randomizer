#pragma once

#include <vector>
#include <unordered_map>

#include <logic/GameItem.hpp>

constexpr unsigned int MAXIMUM_ADDITIONAL_STARTING_ITEMS = 256;
constexpr unsigned int MAXIMUM_STARTING_HC = 6;
constexpr unsigned int MAXIMUM_STARTING_HP = 44;
constexpr unsigned int MAXIMUM_STARTING_JOY_PENDANTS = 40;
constexpr unsigned int MAXIMUM_STARTING_SKULL_NECKLACES = 23;
constexpr unsigned int MAXIMUM_STARTING_BOKO_BABA_SEEDS = 10;
constexpr unsigned int MAXIMUM_STARTING_GOLDEN_FEATHERS = 20;
constexpr unsigned int MAXIMUM_STARTING_KNIGHTS_CRESTS = 10;
constexpr unsigned int MAXIMUM_STARTING_RED_CHU_JELLYS = 15;
constexpr unsigned int MAXIMUM_STARTING_GREEN_CHU_JELLYS = 15;
constexpr unsigned int MAXIMUM_STARTING_BLUE_CHU_JELLYS = 15;

enum struct PigColor : uint8_t {
    BLACK = 0,
    PINK,
    SPOTTED,
    RANDOM,
    INVALID
};

enum struct SwordMode {
    StartWithSword = 0,
    RandomSword,
    NoSword,
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

enum struct Option {
    INVALID = 0,
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
    ProgressObscure,

    // Additional Randomization Options
    SwordMode,
    Keylunacy,
    RaceMode,
    NumRaceModeDungeons,
    NumShards,
    RandomCharts,
    CTMC,

    // Dungeon Randomization Options
    DungeonSmallKeys,
    DungeonBigKeys,
    DungeonMapsAndCompasses,

    // Convenince Tweaks
    InvertCompass,
    InstantText,
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

    // Advanced Options
    NoSpoilerLog,
    StartWithRandomItem,
    Plandomizer,
    PlandomizerFile,

    // Hints
    HoHoHints,
    KorlHints,
    ClearerHints,
    UseAlwaysHints,
    PathHints,
    BarrenHints,
    ItemHints,
    LocationHints,

    // Entrance Randomizer
    RandomizeDungeonEntrances,
    RandomizeCaveEntrances,
    RandomizeDoorEntrances,
    RandomizeMiscEntrances,
    MixDungeons,
    MixCaves,
    MixDoors,
    MixMisc,
    DecoupleEntrances,
    RandomStartIsland,

    // Cosmetics
    CasualClothes,
    PigColor,


    DamageMultiplier,

    // In game preference
    TargetType,
    Camera,
    FirstPersonCamera,
    Gyroscope,
    UIDisplay,

    // Dummy options to satisfy tracker permalink
    RandomizeEntrances,
    SwiftSail,
    DisableTingleChestsWithTingleBombs,
    RandomizeEnemyPalettes,
    RandomizeEnemies,
    RandomizeMusic,
    InvertCameraXAxis,
    WindWakerHD,
    COUNT
};

struct Settings {
    ProgressionDungeons progression_dungeons = ProgressionDungeons::Standard;
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

    PlacementOption dungeon_small_keys = PlacementOption::Vanilla;
    PlacementOption dungeon_big_keys = PlacementOption::Vanilla;
    PlacementOption dungeon_maps_compasses = PlacementOption::Vanilla;
    bool randomize_charts = false;
    bool randomize_starting_island = false;
    bool randomize_dungeon_entrances = false;
    bool randomize_cave_entrances = false;
    bool randomize_door_entrances = false;
    bool randomize_misc_entrances = false;
    bool mix_dungeons = false;
    bool mix_caves = false;
    bool mix_doors = false;
    bool mix_misc = false;
    bool decouple_entrances = false;

    bool ho_ho_hints = false;
    bool korl_hints = false;
    bool clearer_hints = false;
    bool use_always_hints = false;
    uint8_t path_hints = 0;
    uint8_t barren_hints = 0;
    uint8_t item_hints = 0;
    uint8_t location_hints = 0;

    bool instant_text_boxes = false;
    bool reveal_full_sea_chart = false;
    uint8_t num_starting_triforce_shards = 0;
    bool add_shortcut_warps_between_dungeons = false;
    bool do_not_generate_spoiler_log = false;
    SwordMode sword_mode = SwordMode::StartWithSword;
    bool skip_rematch_bosses = false;
    bool invert_sea_compass_x_axis = false;
    uint8_t num_race_mode_dungeons = 3;
    float damage_multiplier = 2.0f;
    bool chest_type_matches_contents = false;

    bool player_in_casual_clothes = false;
    PigColor pig_color = PigColor::RANDOM;

    std::vector<GameItem> starting_gear = {};
    uint16_t starting_pohs = 0;
    uint16_t starting_hcs = 0;
    uint16_t starting_joy_pendants = 0;
    uint16_t starting_skull_necklaces = 0;
    uint16_t starting_boko_baba_seeds = 0;
    uint16_t starting_golden_feathers = 0;
    uint16_t starting_knights_crests = 0;
    uint16_t starting_red_chu_jellys = 0;
    uint16_t starting_green_chu_jellys = 0;
    uint16_t starting_blue_chu_jellys = 0;
    bool remove_music = false;

    bool start_with_random_item = false;
    bool plandomizer = false;
    std::string plandomizerFile = "";

    TargetTypePreference target_type = TargetTypePreference::Hold;
    CameraPreference camera = CameraPreference::Standard;
    FirstPersonCameraPreference first_person_camera = FirstPersonCameraPreference::Standard;
    GyroscopePreference gyroscope = GyroscopePreference::On;
    UIDisplayPreference ui_display = UIDisplayPreference::On;
};

SwordMode nameToSwordMode(const std::string& name);

std::string SwordModeToName(const SwordMode& mode);

PigColor nameToPigColor(const std::string& name);

std::string PigColorToName(const PigColor& name);

PlacementOption nameToPlacementOption(const std::string& name);

std::string PlacementOptionToName(const PlacementOption& option);

ProgressionDungeons nameToProgressionDungeons(const std::string& name);

std::string ProgressionDungeonsToName(const ProgressionDungeons& option);

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

uint8_t getSetting(const Settings& settings, const Option& option);

void setSetting(Settings& settings, const Option& option, const size_t& value);

int evaluateOption(const Settings& settings, const std::string& optionStr);
