#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <filesystem>

#include <logic/Generate.hpp>
#include <seedgen/random.hpp>
#include <seedgen/config.hpp>
#include <utility/path.hpp>
#include <utility/file.hpp>

Config config;
#define ERROR_CONFIG_PATH "./error_configs"

static int testSettings(const Settings& settings, bool& settingToChange, const std::string& settingName)
{

    settingToChange = true;
    std::cout << "Now Testing setting " << settingName;

    int successes = 0;
    int attempts = 10;

    for (int i = 0; i < attempts; i++)
    {
        config.seed = "LOGIC_TESTING_SEED" + std::to_string(i);
        config.settings = settings;

        const std::string permalink = config.getPermalink();
        const Seed_t integer_seed = seedFromString(permalink);
        Random_Init(integer_seed);

        int worldCount = 1;
        WorldPool worlds (worldCount);
        std::vector<Settings> settingsVector (1, settings);

        // Pre-emptively write config incase of crashing
        const fspath errorConfigFilename = ERROR_CONFIG_PATH "/" + settingName + " " + config.seed + "_error_config.yaml";
        const fspath preferencesFilename = ERROR_CONFIG_PATH "/" + settingName + " " + config.seed + "_error_preferences.yaml";
        ConfigError err = config.writeToFile(errorConfigFilename, preferencesFilename);

        int retVal = generateWorlds(worlds, settingsVector);

        if (retVal == 0)
        {
            std::cout << ".";
            successes += 1;
            // Delete files if there were no errors
            std::filesystem::remove(errorConfigFilename);
            std::filesystem::remove(preferencesFilename);
        }
        else
        {
            std::cout << "F";
        }
    }

    int successRate = static_cast<float>(successes) / static_cast<float>(attempts) * 100.0f;
    std::cout << "Success Rate: " << std::to_string(successRate) << "%" << std::endl;

    // Somewhat arbitrarily chosen, but ideally even the most prone to failing settings
    // shouldn't fail this much
    if (successes < 7)
    {
        std::cout << "Success rate after changing setting \"" << settingName << "\" is too poor.\nSettings saved to error_configs folder" << std::endl;
        exit(1);
    }

    return 0;
}

static int multiWorldTest(const Settings& settings)
{
    std::cout << "Now testing multiworld generation" << std::endl;

    Config localConf;
    localConf.seed = "LOGIC_TESTING_SEED";
    localConf.settings = settings;
    const std::string permalink = localConf.getPermalink();
    const Seed_t integer_seed = seedFromString(permalink);

    Random_Init(integer_seed);

    int worldCount = 3;
    WorldPool worlds (worldCount);
    std::vector<Settings> settingsVector (worldCount, settings);

    int retVal = generateWorlds(worlds, settingsVector);

    if (retVal != 0)
    {
        std::cout << "Generation after multiworld test failed." << std::endl;
        exit(1);
    }
    return 0;
}

#define TEST(settings, setting, name) if(testSettings(settings, setting, name)) allPassed = false;

void runLogicTests(Config& newConfig)
{
    Utility::create_directories(ERROR_CONFIG_PATH);

    bool allPassed = true;
    config = std::move(newConfig);
    Settings settings1;
    settings1.starting_gear = {GameItem::SongOfPassing};

    // Test settings 1 by 1
    bool dummy = false;
    settings1.progression_dungeons = ProgressionDungeons::Standard;
    TEST(settings1, dummy, "progression dungeons");
    TEST(settings1, settings1.progression_great_fairies, "progression great faires");
    TEST(settings1, settings1.progression_puzzle_secret_caves, "progression puzzle secret caves");
    TEST(settings1, settings1.progression_combat_secret_caves, "progression combat secret caves");
    TEST(settings1, settings1.progression_short_sidequests, "progression short sidequests");
    TEST(settings1, settings1.progression_long_sidequests, "progression long sidequests");
    TEST(settings1, settings1.progression_spoils_trading, "progression spoils trading");
    TEST(settings1, settings1.progression_minigames, "progression minigames");
    TEST(settings1, settings1.progression_free_gifts, "progression free gifts");
    TEST(settings1, settings1.progression_mail, "progression mail");
    TEST(settings1, settings1.progression_platforms_rafts, "progression platforms rafts");
    TEST(settings1, settings1.progression_submarines, "progression submarines");
    TEST(settings1, settings1.progression_eye_reef_chests, "progression eye reef chests");
    TEST(settings1, settings1.progression_big_octos_gunboats, "progression big octos gunboats");
    TEST(settings1, settings1.progression_triforce_charts, "progression triforce charts");
    TEST(settings1, settings1.progression_treasure_charts, "progression treasure charts");
    TEST(settings1, settings1.progression_expensive_purchases, "progression expensive purchases");
    TEST(settings1, settings1.progression_misc, "progression misc");
    TEST(settings1, settings1.progression_tingle_chests, "progression tingle chests");
    TEST(settings1, settings1.progression_battlesquid, "progression battlesquid");
    TEST(settings1, settings1.progression_savage_labyrinth, "progression savage labyrinth");
    TEST(settings1, settings1.progression_island_puzzles, "progression island puzzles");
    TEST(settings1, settings1.progression_obscure, "progression obscure");
    settings1.num_required_dungeons = 1;
    settings1.progression_dungeons = ProgressionDungeons::RaceMode;
    TEST(settings1, dummy, "race mode 1 dungeon");
    settings1.num_required_dungeons = 2;
    TEST(settings1, dummy, "race mode 2 dungeon");
    settings1.num_required_dungeons = 3;
    TEST(settings1, dummy, "race mode 3 dungeon");
    settings1.num_required_dungeons = 4;
    TEST(settings1, dummy, "race mode 4 dungeon");
    settings1.num_required_dungeons = 5;
    TEST(settings1, dummy, "race mode 5 dungeon");
    settings1.num_required_dungeons = 6;
    TEST(settings1, dummy, "race mode 6 dungeon");
    settings1.dungeon_small_keys = PlacementOption::OwnDungeon;
    TEST(settings1, dummy, "dungeon small keys Own Dungeon");
    settings1.dungeon_small_keys = PlacementOption::AnyDungeon;
    TEST(settings1, dummy, "dungeon small keys Any Dungeon");
    settings1.dungeon_small_keys = PlacementOption::Overworld;
    TEST(settings1, dummy, "dungeon small keys Overworld");
    settings1.dungeon_small_keys = PlacementOption::Keysanity;
    TEST(settings1, dummy, "dungeon small keys Keysanity");
    settings1.progression_dungeons = ProgressionDungeons::Standard;
    TEST(settings1, dummy, "switching to standard");
    settings1.dungeon_big_keys = PlacementOption::OwnDungeon;
    TEST(settings1, dummy, "dungeon big keys Own Dungeon");
    settings1.dungeon_big_keys = PlacementOption::AnyDungeon;
    TEST(settings1, dummy, "dungeon big keys Any Dungeon");
    settings1.dungeon_big_keys = PlacementOption::Overworld;
    TEST(settings1, dummy, "dungeon big keys Overworld");
    settings1.dungeon_big_keys = PlacementOption::Keysanity;
    TEST(settings1, dummy, "dungeon big keys Keysanity");
    settings1.progression_dungeons = ProgressionDungeons::RaceMode;
    TEST(settings1, dummy, "switching to race mode");
    settings1.dungeon_maps_compasses = PlacementOption::OwnDungeon;
    TEST(settings1, dummy, "dungeon maps compasses Own Dungeon");
    settings1.dungeon_maps_compasses = PlacementOption::AnyDungeon;
    TEST(settings1, dummy, "dungeon maps compasses Any Dungeon");
    settings1.dungeon_maps_compasses = PlacementOption::Overworld;
    TEST(settings1, dummy, "dungeon maps compasses Overworld");
    settings1.dungeon_maps_compasses = PlacementOption::Keysanity;
    TEST(settings1, dummy, "dungeon maps compasses Keysanity");
    settings1.path_hints = 5;
    TEST(settings1, settings1.korl_hints, "5 path hints");
    settings1.barren_hints = 5;
    TEST(settings1, settings1.korl_hints, "5 barren hints");
    settings1.item_hints = 5;
    TEST(settings1, settings1.ho_ho_hints, "5 item hints");
    settings1.location_hints = 5;
    TEST(settings1, settings1.ho_ho_hints, "5 location hints");
    TEST(settings1, settings1.kreeb_bow_hints, "kreeb bow hints");
    TEST(settings1, settings1.use_always_hints, "use always hints");
    TEST(settings1, settings1.clearer_hints, "clearer hints");
    TEST(settings1, settings1.hint_importance, "hint importance");
    TEST(settings1, settings1.open_drc, "open_drc");
    TEST(settings1, settings1.randomize_charts, "randomize charts");
    TEST(settings1, settings1.required_boss_items, "required_boss_items");
    TEST(settings1, settings1.randomize_starting_island, "random starting island");
    TEST(settings1, settings1.randomize_dungeon_entrances, "randomize dungeon entrances");
    settings1.randomize_cave_entrances = ShuffleCaveEntrances::Disabled;
    TEST(settings1, dummy, "randomize cave entrances");
    TEST(settings1, settings1.randomize_door_entrances, "randomize door entrances");
    TEST(settings1, settings1.randomize_misc_entrances, "randomize misc entrances");
    TEST(settings1, settings1.decouple_entrances, "decouple entrances");
    TEST(settings1, settings1.mix_dungeons, "mix dungeons");
    TEST(settings1, settings1.mix_bosses, "mix bosses");
    TEST(settings1, settings1.mix_caves, "mix caves");
    TEST(settings1, settings1.mix_doors, "mix doors");
    TEST(settings1, settings1.mix_misc, "mix misc");

    // Now set all settings in reverse (except dungeons since they have a lot of checks)
    std::cout << "REVERSING TEST DIRECTION" << std::endl;
    Settings settings2;
    settings2.starting_gear = {GameItem::BalladOfGales};

    settings2.progression_dungeons = ProgressionDungeons::Standard;
    TEST(settings2, dummy, "progression dungeons");
    TEST(settings2, settings2.mix_misc, "mix misc");
    TEST(settings2, settings2.mix_doors, "mix doors");
    TEST(settings2, settings2.mix_caves, "mix caves");
    TEST(settings2, settings2.mix_bosses, "mix bosses");
    TEST(settings2, settings2.mix_dungeons, "mix dungeons");
    TEST(settings2, settings2.randomize_misc_entrances, "randomize misc entrances");
    TEST(settings2, settings2.randomize_door_entrances, "randomize door entrances");
    settings2.randomize_cave_entrances = ShuffleCaveEntrances::Disabled;
    TEST(settings2, dummy, "randomize cave entrances");
    TEST(settings2, settings2.randomize_dungeon_entrances, "randomize dungeon entrances");
    TEST(settings2, settings2.randomize_starting_island, "randomize starting island");
    TEST(settings1, settings1.required_boss_items, "required_boss_items");
    TEST(settings2, settings2.randomize_charts, "randomize charts");
    TEST(settings2, settings2.open_drc, "open_drc");
    TEST(settings2, settings2.hint_importance, "hint importance");
    TEST(settings2, settings2.clearer_hints, "clearer hints");
    TEST(settings2, settings2.use_always_hints, "use always hints");
    TEST(settings2, settings2.kreeb_bow_hints, "kreeb bow hints");
    settings2.path_hints = 5;
    TEST(settings2, settings2.korl_hints, "5 path hints");
    settings2.barren_hints = 5;
    TEST(settings2, settings2.korl_hints, "5 barren hints");
    settings2.item_hints = 5;
    TEST(settings2, settings2.ho_ho_hints, "5 item hints");
    settings2.location_hints = 5;
    TEST(settings2, settings2.ho_ho_hints, "5 location hints");
    settings2.num_required_dungeons = 3;
    TEST(settings2, dummy, "race mode 3 dungeon");
    settings2.num_required_dungeons = 4;
    TEST(settings2, dummy, "race mode 4 dungeon");
    settings2.num_required_dungeons = 5;
    TEST(settings2, dummy, "race mode 5 dungeon");
    settings2.num_required_dungeons = 6;
    TEST(settings2, dummy, "race mode 6 dungeon");
    settings2.progression_dungeons = ProgressionDungeons::Standard;
    TEST(settings2, dummy, "switching to standard");
    settings2.dungeon_maps_compasses = PlacementOption::Keysanity;
    TEST(settings2, dummy, "dungeon maps compasses Keysanity");
    settings2.dungeon_maps_compasses = PlacementOption::Overworld;
    TEST(settings2, dummy, "dungeon maps compasses Overworld");
    settings2.dungeon_maps_compasses = PlacementOption::AnyDungeon;
    TEST(settings2, dummy, "dungeon maps compasses Any Dungeon");
    settings2.dungeon_maps_compasses = PlacementOption::OwnDungeon;
    TEST(settings2, dummy, "dungeon maps compasses Own Dungeon");
    settings2.progression_dungeons = ProgressionDungeons::RaceMode;
    TEST(settings2, dummy, "switching to race mode");
    settings2.dungeon_big_keys = PlacementOption::Keysanity;
    TEST(settings2, dummy, "dungeon big keys Keysanity");
    settings2.dungeon_big_keys = PlacementOption::AnyDungeon;
    TEST(settings2, dummy, "dungeon big keys Any Dungeon");
    settings2.dungeon_big_keys = PlacementOption::OwnDungeon;
    TEST(settings2, dummy, "dungeon big keys Own Dungeon");
    settings2.dungeon_small_keys = PlacementOption::Keysanity;
    TEST(settings2, dummy, "dungeon small keys Keysanity");
    settings2.progression_dungeons = ProgressionDungeons::Standard;
    TEST(settings2, dummy, "switching to standard");
    settings2.dungeon_small_keys = PlacementOption::AnyDungeon;
    TEST(settings2, dummy, "dungeon small keys Any Dungeon");
    settings2.dungeon_small_keys = PlacementOption::OwnDungeon;
    TEST(settings2, dummy, "dungeon small keys Own Dungeon");
    TEST(settings2, settings2.progression_obscure, "progression obscure");
    TEST(settings2, settings2.progression_island_puzzles, "progression island puzzles");
    TEST(settings2, settings2.progression_savage_labyrinth, "progression savage labyrinth");
    TEST(settings2, settings2.progression_battlesquid, "progression battlesquid");
    TEST(settings2, settings2.progression_tingle_chests, "progression tingle chests");
    TEST(settings2, settings2.progression_misc, "progression misc");
    TEST(settings2, settings2.progression_expensive_purchases, "progression expensive purchases");
    TEST(settings2, settings2.progression_treasure_charts, "progression treasure charts");
    TEST(settings2, settings2.progression_triforce_charts, "progression triforce charts");
    TEST(settings2, settings2.progression_big_octos_gunboats, "progression big octos gunboats");
    TEST(settings2, settings2.progression_eye_reef_chests, "progression eye reef chests");
    TEST(settings2, settings2.progression_submarines, "progression submarines");
    TEST(settings2, settings2.progression_platforms_rafts, "progression platforms rafts");
    TEST(settings2, settings2.progression_mail, "progression mail");
    TEST(settings2, settings2.progression_free_gifts, "progression free gifts");
    TEST(settings2, settings2.progression_minigames, "progression minigames");
    TEST(settings2, settings2.progression_spoils_trading, "progression spoils trading");
    TEST(settings2, settings2.progression_long_sidequests, "progression long sidequests");
    TEST(settings2, settings2.progression_short_sidequests, "progression short sidequests");
    TEST(settings2, settings2.progression_combat_secret_caves, "progression combat secret caves");
    TEST(settings2, settings2.progression_puzzle_secret_caves, "progression puzzle secret caves");
    TEST(settings2, settings2.progression_great_fairies, "progression great faires");
    settings2.progression_dungeons = ProgressionDungeons::Disabled;
    TEST(settings2, dummy, "disabled dungeons");

    multiWorldTest(settings1);

    std::cout << "Success rate acceptable" << std::endl;
}

// Tests how often a settings configuration succeeds
void testSettings(Config& newConfig, int testCount /*= 1*/)
{
    config = std::move(newConfig);
    int successfulTests = 0;
    for (int i = 0; i < testCount; i++)
    {
        config.seed = std::to_string(Random(0, 10000000));
        const std::string permalink = config.getPermalink();
        const Seed_t integer_seed = seedFromString(permalink);

        std::cout << "Testing with seed \"" << config.seed << "\"..." << std::flush;

        Random_Init(integer_seed);

        int worldCount = 1;
        WorldPool worlds (worldCount);
        std::vector<Settings> settingsVector (1, config.settings);

        int retVal = generateWorlds(worlds, settingsVector);

        if (retVal == 0)
        {
            successfulTests++;
            std::cout << "Passed" << std::endl;
        }
        else
        {
            std::cout << "Failed" << std::endl;
        }
    }

    int successRate = static_cast<float>(successfulTests) / static_cast<float>(testCount) * 100.0f;

    std::cout << "Passed " << std::to_string(successfulTests) << "/" << std::to_string(testCount) << " tests. Success Rate: " << std::to_string(successRate) << "%" << std::endl;
}
