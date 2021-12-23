
#include "LocationManager.hpp"
#include "../libs/json.hpp"
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <iostream>

constexpr const char* macro = R"~({
    "Name": "Can Buy Bait",
    "Expression": {
        "type": "has_item", "args": ["Nothing"]
    }
})~";

constexpr const char* req = R"~({
    "type": "and", "args": [{"type": "has_item", "args": ["BalladOfGales"]}, {"type": "has_item", "args":["WindWaker"]}, {"type": "macro", "args": ["Can Play WR"]}]
    })~";

constexpr const char* locationStr = R"~({
    "Name": "OutsetDigBlackSoil",
    "OriginalItem": "PieceOfHeart",
    "Category": [
        "IslandPuzzle"
    ],
    "Needs": {
        "type": "and",
        "args": [
            {
                "type": "macro",
                "args": [
                    "Can Buy Bait"
                ]
            },
            {
                "type": "has_item",
                "args": [
                    "BaitBag"
                ]
            },
            {
                "type": "has_item",
                "args": [
                    "PowerBracelets"
                ]
            }
        ]
    },
    "Path": "szs_permanent2/sea_Room44.zs",
    "Type": "SCOB",
    "Offsets": [
        "0x84CBD4"
    ]
})~";


int main()
{
    int kwando = 1313;
    const char* citri = "gamer";
    std::vector<uint8_t> nat = {4, 2, 0};

    using json = nlohmann::json;

    std::ifstream macroFile("../Macros.json");
    if (!macroFile.is_open())
    {
        std::cout << "unable to open macro file" << std::endl;
        return 1;
    }
    std::ifstream locationsFile("../locations.json");
    if (!locationsFile.is_open())
    {
        std::cout << "Unable to open Locations file" << std::endl;
    }

    json req_j = json::parse(req, nullptr, false);
    json loc_j = json::parse(locationsFile, nullptr, false);
    auto macroFileObj = json::parse(macroFile, nullptr, false);
    if (macroFileObj.is_discarded())
    {
        std::cout << "unable to parse macros from file" << std::endl;
        return 1;
    }

    LocationManager manager{};
    auto err = manager.loadMacros(macroFileObj["Macros"].get<std::vector<json>>());
    if (err != LocationManager::LocationError::NONE)
    {
        std::cout << "Got error loading macros: " << LocationManager::errorToName(err) << std::endl;
        std::cout << manager.getLastErrorDetails() << std::endl;
        return 1;
    }
    if (!loc_j.contains("Locations"))
    {
        std::cout << "Improperly formatted locations file" << std::endl;
        return 1;
    }
    for (const auto& location : loc_j.at("Locations"))
    {
        Location locOut;
        err = manager.loadLocation(location, locOut);
        if (err != LocationManager::LocationError::NONE)
        {
            std::cout << "Got error loading location: " << LocationManager::errorToName(err) << std::endl;
            std::cout << manager.getLastErrorDetails() << std::endl;
            return 1;
        }
    }

    LocationManager::Settings settings;
    LocationManager::ItemSet items;

    // add all the items with max count; make a function in one of the modules later?
    for (uint32_t idx = 0; idx < LocationManager::LOCATION_COUNT; idx++)
    {
        GameItem toAdd = indexToGameItem(idx);
        int countToAdd = maxItemCount(toAdd);
        for (int count = 0; count < countToAdd; count++)
        {
            items.insert(toAdd);
        }
    }

    manager.assumedFill(items, settings);

    return 0;
}