
#include "Generate.hpp"
#include "../seedgen/random.hpp"
#include "../seedgen/permalink.hpp"
#include "../seedgen/config.hpp"
#include "../server/command/Log.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>

Config config;

static int testSettings(const Settings& settings, bool& settingToChange, const std::string& settingName)
{

    settingToChange = true;
    std::cout << "Now Testing setting " << settingName;

    const std::string seed = std::to_string(Random(0, 10000000));
    std::cout << " using seed \"" << seed << "\"" << std::endl;

    auto permalink = create_permalink(settings, seed);
    std::hash<std::string> strHash;
    auto integer_seed = strHash(permalink);

    Random_Init(integer_seed);

    config.settings = settings;
    config.seed = seed;
    ConfigError err = writeToFile("error_config.yaml", config);
    if (err != ConfigError::NONE)
    {
        std::cout << "Could not write error_config to file" << std::endl;
        return 1;
    }

    int worldCount = 1;
    WorldPool worlds (worldCount);
    std::vector<Settings> settingsVector (1, settings);

    int retVal = generateWorlds(worlds, settingsVector);

    if (retVal != 0)
    {
        std::cout << "Generation after changing setting \"" << settingName << "\" failed.\nSettings saved to \"error_config.yaml\"" << std::endl;
        return 1;
    }
    return 0;
}

static int multiWorldTest(const Settings& settings)
{
    std::cout << "Now testing multiworld generation";

    const std::string seed = std::to_string(Random(0, 10000000));
    std::cout << " using seed \"" << seed << "\"" << std::endl;

    auto permalink = create_permalink(settings, seed);
    std::hash<std::string> strHash;
    auto integer_seed = strHash(permalink);

    Random_Init(integer_seed);

    int worldCount = 3;
    WorldPool worlds (worldCount);
    std::vector<Settings> settingsVector (worldCount, settings);

    int retVal = generateWorlds(worlds, settingsVector);

    if (retVal != 0)
    {
        std::cout << "Generation after multiworld test failed." << std::endl;
        return 1;
    }
    return 0;
}

#define TEST(settings, setting, name) if(testSettings(settings, setting, name)) return;

void massTest(Config& newConfig)
{
    config = std::move(newConfig);
    Settings settings1;
    settings1.starting_gear = {GameItem::SongOfPassing};

    // Test settings 1 by 1
    TEST(settings1, settings1.progression_dungeons, "progression dungeons");
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
    settings1.num_race_mode_dungeons = 1;
    TEST(settings1, settings1.race_mode, "race mode 1 dungeon");
    settings1.num_race_mode_dungeons = 2;
    TEST(settings1, settings1.race_mode, "race mode 2 dungeon");
    settings1.num_race_mode_dungeons = 3;
    TEST(settings1, settings1.race_mode, "race mode 3 dungeon");
    settings1.num_race_mode_dungeons = 4;
    TEST(settings1, settings1.race_mode, "race mode 4 dungeon");
    settings1.num_race_mode_dungeons = 5;
    TEST(settings1, settings1.race_mode, "race mode 5 dungeon");
    settings1.num_race_mode_dungeons = 6;
    TEST(settings1, settings1.race_mode, "race mode 6 dungeon");
    TEST(settings1, settings1.keylunacy, "keylunacy");
    settings1.path_hints = 5;
    TEST(settings1, settings1.korl_hints, "5 path hints");
    settings1.barren_hints = 5;
    TEST(settings1, settings1.korl_hints, "5 barren hints");
    settings1.item_hints = 5;
    TEST(settings1, settings1.ho_ho_hints, "5 item hints");
    settings1.location_hints = 5;
    TEST(settings1, settings1.ho_ho_hints, "5 loaction hints");
    TEST(settings1, settings1.use_always_hints, "use always hints");
    TEST(settings1, settings1.clearer_hints, "clearer hints");
    TEST(settings1, settings1.randomize_charts, "randomize charts");
    TEST(settings1, settings1.randomize_starting_island, "random starting island");
    TEST(settings1, settings1.randomize_dungeon_entrances, "randomize dungeon entrances");
    TEST(settings1, settings1.randomize_cave_entrances, "randomize cave entrances");
    TEST(settings1, settings1.randomize_door_entrances, "randomize door entrances");
    TEST(settings1, settings1.randomize_misc_entrances, "randomize misc entrances");
    TEST(settings1, settings1.decouple_entrances, "decouple entrances");
    TEST(settings1, settings1.mix_entrance_pools, "mix entrance pools");
    TEST(settings1, settings1.plandomizer, "plandomizer");

    // Now set all settings in reverse (except dungeons since they have a lot of checks)
    std::cout << "REVERSING TEST DIRECTION" << std::endl;
    Settings settings2;
    settings2.starting_gear = {GameItem::BalladOfGales};

    TEST(settings2, settings2.progression_dungeons, "progression dungeons");
    TEST(settings2, settings2.mix_entrance_pools, "mix entrance pools");
    TEST(settings2, settings2.randomize_misc_entrances, "randomize misc entrances");
    TEST(settings2, settings2.randomize_door_entrances, "randomize door entrances");
    TEST(settings2, settings2.randomize_cave_entrances, "randomize cave entrances");
    TEST(settings2, settings2.randomize_dungeon_entrances, "randomize dungeon entrances");
    TEST(settings2, settings2.randomize_starting_island, "randomize starting island");
    TEST(settings2, settings2.randomize_charts, "randomize charts");
    TEST(settings2, settings2.clearer_hints, "clearer hints");
    TEST(settings2, settings2.use_always_hints, "use always hints");
    settings2.path_hints = 5;
    TEST(settings2, settings2.korl_hints, "5 path hints");
    settings2.barren_hints = 5;
    TEST(settings2, settings2.korl_hints, "5 barren hints");
    settings2.item_hints = 5;
    TEST(settings2, settings2.ho_ho_hints, "5 item hints");
    settings2.location_hints = 5;
    TEST(settings2, settings2.ho_ho_hints, "5 loaction hints");
    TEST(settings2, settings2.keylunacy, "keylunacy");
    settings2.num_race_mode_dungeons = 3;
    TEST(settings2, settings2.race_mode, "race mode 3 dungeon");
    settings2.num_race_mode_dungeons = 4;
    TEST(settings2, settings2.race_mode, "race mode 4 dungeon");
    settings2.num_race_mode_dungeons = 5;
    TEST(settings2, settings2.race_mode, "race mode 5 dungeon");
    settings2.num_race_mode_dungeons = 6;
    TEST(settings2, settings2.race_mode, "race mode 6 dungeon");
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
    TEST(settings2, settings2.plandomizer, "plandomizer");

    multiWorldTest(settings1);

    std::cout << "All settings tests passed" << std::endl;

    // Delete error_config if everything passes
    std::filesystem::remove("./error_config.yaml");
}
