#include "OptionActions.hpp"

#include <algorithm>

#include <seedgen/seed.hpp>

static Config conf;

static std::string fromBool(const bool& b) {
    return b ? "Enabled" : "Disabled";
}

namespace OptionCB {
    void setSeed(const std::string& seed_) {
        conf.seed = seed_;
    }

    void changeSeed() {
        conf.seed = generate_seed();
    }

    void loadPermalink(const std::string& permalink_) {
        conf.loadPermalink(permalink_);
    }

    std::string cycleDungeonMode() {
        using enum ProgressionDungeons;

        switch(conf.settings.progression_dungeons) {
            case Disabled:
                conf.settings.progression_dungeons = Standard;
                break;
            case Standard:
                conf.settings.progression_dungeons = RaceMode;
                if(conf.settings.num_required_dungeons < 1) {
                    conf.settings.num_required_dungeons = 1;
                }
                break;
            case RaceMode:
                conf.settings.progression_dungeons = Disabled;
                break;
            case INVALID:
            default:
                conf.settings.progression_dungeons = Disabled;
                break;
        }

        return ProgressionDungeonsToName(conf.settings.progression_dungeons);
    }

    std::string toggleFairies() {
        conf.settings.progression_great_fairies = !conf.settings.progression_great_fairies;
        return fromBool(conf.settings.progression_great_fairies);
    }

    std::string togglePuzzleCaves() {
        conf.settings.progression_puzzle_secret_caves = !conf.settings.progression_puzzle_secret_caves;
        return fromBool(conf.settings.progression_puzzle_secret_caves);
    }

    std::string toggleCombatCaves() {
        conf.settings.progression_combat_secret_caves = !conf.settings.progression_combat_secret_caves;
        return fromBool(conf.settings.progression_combat_secret_caves);
    }

    std::string toggleShortSidequests() {
        conf.settings.progression_short_sidequests = !conf.settings.progression_short_sidequests;
        return fromBool(conf.settings.progression_short_sidequests);
    }

    std::string toggleLongSidequests() {
        conf.settings.progression_long_sidequests = !conf.settings.progression_long_sidequests;
        return fromBool(conf.settings.progression_long_sidequests);
    }

    std::string toggleSpoilsTrading() {
        conf.settings.progression_spoils_trading = !conf.settings.progression_spoils_trading;
        return fromBool(conf.settings.progression_spoils_trading);
    }

    std::string toggleMinigames() {
        conf.settings.progression_minigames = !conf.settings.progression_minigames;
        return fromBool(conf.settings.progression_minigames);
    }

    std::string toggleFreeGifts() {
        conf.settings.progression_free_gifts = !conf.settings.progression_free_gifts;
        return fromBool(conf.settings.progression_free_gifts);
    }

    std::string toggleMail() {
        conf.settings.progression_mail = !conf.settings.progression_mail;
        return fromBool(conf.settings.progression_mail);
    }

    std::string togglePlatforms() {
        conf.settings.progression_platforms_rafts = !conf.settings.progression_platforms_rafts;
        return fromBool(conf.settings.progression_platforms_rafts);
    }

    std::string toggleSubmarines() {
        conf.settings.progression_submarines = !conf.settings.progression_submarines;
        return fromBool(conf.settings.progression_submarines);
    }

    std::string toggleReefs() {
        conf.settings.progression_eye_reef_chests = !conf.settings.progression_eye_reef_chests;
        return fromBool(conf.settings.progression_eye_reef_chests);
    }

    std::string toggleOctos() {
        conf.settings.progression_big_octos_gunboats = !conf.settings.progression_big_octos_gunboats;
        return fromBool(conf.settings.progression_big_octos_gunboats);
    }

    std::string toggleTriforceCharts() {
        conf.settings.progression_triforce_charts = !conf.settings.progression_triforce_charts;
        return fromBool(conf.settings.progression_triforce_charts);
    }

    std::string toggleTreasureCharts() {
        conf.settings.progression_treasure_charts = !conf.settings.progression_treasure_charts;
        return fromBool(conf.settings.progression_treasure_charts);
    }

    std::string toggleExpensivePurchases() {
        conf.settings.progression_expensive_purchases = !conf.settings.progression_expensive_purchases;
        return fromBool(conf.settings.progression_expensive_purchases);
    }

    std::string toggleMisc() {
        conf.settings.progression_misc = !conf.settings.progression_misc;
        return fromBool(conf.settings.progression_misc);
    }

    std::string toggleTingleChests() {
        conf.settings.progression_tingle_chests = !conf.settings.progression_tingle_chests;
        return fromBool(conf.settings.progression_tingle_chests);
    }

    std::string toggleBattlesquid() {
        conf.settings.progression_battlesquid = !conf.settings.progression_battlesquid;
        return fromBool(conf.settings.progression_battlesquid);
    }

    std::string toggleSavage() {
        conf.settings.progression_savage_labyrinth = !conf.settings.progression_savage_labyrinth;
        return fromBool(conf.settings.progression_savage_labyrinth);
    }

    std::string toggleIslandPuzzles() {
        conf.settings.progression_island_puzzles = !conf.settings.progression_island_puzzles;
        return fromBool(conf.settings.progression_island_puzzles);
    }

    std::string toggleDungeonSecrets() {
        conf.settings.progression_dungeon_secrets = !conf.settings.progression_dungeon_secrets;
        return fromBool(conf.settings.progression_dungeon_secrets);
    }

    std::string toggleObscure() {
        conf.settings.progression_obscure = !conf.settings.progression_obscure;
        return fromBool(conf.settings.progression_obscure);
    }


    std::string cycleSmallKeyMode() {
        using enum PlacementOption;

        switch(conf.settings.dungeon_small_keys) {
            case Vanilla:
                conf.settings.dungeon_small_keys = OwnDungeon;
                break;
            case OwnDungeon:
                conf.settings.dungeon_small_keys = AnyDungeon;
                break;
            case AnyDungeon:
                conf.settings.dungeon_small_keys = Overworld;
                break;
            case Overworld:
                conf.settings.dungeon_small_keys = Keysanity;
                break;
            case Keysanity:
                conf.settings.dungeon_small_keys = Vanilla;
                break;
            case INVALID:
            default:
                conf.settings.dungeon_small_keys = Vanilla;
                break;
        }

        return PlacementOptionToName(conf.settings.dungeon_small_keys);
    }

    std::string cycleBigKeyMode() {
        using enum PlacementOption;

        switch(conf.settings.dungeon_big_keys) {
            case Vanilla:
                conf.settings.dungeon_big_keys = OwnDungeon;
                break;
            case OwnDungeon:
                conf.settings.dungeon_big_keys = AnyDungeon;
                break;
            case AnyDungeon:
                conf.settings.dungeon_big_keys = Overworld;
                break;
            case Overworld:
                conf.settings.dungeon_big_keys = Keysanity;
                break;
            case Keysanity:
                conf.settings.dungeon_big_keys = Vanilla;
                break;
            case INVALID:
            default:
                conf.settings.dungeon_big_keys = Vanilla;
                break;
        }

        return PlacementOptionToName(conf.settings.dungeon_big_keys);
    }

    std::string cycleMapCompassMode() {
        using enum PlacementOption;

        switch(conf.settings.dungeon_maps_compasses) {
            case Vanilla:
                conf.settings.dungeon_maps_compasses = OwnDungeon;
                break;
            case OwnDungeon:
                conf.settings.dungeon_maps_compasses = AnyDungeon;
                break;
            case AnyDungeon:
                conf.settings.dungeon_maps_compasses = Overworld;
                break;
            case Overworld:
                conf.settings.dungeon_maps_compasses = Keysanity;
                break;
            case Keysanity:
                conf.settings.dungeon_maps_compasses = Vanilla;
                break;
            case INVALID:
            default:
                conf.settings.dungeon_maps_compasses = Vanilla;
                break;
        }

        return PlacementOptionToName(conf.settings.dungeon_maps_compasses);
    }

    std::string toggleChartShuffle() {
        conf.settings.randomize_charts = !conf.settings.randomize_charts;
        return fromBool(conf.settings.randomize_charts);
    }

    std::string toggleRandomIsland() {
        conf.settings.randomize_starting_island = !conf.settings.randomize_starting_island;
        return fromBool(conf.settings.randomize_starting_island);
    }

    std::string toggleDungeonEntranceShuffle() {
        conf.settings.randomize_dungeon_entrances = !conf.settings.randomize_dungeon_entrances;
        return fromBool(conf.settings.randomize_dungeon_entrances);
    }

    std::string toggleBossEntranceShuffle() {
        conf.settings.randomize_boss_entrances = !conf.settings.randomize_boss_entrances;
        return fromBool(conf.settings.randomize_boss_entrances);
    }

    std::string toggleMinibossEntranceShuffle() {
        conf.settings.randomize_miniboss_entrances = !conf.settings.randomize_miniboss_entrances;
        return fromBool(conf.settings.randomize_miniboss_entrances);
    }

    std::string cycleCaveEntranceShuffle() {
        using enum ShuffleCaveEntrances;

        switch(conf.settings.randomize_cave_entrances) {
            case Disabled:
                conf.settings.randomize_cave_entrances = Caves;
                break;
            case Caves:
                conf.settings.randomize_cave_entrances = CavesFairies;
                break;
            case CavesFairies:
                conf.settings.randomize_cave_entrances = Disabled;
                break;
            case INVALID:
            default:
                conf.settings.randomize_cave_entrances = Disabled;
                break;
        }

        return ShuffleCaveEntrancesToName(conf.settings.randomize_cave_entrances);
    }

    std::string toggleDoorEntranceShuffle() {
        conf.settings.randomize_door_entrances = !conf.settings.randomize_door_entrances;
        return fromBool(conf.settings.randomize_door_entrances);
    }

    std::string toggleMiscEntranceShuffle() {
        conf.settings.randomize_misc_entrances = !conf.settings.randomize_misc_entrances;
        return fromBool(conf.settings.randomize_misc_entrances);
    }

    std::string toggleDungeonEntranceMix() {
        conf.settings.mix_dungeons = !conf.settings.mix_dungeons;
        return fromBool(conf.settings.mix_dungeons);
    }
    std::string toggleBossEntranceMix() {
        conf.settings.mix_bosses = !conf.settings.mix_bosses;
        return fromBool(conf.settings.mix_bosses);
    }
    std::string toggleMinibossEntranceMix() {
        conf.settings.mix_minibosses = !conf.settings.mix_minibosses;
        return fromBool(conf.settings.mix_minibosses);
    }
    std::string toggleCaveEntranceMix() {
        conf.settings.mix_caves = !conf.settings.mix_caves;
        return fromBool(conf.settings.mix_caves);
    }
    std::string toggleDoorEntranceMix() {
        conf.settings.mix_doors = !conf.settings.mix_doors;
        return fromBool(conf.settings.mix_doors);
    }
    std::string toggleMiscEntranceMix() {
        conf.settings.mix_misc = !conf.settings.mix_misc;
        return fromBool(conf.settings.mix_misc);
    }
    std::string toggleDecoupleEntrances() {
        conf.settings.decouple_entrances = !conf.settings.decouple_entrances;
        return fromBool(conf.settings.decouple_entrances);
    }


    std::string toggleHoHoHints() {
        conf.settings.ho_ho_hints = !conf.settings.ho_ho_hints;
        return fromBool(conf.settings.ho_ho_hints);
    }

    std::string toggleKorlHints() {
        conf.settings.korl_hints = !conf.settings.korl_hints;
        return fromBool(conf.settings.korl_hints);
    }

    std::string toggleClearHints() {
        conf.settings.clearer_hints = !conf.settings.clearer_hints;
        return fromBool(conf.settings.clearer_hints);
    }

    std::string toggleAlwaysHints() {
        conf.settings.use_always_hints = !conf.settings.use_always_hints;
        return fromBool(conf.settings.use_always_hints);
    }

    std::string toggleHintImportance() {
        conf.settings.hint_importance = !conf.settings.hint_importance;
        return fromBool(conf.settings.hint_importance);
    }

    std::string cyclePathHints() {
        if(conf.settings.path_hints == MAXIMUM_PATH_HINT_COUNT) {
            conf.settings.path_hints = 0;
        }
        else {
            conf.settings.path_hints++;
        }

        return std::to_string(conf.settings.path_hints);
    }

    std::string cycleBarrenHints() {
        if(conf.settings.barren_hints == MAXIMUM_BARREN_HINT_COUNT) {
            conf.settings.barren_hints = 0;
        }
        else {
            conf.settings.barren_hints++;
        }

        return std::to_string(conf.settings.barren_hints);
    }

    std::string cycleItemHints() {
        if(conf.settings.item_hints == MAXIMUM_ITEM_HINT_COUNT) {
            conf.settings.item_hints = 0;
        }
        else {
            conf.settings.item_hints++;
        }

        return std::to_string(conf.settings.item_hints);
    }

    std::string cycleLocationHints() {
        if(conf.settings.location_hints == MAXIMUM_LOCATION_HINT_COUNT) {
            conf.settings.location_hints = 0;
        }
        else {
            conf.settings.location_hints++;
        }

        return std::to_string(conf.settings.location_hints);
    }


    std::string toggleInstantText() {
        conf.settings.instant_text_boxes = !conf.settings.instant_text_boxes;
        return fromBool(conf.settings.instant_text_boxes);
    }

    std::string toggleRNG() {
        conf.settings.fix_rng = !conf.settings.fix_rng;
        return fromBool(conf.settings.fix_rng);
    }

    std::string togglePerformance() {
        conf.settings.performance = !conf.settings.performance;
        return fromBool(conf.settings.performance);
    }

    std::string toggleFullSeaChart() {
        conf.settings.reveal_full_sea_chart = !conf.settings.reveal_full_sea_chart;
        return fromBool(conf.settings.reveal_full_sea_chart);
    }

    std::string toggleDungeonWarps() {
        conf.settings.add_shortcut_warps_between_dungeons = !conf.settings.add_shortcut_warps_between_dungeons;
        return fromBool(conf.settings.add_shortcut_warps_between_dungeons);
    }

    std::string toggleSpoilerLog() {
        conf.settings.do_not_generate_spoiler_log = !conf.settings.do_not_generate_spoiler_log;
        return fromBool(conf.settings.do_not_generate_spoiler_log);
    }

    std::string toggleSwords() {
        conf.settings.remove_swords = !conf.settings.remove_swords;

        if(conf.settings.remove_swords) {
            std::erase(conf.settings.starting_gear, GameItem::ProgressiveSword); // can't start with swords if we're removing them
            std::erase(conf.settings.starting_gear, GameItem::HurricaneSpin); // also take out hurricane spin if necessary
        }
        else {
            addStartingItem(GameItem::ProgressiveSword); // start with 1 sword by default
        }

        return fromBool(conf.settings.remove_swords);
    }

    std::string toggleTrials() {
        conf.settings.skip_rematch_bosses = !conf.settings.skip_rematch_bosses;
        return fromBool(conf.settings.skip_rematch_bosses);
    }

    std::string toggleInvertCompass() {
        conf.settings.invert_sea_compass_x_axis = !conf.settings.invert_sea_compass_x_axis;
        return fromBool(conf.settings.invert_sea_compass_x_axis);
    }

    std::string cycleNumDungeons() {
        uint8_t min_dungeons = 0;
        if(conf.settings.progression_dungeons == ProgressionDungeons::RaceMode) {
            min_dungeons = 1;
        }

        if(conf.settings.num_required_dungeons == MAXIMUM_NUM_DUNGEONS) {
            conf.settings.num_required_dungeons = min_dungeons;
        }
        else {
            conf.settings.num_required_dungeons++;
        }

        return std::to_string(conf.settings.num_required_dungeons);
    }

    std::string cycleDamageMultiplier() {
        if(conf.settings.damage_multiplier == MAXIMUM_DAMAGE_MULTIPLIER) {
            conf.settings.damage_multiplier = 2.0f;
        }
        else {
            conf.settings.damage_multiplier += 1.0f;
        }

        return std::to_string((uint32_t)conf.settings.damage_multiplier);
    }

    std::string toggleCTMC() {
        conf.settings.chest_type_matches_contents = !conf.settings.chest_type_matches_contents;
        return fromBool(conf.settings.chest_type_matches_contents);
    }


    std::string toggleCasualClothes() {
        conf.settings.selectedModel.casual = !conf.settings.selectedModel.casual;
        return fromBool(conf.settings.selectedModel.casual);
    }
    
    std::string isCasual() {
        return fromBool(conf.settings.selectedModel.casual);
    }

    std::string randomizeColorsOrderly() {
        conf.settings.selectedModel.randomizeOrderly();
        return "";
    }

    std::string randomizeColorsChaotically() {
        conf.settings.selectedModel.randomizeChaotically();
        return "";
    }

    std::string cyclePigColor() {
        using enum PigColor;

        switch(conf.settings.pig_color) {
            case Black:
                conf.settings.pig_color = Pink;
                break;
            case Pink:
                conf.settings.pig_color = Spotted;
                break;
            case Spotted:
                conf.settings.pig_color = Random;
                break;
            case Random:
                conf.settings.pig_color = Black;
                break;
            case INVALID:
            default:
                conf.settings.pig_color = Random;
                break;
        }

        return PigColorToName(conf.settings.pig_color);
    }


    //this one gets special handling
    //std::string setStartingGear();

    std::string cycleStartingHP() {
        if(conf.settings.starting_pohs == MAXIMUM_STARTING_HP) {
            conf.settings.starting_pohs = 0;
        }
        else {
            conf.settings.starting_pohs++;
        }

        return std::to_string(conf.settings.starting_pohs);
    }

    std::string cycleStartingHC() {
        if(conf.settings.starting_hcs == MAXIMUM_STARTING_HC) {
            conf.settings.starting_hcs = 1;
        }
        else {
            conf.settings.starting_hcs++;
        }

        return std::to_string(conf.settings.starting_hcs);
    }

    std::string cycleStartingJP() {
        if(conf.settings.starting_joy_pendants == MAXIMUM_STARTING_JOY_PENDANTS) {
            conf.settings.starting_joy_pendants = 0;
        }
        else {
            conf.settings.starting_joy_pendants++;
        }

        return std::to_string(conf.settings.starting_joy_pendants);
    }

    std::string cycleStartingSN() {
        if(conf.settings.starting_skull_necklaces == MAXIMUM_STARTING_SKULL_NECKLACES) {
            conf.settings.starting_skull_necklaces = 0;
        }
        else {
            conf.settings.starting_skull_necklaces++;
        }

        return std::to_string(conf.settings.starting_skull_necklaces);
    }

    std::string cycleStartingBS() {
        if(conf.settings.starting_boko_baba_seeds == MAXIMUM_STARTING_BOKO_BABA_SEEDS) {
            conf.settings.starting_boko_baba_seeds = 0;
        }
        else {
            conf.settings.starting_boko_baba_seeds++;
        }

        return std::to_string(conf.settings.starting_boko_baba_seeds);
    }

    std::string cycleStartingGF() {
        if(conf.settings.starting_golden_feathers == MAXIMUM_STARTING_GOLDEN_FEATHERS) {
            conf.settings.starting_golden_feathers = 0;
        }
        else {
            conf.settings.starting_golden_feathers++;
        }

        return std::to_string(conf.settings.starting_golden_feathers);
    }

    std::string cycleStartingKC() {
        if(conf.settings.starting_knights_crests == MAXIMUM_STARTING_KNIGHTS_CRESTS) {
            conf.settings.starting_knights_crests = 0;
        }
        else {
            conf.settings.starting_knights_crests++;
        }

        return std::to_string(conf.settings.starting_knights_crests);
    }

    std::string cycleStartingRC() {
        if(conf.settings.starting_red_chu_jellys == MAXIMUM_STARTING_RED_CHU_JELLYS) {
            conf.settings.starting_red_chu_jellys = 0;
        }
        else {
            conf.settings.starting_red_chu_jellys++;
        }

        return std::to_string(conf.settings.starting_red_chu_jellys);
    }

    std::string cycleStartingGC() {
        if(conf.settings.starting_green_chu_jellys == MAXIMUM_STARTING_GREEN_CHU_JELLYS) {
            conf.settings.starting_green_chu_jellys = 0;
        }
        else {
            conf.settings.starting_green_chu_jellys++;
        }

        return std::to_string(conf.settings.starting_green_chu_jellys);
    }

    std::string cycleStartingBC() {
        if(conf.settings.starting_blue_chu_jellys == MAXIMUM_STARTING_BLUE_CHU_JELLYS) {
            conf.settings.starting_blue_chu_jellys = 0;
        }
        else {
            conf.settings.starting_blue_chu_jellys++;
        }

        return std::to_string(conf.settings.starting_blue_chu_jellys);
    }

    std::string toggleMusic() {
        conf.settings.remove_music = !conf.settings.remove_music;
        return fromBool(conf.settings.remove_music);
    }


    std::string toggleRandomStartItem() {
        conf.settings.start_with_random_item = !conf.settings.start_with_random_item;
        return fromBool(conf.settings.start_with_random_item);
    }
    
    std::string toggleRandomItemSlideItem() {
        conf.settings.random_item_slide_item = !conf.settings.random_item_slide_item;
        return fromBool(conf.settings.random_item_slide_item);
    }
    
    std::string toggleClassicMode() {
        conf.settings.classic_mode = !conf.settings.classic_mode;
        return fromBool(conf.settings.classic_mode);
    }

    std::string togglePlandomizer() {
        conf.settings.plandomizer = !conf.settings.plandomizer;
        return fromBool(conf.settings.plandomizer);
    }


    std::string toggleTargetPref() {
        using enum TargetTypePreference;

        switch(conf.settings.target_type) {
            case Hold:
                conf.settings.target_type = Switch;
                break;
            case Switch:
                conf.settings.target_type = Hold;
                break;
            case INVALID:
            default:
                conf.settings.target_type = Hold;
                break;
        }

        return TargetTypePreferenceToName(conf.settings.target_type);
    }

    std::string toggleCameraPref() {
        using enum CameraPreference;

        switch(conf.settings.camera) {
            case Standard:
                conf.settings.camera = ReverseLeftRight;
                break;
            case ReverseLeftRight:
                conf.settings.camera = Standard;
                break;
            case INVALID:
            default:
                conf.settings.camera = Standard;
                break;
        }

        return CameraPreferenceToName(conf.settings.camera);

    }

    std::string toggleFirstPersonPref() {
        using enum FirstPersonCameraPreference;

        switch(conf.settings.first_person_camera) {
            case Standard:
                conf.settings.first_person_camera = ReverseUpDown;
                break;
            case ReverseUpDown:
                conf.settings.first_person_camera = Standard;
                break;
            case INVALID:
            default:
                conf.settings.first_person_camera = Standard;
                break;
        }

        return FirstPersonCameraPreferenceToName(conf.settings.first_person_camera);

    }

    std::string toggleGyroPref() {
        using enum GyroscopePreference;

        switch(conf.settings.gyroscope) {
            case On:
                conf.settings.gyroscope = Off;
                break;
            case Off:
                conf.settings.gyroscope = On;
                break;
            case INVALID:
            default:
                conf.settings.gyroscope = On;
                break;
        }

        return GyroscopePreferenceToName(conf.settings.gyroscope);

    }

    std::string toggleUIPref() {
        using enum UIDisplayPreference;

        switch(conf.settings.ui_display) {
            case On:
                conf.settings.ui_display = Off;
                break;
            case Off:
                conf.settings.ui_display = On;
                break;
            case INVALID:
            default:
                conf.settings.ui_display = On;
                break;
        }

        return UIDisplayPreferenceToName(conf.settings.ui_display);

    }

    std::string invalidCB() {
        return "";
    }


    void clearStartingItems() {
        conf.settings.starting_gear.clear();
    }
    
    bool hasStartingItem(const GameItem& item, const size_t& num) {
        return std::count(conf.settings.starting_gear.begin(), conf.settings.starting_gear.end(), item) >= num;
    }

    void addStartingItem(const GameItem& item) {
        conf.settings.starting_gear.push_back(item);
    }

    void clearExcludedLocations() {
        conf.settings.excluded_locations.clear();
    }

    bool hasExcludedLocation(const std::string& locName) {
        return conf.settings.excluded_locations.contains(locName);
    }

    void addExcludedLocation(const std::string& locName) {
        conf.settings.excluded_locations.insert(locName);
    }
    
    bool hasAllCategories(const std::set<LocationCategory>& locationCategories) {
        return std::all_of(locationCategories.begin(), locationCategories.end(), [&](const LocationCategory& category)
        {
            if (category == LocationCategory::AlwaysProgression) return true;
            return ( category == LocationCategory::Dungeon           && conf.settings.progression_dungeons != ProgressionDungeons::Disabled)        ||
                   ( category == LocationCategory::DungeonSecret     && conf.settings.progression_dungeon_secrets)                                  ||
                   ( category == LocationCategory::GreatFairy        && conf.settings.progression_great_fairies)                                    ||
                   ( category == LocationCategory::PuzzleSecretCave  && conf.settings.progression_puzzle_secret_caves)                              ||
                   ( category == LocationCategory::CombatSecretCave  && conf.settings.progression_combat_secret_caves)                              ||
                   ( category == LocationCategory::ShortSideQuest    && conf.settings.progression_short_sidequests)                                 ||
                   ( category == LocationCategory::LongSideQuest     && conf.settings.progression_long_sidequests)                                  ||
                   ( category == LocationCategory::SpoilsTrading     && conf.settings.progression_spoils_trading)                                   ||
                   ( category == LocationCategory::Minigame          && conf.settings.progression_minigames)                                        ||
                   ( category == LocationCategory::FreeGift          && conf.settings.progression_free_gifts)                                       ||
                   ( category == LocationCategory::Mail              && conf.settings.progression_mail)                                             ||
                   ( category == LocationCategory::Submarine         && conf.settings.progression_submarines)                                       ||
                   ( category == LocationCategory::EyeReefChests     && conf.settings.progression_eye_reef_chests)                                  ||
                   ( category == LocationCategory::SunkenTreasure    && conf.settings.progression_triforce_charts)                                  ||
                   ( category == LocationCategory::SunkenTreasure    && conf.settings.progression_treasure_charts)                                  ||
                   ( category == LocationCategory::ExpensivePurchase && conf.settings.progression_expensive_purchases)                              ||
                   ( category == LocationCategory::Misc              && conf.settings.progression_misc)                                             ||
                   ( category == LocationCategory::TingleChest       && conf.settings.progression_tingle_chests)                                    ||
                   ( category == LocationCategory::BattleSquid       && conf.settings.progression_battlesquid)                                      ||
                   ( category == LocationCategory::SavageLabyrinth   && conf.settings.progression_savage_labyrinth)                                 ||
                   ( category == LocationCategory::IslandPuzzle      && conf.settings.progression_island_puzzles)                                   ||
                   ( category == LocationCategory::Obscure           && conf.settings.progression_obscure)                                          ||
                   ((category == LocationCategory::Platform || category == LocationCategory::Raft)    && conf.settings.progression_platforms_rafts) ||
                   ((category == LocationCategory::BigOcto  || category == LocationCategory::Gunboat) && conf.settings.progression_big_octos_gunboats);
        });
    }

    void resetInternal() {
        conf.resetDefaults();
    }

    void setInternal(const Config& in) {
        conf = in;
    }
    
    Config getInternal() {
        return conf;
    }
}

bool wasUpdated() {
    return conf.updated;
}

bool wasConverted() {
    return conf.converted;
}

std::string getSeed() {
    return conf.seed;
}

std::string getSeedHash() {
    return hash_for_config(conf);
}

std::string getPermalink() {
    return conf.getPermalink();
}

std::string getValue(const Option& option) {
    switch (option) {
        case Option::ProgressDungeons:
            return ProgressionDungeonsToName(conf.settings.progression_dungeons);
        case Option::ProgressGreatFairies:
            return fromBool(conf.settings.progression_great_fairies);
        case Option::ProgressPuzzleCaves:
            return fromBool(conf.settings.progression_puzzle_secret_caves);
        case Option::ProgressCombatCaves:
            return fromBool(conf.settings.progression_combat_secret_caves);
        case Option::ProgressShortSidequests:
            return fromBool(conf.settings.progression_short_sidequests);
        case Option::ProgressLongSidequests:
            return fromBool(conf.settings.progression_long_sidequests);
        case Option::ProgressSpoilsTrading:
            return fromBool(conf.settings.progression_spoils_trading);
        case Option::ProgressMinigames:
            return fromBool(conf.settings.progression_minigames);
        case Option::ProgressFreeGifts:
            return fromBool(conf.settings.progression_free_gifts);
        case Option::ProgressMail:
            return fromBool(conf.settings.progression_mail);
        case Option::ProgressPlatformsRafts:
            return fromBool(conf.settings.progression_platforms_rafts);
        case Option::ProgressSubmarines:
            return fromBool(conf.settings.progression_submarines);
        case Option::ProgressEyeReefs:
            return fromBool(conf.settings.progression_eye_reef_chests);
        case Option::ProgressOctosGunboats:
            return fromBool(conf.settings.progression_big_octos_gunboats);
        case Option::ProgressTriforceCharts:
            return fromBool(conf.settings.progression_triforce_charts);
        case Option::ProgressTreasureCharts:
            return fromBool(conf.settings.progression_treasure_charts);
        case Option::ProgressExpPurchases:
            return fromBool(conf.settings.progression_expensive_purchases);
        case Option::ProgressMisc:
            return fromBool(conf.settings.progression_misc);
        case Option::ProgressTingleChests:
            return fromBool(conf.settings.progression_tingle_chests);
        case Option::ProgressBattlesquid:
            return fromBool(conf.settings.progression_battlesquid);
        case Option::ProgressSavageLabyrinth:
            return fromBool(conf.settings.progression_savage_labyrinth);
        case Option::ProgressIslandPuzzles:
            return fromBool(conf.settings.progression_island_puzzles);
        case Option::ProgressDungeonSecrets:
            return fromBool(conf.settings.progression_dungeon_secrets);
        case Option::ProgressObscure:
            return fromBool(conf.settings.progression_obscure);
        case Option::DungeonSmallKeys:
            return PlacementOptionToName(conf.settings.dungeon_small_keys);
        case Option::DungeonBigKeys:
            return PlacementOptionToName(conf.settings.dungeon_big_keys);
        case Option::DungeonMapsAndCompasses:
            return PlacementOptionToName(conf.settings.dungeon_maps_compasses);
        case Option::RandomCharts:
            return fromBool(conf.settings.randomize_charts);
        case Option::RandomStartIsland:
            return fromBool(conf.settings.randomize_starting_island);
        case Option::RandomizeDungeonEntrances:
            return fromBool(conf.settings.randomize_dungeon_entrances);
        case Option::RandomizeMinibossEntrances:
            return fromBool(conf.settings.randomize_miniboss_entrances);
        case Option::RandomizeBossEntrances:
            return fromBool(conf.settings.randomize_boss_entrances);
        case Option::RandomizeCaveEntrances:
            return ShuffleCaveEntrancesToName(conf.settings.randomize_cave_entrances);
        case Option::RandomizeDoorEntrances:
            return fromBool(conf.settings.randomize_door_entrances);
        case Option::RandomizeMiscEntrances:
            return fromBool(conf.settings.randomize_misc_entrances);
        case Option::MixDungeons:
            return fromBool(conf.settings.mix_dungeons);
        case Option::MixBosses:
            return fromBool(conf.settings.mix_bosses);
        case Option::MixMinibosses:
            return fromBool(conf.settings.mix_minibosses);
        case Option::MixCaves:
            return fromBool(conf.settings.mix_caves);
        case Option::MixDoors:
            return fromBool(conf.settings.mix_doors);
        case Option::MixMisc:
            return fromBool(conf.settings.mix_misc);
        case Option::DecoupleEntrances:
            return fromBool(conf.settings.decouple_entrances);
        case Option::HoHoHints:
            return fromBool(conf.settings.ho_ho_hints);
        case Option::KorlHints:
            return fromBool(conf.settings.korl_hints);
        case Option::ClearerHints:
            return fromBool(conf.settings.clearer_hints);
        case Option::UseAlwaysHints:
            return fromBool(conf.settings.use_always_hints);
        case Option::HintImportance:
            return fromBool(conf.settings.hint_importance);
        case Option::PathHints:
            return std::to_string(conf.settings.path_hints);
        case Option::BarrenHints:
            return std::to_string(conf.settings.barren_hints);
        case Option::ItemHints:
            return std::to_string(conf.settings.item_hints);
        case Option::LocationHints:
            return std::to_string(conf.settings.location_hints);
        case Option::InstantText:
            return fromBool(conf.settings.instant_text_boxes);
        case Option::FixRNG:
            return fromBool(conf.settings.fix_rng);
        case Option::Performance:
            return fromBool(conf.settings.performance);
        case Option::RevealSeaChart:
            return fromBool(conf.settings.reveal_full_sea_chart);
        case Option::AddShortcutWarps:
            return fromBool(conf.settings.add_shortcut_warps_between_dungeons);
        case Option::NoSpoilerLog:
            return fromBool(conf.settings.do_not_generate_spoiler_log);
        case Option::RemoveSwords:
            return fromBool(conf.settings.remove_swords);
        case Option::SkipRefights:
            return fromBool(conf.settings.skip_rematch_bosses);
        case Option::InvertCompass:
            return fromBool(conf.settings.invert_sea_compass_x_axis);
        case Option::NumRequiredDungeons:
            return std::to_string(conf.settings.num_required_dungeons);
        case Option::DamageMultiplier:
            return std::to_string((uint32_t)conf.settings.damage_multiplier);
        case Option::CTMC:
            return fromBool(conf.settings.chest_type_matches_contents);
        //case Option::CasualClothes:
        //    return fromBool(conf.settings.player_in_casual_clothes);
        case Option::PigColor:
            return PigColorToName(conf.settings.pig_color);
        case Option::StartingGear: //placeholder
            return "";
        case Option::StartingHP:
            return std::to_string(conf.settings.starting_pohs);
        case Option::StartingHC:
            return std::to_string(conf.settings.starting_hcs);
        case Option::StartingJoyPendants:
            return std::to_string(conf.settings.starting_joy_pendants);
        case Option::StartingSkullNecklaces:
            return std::to_string(conf.settings.starting_skull_necklaces);
        case Option::StartingBokoBabaSeeds:
            return std::to_string(conf.settings.starting_boko_baba_seeds);
        case Option::StartingGoldenFeathers:
            return std::to_string(conf.settings.starting_golden_feathers);
        case Option::StartingKnightsCrests:
            return std::to_string(conf.settings.starting_knights_crests);
        case Option::StartingRedChuJellys:
            return std::to_string(conf.settings.starting_red_chu_jellys);
        case Option::StartingGreenChuJellys:
            return std::to_string(conf.settings.starting_green_chu_jellys);
        case Option::StartingBlueChuJellys:
            return std::to_string(conf.settings.starting_blue_chu_jellys);
        case Option::RemoveMusic:
            return fromBool(conf.settings.remove_music);
        case Option::StartWithRandomItem:
            return fromBool(conf.settings.start_with_random_item);
        case Option::RandomItemSlideItem:
            return fromBool(conf.settings.random_item_slide_item);
        case Option::ClassicMode:
            return fromBool(conf.settings.classic_mode);
        case Option::Plandomizer:
            return fromBool(conf.settings.plandomizer);
        case Option::PlandomizerFile: //cant return this like everything else, just here as placeholder
            return "";
        case Option::TargetType:
            return TargetTypePreferenceToName(conf.settings.target_type); 
        case Option::Camera:
            return CameraPreferenceToName(conf.settings.camera);
        case Option::FirstPersonCamera:
            return FirstPersonCameraPreferenceToName(conf.settings.first_person_camera);
        case Option::Gyroscope:
            return GyroscopePreferenceToName(conf.settings.gyroscope);
        case Option::UIDisplay:
            return UIDisplayPreferenceToName(conf.settings.ui_display);
        case Option::INVALID:
        default:
            return "";
    }
}

namespace ColorCB {
    std::string randomizeColor(const std::string& name_) {
        getModel().randomizeSingleColor(name_);

        return "";
    }

    std::string resetColor(const std::string& name_) {
        getModel().resetSingleColor(name_);

        return "";
    }

    std::string invalidCB(const std::string&) {
        return "";
    }
}

CustomModel& getModel() {
    return conf.settings.selectedModel;
}

TriggerCallback getCallback(const Option& option) {
    using namespace OptionCB;

    switch (option) {
        case Option::ProgressDungeons:
            return &cycleDungeonMode;
        case Option::ProgressGreatFairies:
            return &toggleFairies;
        case Option::ProgressPuzzleCaves:
            return &togglePuzzleCaves;
        case Option::ProgressCombatCaves:
            return &toggleCombatCaves;
        case Option::ProgressShortSidequests:
            return &toggleShortSidequests;
        case Option::ProgressLongSidequests:
            return &toggleLongSidequests;
        case Option::ProgressSpoilsTrading:
            return &toggleSpoilsTrading;
        case Option::ProgressMinigames:
            return &toggleMinigames;
        case Option::ProgressFreeGifts:
            return &toggleFreeGifts;
        case Option::ProgressMail:
            return &toggleMail;
        case Option::ProgressPlatformsRafts:
            return &togglePlatforms;
        case Option::ProgressSubmarines:
            return &toggleSubmarines;
        case Option::ProgressEyeReefs:
            return &toggleReefs;
        case Option::ProgressOctosGunboats:
            return &toggleOctos;
        case Option::ProgressTriforceCharts:
            return &toggleTriforceCharts;
        case Option::ProgressTreasureCharts:
            return &toggleTreasureCharts;
        case Option::ProgressExpPurchases:
            return &toggleExpensivePurchases;
        case Option::ProgressMisc:
            return &toggleMisc;
        case Option::ProgressTingleChests:
            return &toggleTingleChests;
        case Option::ProgressBattlesquid:
            return &toggleBattlesquid;
        case Option::ProgressSavageLabyrinth:
            return &toggleSavage;
        case Option::ProgressIslandPuzzles:
            return &toggleIslandPuzzles;
        case Option::ProgressDungeonSecrets:
            return &toggleDungeonSecrets;
        case Option::ProgressObscure:
            return &toggleObscure;
        case Option::DungeonSmallKeys:
            return &cycleSmallKeyMode;
        case Option::DungeonBigKeys:
            return &cycleBigKeyMode;
        case Option::DungeonMapsAndCompasses:
            return &cycleMapCompassMode;
        case Option::RandomCharts:
            return &toggleChartShuffle;
        case Option::RandomStartIsland:
            return &toggleRandomIsland;
        case Option::RandomizeDungeonEntrances:
            return &toggleDungeonEntranceShuffle;
        case Option::RandomizeMinibossEntrances:
            return &toggleMinibossEntranceShuffle;
        case Option::RandomizeBossEntrances:
            return &toggleBossEntranceShuffle;
        case Option::RandomizeCaveEntrances:
            return &cycleCaveEntranceShuffle;
        case Option::RandomizeDoorEntrances:
            return &toggleDoorEntranceShuffle;
        case Option::RandomizeMiscEntrances:
            return &toggleMiscEntranceShuffle;
        case Option::MixDungeons:
            return &toggleDungeonEntranceMix;
        case Option::MixBosses:
            return &toggleBossEntranceMix;
        case Option::MixMinibosses:
            return &toggleMinibossEntranceMix;
        case Option::MixCaves:
            return &toggleCaveEntranceMix;
        case Option::MixDoors:
            return &toggleDoorEntranceMix;
        case Option::MixMisc:
            return &toggleMiscEntranceMix;
        case Option::DecoupleEntrances:
            return &toggleDecoupleEntrances;
        case Option::HoHoHints:
            return &toggleHoHoHints;
        case Option::KorlHints:
            return &toggleKorlHints;
        case Option::ClearerHints:
            return &toggleClearHints;
        case Option::UseAlwaysHints:
            return &toggleAlwaysHints;
        case Option::HintImportance:
            return &toggleHintImportance;
        case Option::PathHints:
            return &cyclePathHints;
        case Option::BarrenHints:
            return &cycleBarrenHints;
        case Option::ItemHints:
            return &cycleItemHints;
        case Option::LocationHints:
            return &cycleLocationHints;
        case Option::InstantText:
            return &toggleInstantText;
        case Option::FixRNG:
            return &toggleRNG;
        case Option::Performance:
            return &togglePerformance;
        case Option::RevealSeaChart:
            return &toggleFullSeaChart;
        case Option::AddShortcutWarps:
            return &toggleDungeonWarps;
        case Option::NoSpoilerLog:
            return &toggleSpoilerLog;
        case Option::RemoveSwords:
            return &toggleSwords;
        case Option::SkipRefights:
            return &toggleTrials;
        case Option::InvertCompass:
            return &toggleInvertCompass;
        case Option::NumRequiredDungeons:
            return &cycleNumDungeons;
        case Option::DamageMultiplier:
            return &cycleDamageMultiplier;
        case Option::CTMC:
            return &toggleCTMC;
        //case Option::CasualClothes:
        //    return &toggleCasualClothes;
        case Option::PigColor:
            return &cyclePigColor;
        case Option::StartingGear: //placeholder
            return &invalidCB;
        case Option::StartingHP:
            return &cycleStartingHP;
        case Option::StartingHC:
            return &cycleStartingHC;
        case Option::StartingJoyPendants:
            return &cycleStartingJP;
        case Option::StartingSkullNecklaces:
            return &cycleStartingSN;
        case Option::StartingBokoBabaSeeds:
            return &cycleStartingBS;
        case Option::StartingGoldenFeathers:
            return &cycleStartingGF;
        case Option::StartingKnightsCrests:
            return &cycleStartingKC;
        case Option::StartingRedChuJellys:
            return &cycleStartingRC;
        case Option::StartingGreenChuJellys:
            return &cycleStartingGC;
        case Option::StartingBlueChuJellys:
            return &cycleStartingBC;
        case Option::RemoveMusic:
            return &toggleMusic;
        case Option::StartWithRandomItem:
            return &toggleRandomStartItem;
        case Option::RandomItemSlideItem:
            return &toggleRandomItemSlideItem;
        case Option::ClassicMode:
            return &toggleClassicMode;
        case Option::Plandomizer:
            return &togglePlandomizer;
        case Option::PlandomizerFile: //cant return this like everything else, just here as placeholder
            return &invalidCB;
        case Option::TargetType:
            return &toggleTargetPref;
        case Option::Camera:
            return &toggleCameraPref;
        case Option::FirstPersonCamera:
            return &toggleFirstPersonPref;
        case Option::Gyroscope:
            return &toggleGyroPref;
        case Option::UIDisplay:
            return &toggleUIPref;
        case Option::INVALID:
        default:
            return &invalidCB;
    }
}

std::pair<std::string, std::string> getNameDesc(const Option& option) {
    using namespace OptionCB;
    using enum Option;

    static const std::unordered_map<Option, std::pair<std::string, std::string>> optionInfoMap = {
        {ProgressDungeons,           {"Dungeons",                            "Disabled: Dungeons will not contain any items necessary to beat the game.\nStandard: Dungeons may contain items necessary to beat the game.\nRace Mode: Dungeons without a required boss will not contain progress items"}},
        {ProgressGreatFairies,       {"Great Fairies",                       "Controls whether the items given by Great Fairies can be progress items."}},
        {ProgressPuzzleCaves,        {"Puzzle Secret Caves",                 "Controls whether puzzle-focused secret caves can contain progress items."}},
        {ProgressCombatCaves,        {"Combat Secret Caves",                 "Controls whether combat-focused secret caves can contain progress items."}},
        {ProgressShortSidequests,    {"Short Sidequests",                    "Controls whether sidequests that can be completed quickly can reward progress items."}},
        {ProgressLongSidequests,     {"Long Sidequests",                     "Controls whether long sidequests (e.g. Lenzo's assistant, withered trees, goron trading) can reward progress items."}},
        {ProgressSpoilsTrading,      {"Spoils Trading",                      "Controls whether the items you get by trading in spoils to NPCs can be progress items."}},
        {ProgressMinigames,          {"Minigames",                           "Controls whether most minigames can reward progress items (auction, mail sorting, barrel shooting, bird-man contest)."}},
        {ProgressFreeGifts,          {"Free Gifts",                          "Controls whether gifts freely given by NPCs can be progress items (Tott, Salvage Corp, imprisoned Tingle)."}},
        {ProgressMail,               {"Mail",                                "Controls whether mail can contain progress items."}},
        {ProgressPlatformsRafts,     {"Lookout Platforms and Rafts",         "Controls whether lookout platforms and rafts can contain progress items."}},
        {ProgressSubmarines,         {"Submarines",                          "Controls whether submarines can contain progress items."}},
        {ProgressEyeReefs,           {"Eye Reef Chests",                     "Controls whether the chests that appear after clearing out the eye reefs can contain progress items."}},
        {ProgressOctosGunboats,      {"Big Octos and Gunboats",              "Controls whether the items dropped by Big Octos and gunboats can contain progress items."}},
        {ProgressTriforceCharts,     {"Triforce Charts",                     "Controls whether the sunken treasure chests marked on Triforce Charts can contain progress items."}},
        {ProgressTreasureCharts,     {"Treasure Charts",                     "Controls whether the sunken treasure chests marked on Treasure Charts can contain progress items."}},
        {ProgressExpPurchases,       {"Expensive Purchases",                 "Controls whether items that cost a lot of rupees can be progress items (Rock Spire Shop, auctions, Tingle's letter, trading quest)."}},
        {ProgressMisc,               {"Miscellaneous",                       "Miscellaneous locations that don't fit into any of the other categories (outdoors chests, wind shrine, Cyclos, etc)."}},
        {ProgressTingleChests,       {"Tingle Chests",                       "Tingle Chests that are hidden in dungeons which appear when bombed."}},
        {ProgressBattlesquid,        {"Battlesquid Minigame",                "Controls whether the Windfall battleship minigame can reward progress items."}},
        {ProgressSavageLabyrinth,    {"Savage Labyrinth",                    "Controls whether the Savage Labyrinth can contain progress items."}},
        {ProgressIslandPuzzles,      {"Island Puzzles",                      "Controls whether various island puzzles can contain progress items (e.g. chests hidden in unusual places)."}},
        {ProgressDungeonSecrets,     {"Dungeon Secrets",                     "Controls whether the 11 secret items hidden in dungeons can be progress items."}},
        {ProgressObscure,            {"Obscure",                             "Controls whether obscure locations can contain progress items (e.g. Kane Windfall gate decorations)."}},

        {RemoveSwords,               {"Remove Swords",                       "Controls whether swords will be placed throughout the game."}},
        {NumRequiredDungeons,        {"Number of Required Bosses",           "The number of dungeon bosses with required items (applies to \"Standard\" and \"Race Mode\" dungeons)."}},
        {RandomCharts,               {"Randomize Charts",                    "Randomizes which sector is drawn on each Triforce/Treasure Chart."}},
        {CTMC,                       {"Chest Type Matches Contents",         "Changes the chest type to reflect its contents. A metal chest has a progress item, a key chest has a dungeon key, and a wooden chest has a non-progress item or a consumable.\nKey chests are dark wood chests that use a custom texture based on Big Key Chests. Keys for non-required dungeons in race mode will be in wooden chests."}},

        {DungeonSmallKeys,           {"Small Keys",                          "Vanilla: Small Keys will appear in their vanilla locations.\nOwn Dungeon: Small Keys can only appear in their respective dungeon.\nAny Dungeon: Small Keys can only appear in dungeon locations.\nOverworld: Small Keys can only appear in non-dungeon locations\nKeysanity: Small Keys can appear anywhere"}},
        {DungeonBigKeys,             {"Big Keys",                            "Vanilla: Big Keys will appear in their vanilla locations.\nOwn Dungeon: Big Keys can only appear in their respective dungeon.\nAny Dungeon: Big Keys can only appear in dungeon locations.\nOverworld: Big Keys can only appear in non-dungeon locations\nKeysanity: Big Keys can appear anywhere"}},
        {DungeonMapsAndCompasses,    {"Maps/Compasses",                      "Vanilla: Maps/Compasses will appear in their vanilla locations.\nOwn Dungeon: Maps/Compasses can only appear in their respective dungeon.\nAny Dungeon: Maps/Compasses can only appear in dungeon locations.\nOverworld: Maps/Compasses can only appear in non-dungeon locations\nKeysanity: Maps/Compasses can appear anywhere"}},

        {InvertCompass,              {"Invert Sea Compass X-Axis",           "Inverts the east-west direction of the compass that shows while at sea."}},
        {InstantText,                {"Instant Text Boxes",                  "Text appears instantly. The B button is changed to instantly skip through text as long as you hold it down."}},
        {FixRNG,                     {"Fix RNG",                             "Certain RNG elements will have fixed outcomes. Currently only includes Helmaroc King's attacks."}},
        {Performance,                {"Performance",                         "Adjust game code that causes lag or other performance issues. May come at the cost of visual quality.\nCurrently only affects particles."}},
        {RevealSeaChart,             {"Reveal Full Sea Chart",               "Start the game with the sea chart fully drawn out."}},
        {SkipRefights,               {"Skip Boss Rematches",                 "Removes the door in Ganon's Tower that only unlocks when you defeat the rematch versios of Gohma, Kalle Demos, Jalhalla, and Molgera."}},
        {AddShortcutWarps,           {"Add Warps Between Dungeons",          "Adds new warp pots that act as shortcuts connecting dungeons to each other directly. Each pot must be unlocked before it can be used, so you cannot use them to access dungeons early."}},
        {RemoveMusic,                {"Remove Music",                        "Mutes all in-game music."}},

        //{StartingGear,               {"",                                    ""}},
        {StartingHP,                 {"Heart Pieces",                        "Number of extra heart pieces that you start with."}},
        {StartingHC,                 {"Heart Containers",                    "Number of extra heart containers that you start with."}},
        {StartingJoyPendants,        {"Joy Pendants",                        "Number of extra joy pendants that you start with."}},
        {StartingSkullNecklaces,     {"Skull Necklaces",                     "Number of extra skull necklaces that you start with."}},
        {StartingBokoBabaSeeds,      {"Boko Baba Seeds",                     "Number of extra boko baba seeds that you start with."}},
        {StartingGoldenFeathers,     {"Golden Feathers",                     "Number of extra golden feathers that you start with."}},
        {StartingKnightsCrests,      {"Knight's Crests",                     "Number of extra knight's crests that you start with."}},
        {StartingRedChuJellys,       {"Red Chu Jellys",                      "Number of extra red Chu jellies that you start with."}},
        {StartingGreenChuJellys,     {"Green Chu Jellys",                    "Number of extra green Chu jellies that you start with."}},
        {StartingBlueChuJellys,      {"Blue Chu Jellys",                     "Number of extra blue Chu jellies that you start with."}},

        {NoSpoilerLog,               {"Do Not Generate Spoiler Log",         "Prevents the randomizer from generating a text file containing item placements. This also changes where items are placed in the seed.\nGenerating a spoiler log is highly recommended even if you don't intend to use it, in case you get stuck."}},
        {StartWithRandomItem,        {"Start With Random Item",              "Start with one extra item, selected uniformly at random from the pool.\n\nDefault item pool: Bait Bag, Bombs, Boomerang, Bow, Deku Leaf, Delivery Bag, Grappling Hook, Hookshot, Picto Box, Power Bracelets, and Skull Hammer.\nThis pool can be modified with the plandomizer."}},
        {RandomItemSlideItem,        {"Random Item Sliding Item",            "Randomly start with one first-person item to allow item sliding (Grappling Hook, Boomerang, Bow, or Hookshot). This option is aimed at glitch-heavy races where finding one of these items could massively change the outcome. If you already start with one of these items, this setting will *not* add another."}},
        {ClassicMode,                {"Classic Mode",                        "Add back behaviors and glitches that were removed in the remake. Currently includes Wind Waker dives and dry storage. Only use these if you know what you are doing!"}},
        {Plandomizer,                {"Plandomizer",                         "Allows you to provide a file which manually sets item locations and/or entrances."}},

        {HoHoHints,                  {"Place Hints on Old Man Ho Ho",        "Places hints on Old Man Ho Ho. Old Man Ho Ho appears at 10 different islands. Simply talk to Old Man Ho Ho to get hints."}},
        {KorlHints,                  {"Place Hints on King of Red Lions",    "Places hints on the King of Red Lions. Talk to the King of Red Lions to get hints."}},
        {ClearerHints,               {"Clearer Hints",                       "When this option is selected, location and item hints will use the standard check or item name, instead of using cryptic hints."}},
        {UseAlwaysHints,             {"Use Always Hints",                    "When the number of location hints is nonzero, certain locations that will always be hinted will take precedence over normal location hints."}},
        {HintImportance,             {"Hint Importance",                     "When this option is selected, item and location hints will also indicate if the hinted item is required, possibly required, or not required. Only progress items will have these additions; non-progress items are trivially not required."}},
        {PathHints,                  {"Path Hints",                          "Determines the number of path hints that will be placed."}},
        {BarrenHints,                {"Barren Hints",                        "Determines the number of barren hints that will be placed."}},
        {ItemHints,                  {"Item Hints",                          "Determines the number of location hints that will be placed."}},
        {LocationHints,              {"Location Hints",                      "Determines the number of item hints that will be placed."}},

        {RandomizeDungeonEntrances,  {"Randomize Dungeon Entrances",         "Shuffles which dungeon entrances take you into which dungeons."}},
        {RandomizeBossEntrances,     {"Randomize Boss Entrance",             "Shuffles which boss entrances take you to what boss."}},
        {RandomizeMinibossEntrances, {"Randomize Miniboss Entrances",        "Shuffles which miniboss entrances take you to what miniboss (excluding DRC)."}},
        {RandomizeCaveEntrances,     {"Randomize Cave Entrances",            "Shuffles which secret cave entrances take you into which secret caves."}},
        {RandomizeDoorEntrances,     {"Randomize Door Entrances",            "Shuffles which door entrances take you into which interiors."}},
        {RandomizeMiscEntrances,     {"Randomzie Misc Entrances",            "Shuffles entrances that do not fit into the other categories\nIncludes entrances on Forest Haven and Dragon Roost Island. Exludes Hyrule."}},
        {MixDungeons,                {"Mix Dungeons",                        "Add dungeons into the combined entrance pool, instead of keeping them separated."}},
        {MixBosses,                  {"Mix Bosses",                          "Add bosses into the combined entrance pool, instead of keeping them separated."}},
        {MixMinibosses,              {"Mix Minibosses",                      "Add minibosses into the combined entrance pool, instead of keeping them separated."}},
        {MixCaves,                   {"Mix Caves",                           "Add secret caves into the combined entrance pool, instead of keeping them separated."}},
        {MixDoors,                   {"Mix Doors",                           "Add doors into the combined entrance pool, instead of keeping them separated."}},
        {MixMisc,                    {"Mix Misc",                            "Add miscellaneous entrances into the combined entrance pool, instead of keeping them separated."}},
        {DecoupleEntrances,          {"Decouple Entrances",                  "Decouple entrances when shuffling. This means you may not end up where you came from if you go back through an entrance."}},
        {RandomStartIsland,          {"Randomize Starting Island",           "Randomzies which island you start the game on."}},

        {PigColor,                   {"Pig Color",                           "Controls the color of the big pig on Outset Island."}},

        {DamageMultiplier,           {"Damage Multiplier",                   "Change the damage multiplier used in Hero Mode. By default Hero Mode applies a 2x damage multiplier. This will not affect damage taken in Normal Mode."}},

        {TargetType,                 {"Target Type",                         "Set your in-game preference for the target type."}},
        {Camera,                     {"Camera",                              "Set your in-game preference for the camera."}},
        {FirstPersonCamera,          {"First Person Camera",                 "Set your in-game preference for the first-person camera."}},
        {Gyroscope,                  {"Gyroscope",                           "Set your in-game preference for gyroscope aiming."}},
        {UIDisplay,                  {"UI Display",                          "Set your in-game preference for the UI display."}}
    };

    if(!optionInfoMap.contains(option)) {
        return {"", {}};
    }

    return optionInfoMap.at(option);
}
