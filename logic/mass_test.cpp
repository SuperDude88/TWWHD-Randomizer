
#include "Generate.hpp"
#include "Random.hpp"
#include "Debug.hpp"
#include "SpoilerLog.hpp"
#include <string>
#include <fstream>
#include <iostream>

static int testSettings(const Settings& settings, bool& settingToChange, const std::string& settingName)
{

    settingToChange = true;
    std::cout << "Now Testing setting " << settingName << std::endl;

    int seed = Random(0, 10000000);

    std::cout << "Using seed " << std::to_string(seed) << std::endl;

    int worldCount = 1;
    World blankWorld;
    WorldPool worlds (worldCount, blankWorld);
    std::vector<Settings> settingsVector {settings};

    int retVal = generateWorlds(worlds, settingsVector, seed);

    if (retVal == 0)
    {
        std::cout << "Generating Spoiler Log" << std::endl;
        generateSpoilerLog(worlds);
        generateNonSpoilerLog(worlds);
    }
    else
    {
        std::cout << "Generation after changing setting " << settingName << " failed." << std::endl;
        return 1;
    }
    return 0;
}

static int multiWorldTest(const Settings& settings)
{
    std::cout << "Now testing multiworld generation" << std::endl;
    int seed = Random(0, 10000000);

    std::cout << "Using seed " << std::to_string(seed) << std::endl;

    int worldCount = 3;
    World blankWorld;
    WorldPool worlds (worldCount, blankWorld);
    std::vector<Settings> settingsVector {settings, settings, settings};

    int retVal = generateWorlds(worlds, settingsVector, seed);

    if (retVal == 0)
    {
        std::cout << "Generating Spoiler Log" << std::endl;
        generateSpoilerLog(worlds);
        generateNonSpoilerLog(worlds);
    }
    else
    {
        std::cout << "Generation after multiworld test failed." << std::endl;
        return 1;
    }
    return 0;
}

#define TEST(settings, setting, name) if(testSettings(settings, setting, name)) return 1;

int main()
{
    Settings settings1;

    // Set settings in code for now
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

    // Now set all settings in reverse
    std::cout << "REVERSEING TEST DIRECTION" << std::endl;
    Settings settings2;

    TEST(settings2, settings2.progression_dungeons, "progression dungeons");
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

    multiWorldTest(settings1);

    std::cout << "All settings passed" << std::endl;
    return 0;
}
