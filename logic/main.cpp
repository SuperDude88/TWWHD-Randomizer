
#include "World.hpp"
#include "ItemPool.hpp"
#include "Fill.hpp"
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
    #ifndef ENABLE_DEBUG
        std::cout << "Debugging is OFF" << std::endl;
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

    Settings settings;

    World blankWorld{};

    // Create all necessary worlds (for any potential multiworld support in the future)
    int worldCount = 2;
    std::vector<World> worlds(worldCount, blankWorld);

    // Load worlds on a per-world basis incase we ever support different worlds
    // per player
    for (size_t i = 0; i < worldCount; i++)
    {
        worlds[i].setWorldId(i);
        auto err = worlds[i].loadMacros(macroFileObj["Macros"].get<std::vector<json>>());
        if (err != World::WorldLoadingError::NONE)
        {
            std::cout << "Got error loading macros: " << World::errorToName(err) << std::endl;
            std::cout << worlds[i].getLastErrorDetails() << std::endl;
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
            err = worlds[i].loadArea(area, areaOut);
            if (err != World::WorldLoadingError::NONE)
            {
                std::cout << "Got error loading area: " << World::errorToName(err) << std::endl;
                std::cout << worlds[i].getLastErrorDetails() << std::endl;
                return 1;
            }
        }
        worlds[i].setSettings(settings);
        worlds[i].setItemPools();
    }

    FillError fillError = fill(worlds);

    #ifdef ENABLE_DEBUG
        for (World& world : worlds) {
            world.dumpWorldGraph("World" + std::to_string(world.getWorldId()));
        }
    #endif

    if (fillError == FillError::NONE) {
        std::cout << "Fill Successful" << std::endl;
    } else {
        std::cout << "Fill Unsuccessful. Error Code: " << errorToName(fillError) << std::endl;
        return 1;
    }

    return 0;
}
