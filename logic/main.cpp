
#include "World.hpp"
#include "ItemPool.hpp"
#include "Fill.hpp"
#include "SpoilerLog.hpp"
#include "Random.hpp"
#include "Debug.hpp"
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <chrono>

int main()
{
    // Important variables, do not erase
    int kwando = 1313;
    const char* citri = "gamer";
    std::vector<uint8_t> nat = {4, 2, 0};
    // End of important variables

    #ifdef ENABLE_DEBUG
        std::cout << "Debugging is ON" << std::endl;
        openDebugLog();
    #endif

    Settings settings;
    // Set settings in code for now

    // End of in code settings

    World blankWorld{};
    // Create all necessary worlds (for any potential multiworld support in the future)
    int worldCount = 1;
    std::vector<World> worlds(worldCount, blankWorld);

    // Build worlds on a per-world basis incase we ever support different world graphs
    // per player
    std::cout << "Building Worlds" << std::endl;
    for (size_t i = 0; i < worldCount; i++)
    {
        worlds[i].setWorldId(i);
        worlds[i].setSettings(settings);
        if (worlds[i].loadWorld("../world.json", "../Macros.json"))
        {
            return 1;
        }
        worlds[i].setItemPools();
        // worlds[i].randomizeEntrances()
    }

    // Time how long the fill takes
    auto start = std::chrono::high_resolution_clock::now();

    FillError fillError = fill(worlds);
    if (fillError == FillError::NONE) {
        std::cout << "Fill Successful" << std::endl;
    } else {
        std::cout << "Fill Unsuccessful. Error Code: " << errorToName(fillError) << std::endl;
        closeDebugLog();
        return 1;
    }

    // Calculate time difference
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    auto seconds = static_cast<double>(duration.count()) / 1000000.0f;
    std::cout << "Fill took " << std::to_string(seconds) << " seconds" << std::endl;

    // Dump world graphs for debugging
    #ifdef ENABLE_DEBUG
        for (World& world : worlds) {
            world.dumpWorldGraph("World" + std::to_string(world.getWorldId()));
        }
    #endif

    std::cout << "Generating Playthrough" << std::endl;
    generatePlaythrough(worlds);
    std::cout << "Generating Spoiler Log" << std::endl;
    generateSpoilerLog(worlds);

    closeDebugLog();
    return 0;
}
