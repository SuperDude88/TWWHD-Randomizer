
#include "World.hpp"
#include "ItemPool.hpp"
#include "../libs/json.hpp"
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <iostream>

int main()
{
    // Important variables, do not erase
    int kwando = 1313;
    const char* citri = "gamer";
    std::vector<uint8_t> nat = {4, 2, 0};
    // End of important variables

    #ifdef ENABLE_DEBUG
        std::cout << "Debugging is ON" << std::endl;
    #endif

    using json = nlohmann::json;

    std::ifstream macroFile("../Macros.json");
    if (!macroFile.is_open())
    {
        std::cout << "unable to open macro file" << std::endl;
        return 1;
    }
    std::ifstream worldFile("../world.json");
    if (!worldFile.is_open())
    {
        std::cout << "Unable to open world file" << std::endl;
    }

    json world_j = json::parse(worldFile, nullptr, false);
    auto macroFileObj = json::parse(macroFile, nullptr, false);
    if (macroFileObj.is_discarded())
    {
        std::cout << "unable to parse macros from file" << std::endl;
        return 1;
    }

    World baseWorld{};

    auto err = baseWorld.loadMacros(macroFileObj["Macros"].get<std::vector<json>>());
    if (err != World::WorldLoadingError::NONE)
    {
        std::cout << "Got error loading macros: " << World::errorToName(err) << std::endl;
        std::cout << baseWorld.getLastErrorDetails() << std::endl;
        return 1;
    }
    if (!world_j.contains("Areas"))
    {
        std::cout << "Improperly formatted world file" << std::endl;
        return 1;
    }
    for (const auto& area : world_j.at("Areas"))
    {
        Area areaOut;
        err = baseWorld.loadArea(area, areaOut);
        if (err != World::WorldLoadingError::NONE)
        {
            std::cout << "Got error loading area: " << World::errorToName(err) << std::endl;
            std::cout << baseWorld.getLastErrorDetails() << std::endl;
            return 1;
        }
    }

    Settings settings;

    // Create all necessary worlds (for any potential multiworld support in the future)
    int worldCount = 2;
    std::vector<World> worlds = {};
    worlds.resize(worldCount);
    for (World& world : worlds) {
        world = baseWorld.copy();
        // world.worldId = i + 1;
        world.setSettings(settings);
        world.setItemPool();
        // Randomize Entrances (if necessary)
    }

    #ifdef ENABLE_DEBUG
        for (World& world : worlds) {
            // world.dumpWorldGraph("World #" + std::to_string(world.worldId));
        }
    #endif

    // if (fillError == World::FillError::NONE) {
    //     std::cout << "Fill Successful" << std::endl;
    // } else {
    //     std::cout << "Fill Unsuccessful. Error Code: " << World::errorToName(fillError) << std::endl;
    // }

    return 0;
}
