
#include "Generate.hpp"
#include "Random.hpp"
#include "Debug.hpp"
#include "SpoilerLog.hpp"
#include <string>
#include <fstream>
#include <iostream>

int main()
{
    // Important variables, do not erase
    int kwando = 1313;
    const char* citri = "gamer";
    std::vector<uint8_t> nat = {4, 2, 0};
    // End of important variables

    int seed = Random(0, 100000);

    #ifdef ENABLE_DEBUG
        std::cout << "Debugging is ON" << std::endl;
        openDebugLog(std::to_string(seed));
    #endif

    std::cout << "Using seed " << std::to_string(seed) << std::endl;

    Settings settings1;
    Settings settings2;

    // Set settings in code for now
    settings1.progression_dungeons = true;
    settings1.progression_mail = true;
    settings1.progression_dungeons = true;
    settings1.progression_great_fairies = true;
    settings1.progression_puzzle_secret_caves = true;
    settings1.progression_combat_secret_caves = true;
    settings1.progression_submarines = true;
    settings1.progression_eye_reef_chests = true;
    settings1.progression_big_octos_gunboats = true;
    settings1.progression_triforce_charts = true;
    settings1.progression_treasure_charts = true;
    settings1.progression_expensive_purchases = true;
    settings1.progression_misc = true;
    settings1.progression_tingle_chests = true;
    settings1.progression_battlesquid = true;
    settings1.progression_savage_labyrinth = true;
    settings1.progression_island_puzzles = true;
    settings1.progression_obscure = true;
    settings1.keylunacy = true;
    settings1.randomize_charts = true;
    settings1.race_mode = true;
    settings1.num_race_mode_dungeons = 3;

    settings2.progression_dungeons = true;
    settings2.progression_mail = true;
    settings2.progression_dungeons = true;
    settings2.progression_great_fairies = true;
    settings2.progression_puzzle_secret_caves = true;
    settings2.progression_combat_secret_caves = true;
    settings2.progression_short_sidequests = true;
    settings2.progression_long_sidequests = true;
    settings2.progression_spoils_trading = true;
    settings2.progression_minigames = true;
    settings2.progression_free_gifts = true;
    settings2.progression_platforms_rafts = true;
    settings2.progression_submarines = true;
    settings2.progression_eye_reef_chests = true;
    settings2.progression_big_octos_gunboats = true;
    settings2.progression_triforce_charts = true;
    settings2.keylunacy = false;
    settings2.randomize_charts = true;
    settings2.race_mode = true;
    settings2.num_race_mode_dungeons = 5;
    // End of in code settings

    // Create all necessary worlds (for any potential multiworld support in the future)
    int worldCount = 1;
    World blankWorld;
    WorldPool worlds (worldCount, blankWorld);
    std::vector<Settings> settingsVector {settings1};

    int retVal = generateWorlds(worlds, settingsVector, seed);

    if (retVal == 0)
    {
        std::cout << "Generating Spoiler Log" << std::endl;
        generateSpoilerLog(worlds);
        generateNonSpoilerLog(worlds);
    }
}
