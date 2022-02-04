
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

    Settings settings;
    // Set settings in code for now
    settings.progression_dungeons = true;
    settings.progression_puzzle_secret_caves = true;
    settings.progression_combat_secret_caves = true;
    settings.progression_mail = true;
    settings.progression_dungeons = true;
    settings.progression_great_fairies = true;
    settings.progression_puzzle_secret_caves = true;
    settings.progression_combat_secret_caves = true;
    settings.progression_short_sidequests = true;
    settings.progression_long_sidequests = true;
    settings.progression_spoils_trading = true;
    settings.progression_minigames = true;
    settings.progression_free_gifts = true;
    settings.progression_mail = true;
    settings.progression_platforms_rafts = true;
    settings.progression_submarines = true;
    settings.progression_eye_reef_chests = true;
    settings.progression_big_octos_gunboats = true;
    settings.progression_triforce_charts = true;
    settings.progression_treasure_charts = true;
    settings.progression_expensive_purchases = true;
    settings.progression_misc = true;
    settings.progression_tingle_chests = true;
    settings.progression_battlesquid = true;
    settings.progression_savage_labyrinth = true;
    settings.progression_island_puzzles = true;
    settings.progression_obscure = true;
    settings.keylunacy = false;
    settings.randomize_charts = true;
    settings.race_mode = true;
    settings.num_race_mode_dungeons = 3;
    // End of in code settings

    // Create all necessary worlds (for any potential multiworld support in the future)
    int worldCount = 2;
    World blankWorld;
    WorldPool worlds (worldCount, blankWorld);
    std::vector<Settings> settingsVector (worldCount, settings);

    int retVal = generateWorlds(worlds, settingsVector, seed);

    if (retVal == 0)
    {
        std::cout << "Generating Spoiler Log" << std::endl;
        generateSpoilerLog(worlds);
    }
}
