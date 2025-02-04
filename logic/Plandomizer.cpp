#include "Plandomizer.hpp"

#include <logic/Area.hpp>
#include <command/Log.hpp>
#include <utility/file.hpp>
#include <utility/platform.hpp>

PlandomizerError loadPlandomizer(const fspath& plandoFilepath, std::vector<Plandomizer>& plandos, size_t numWorlds)
{
    LOG_TO_DEBUG("Loading plandomizer file");

    std::string plandoStr;
    if (Utility::getFileContents(plandoFilepath, plandoStr) != 0)
    {
        Utility::platformLog("Will skip using plando file");
        return PlandomizerError::NONE;
    }

    // Go through and make plandomizer objects for each world
    for (size_t i = 0; i < numWorlds; i++)
    {
        LOG_TO_DEBUG("Loading Plando data for world " + std::to_string(i + 1))
        plandos[i] = Plandomizer();
        auto& plandomizer = plandos[i];
        YAML::Node plandoTree = YAML::Load(plandoStr);
        YAML::Node plandoLocations;
        YAML::Node plandoEntrances;
        YAML::Node randomStartingItemPool;
        std::string plandoStartingIsland = "";
        std::string worldName = "World " + std::to_string(i + 1);
        // Grab the YAML object which holds the plando info for this world.
        if (plandoTree[worldName] && plandoTree[worldName].IsMap())
        {
            if (plandoTree[worldName]["locations"] && plandoTree[worldName]["locations"].IsMap())
            {
                plandoLocations = plandoTree[worldName]["locations"];
            }

            if (plandoTree[worldName]["entrances"] && plandoTree[worldName]["entrances"].IsMap())
            {
                plandoEntrances = plandoTree[worldName]["entrances"];
            }
            if (plandoTree[worldName]["starting island"] && plandoTree[worldName]["starting island"].IsScalar())
            {
                plandoStartingIsland = plandoTree[worldName]["starting island"].as<std::string>();
            }
            if (plandoTree[worldName]["random starting item pool"] && plandoTree[worldName]["random starting item pool"].IsSequence())
            {
                randomStartingItemPool = plandoTree[worldName]["random starting item pool"];
            }
        }

        // Process starting island
        if (!plandoStartingIsland.empty())
        {
            plandomizer.startingIslandRoomNum = islandNameToRoomNum(plandoStartingIsland);
            if (plandomizer.startingIslandRoomNum == 0)
            {
                ErrorLog::getInstance().log("Plandomizer Error: Starting island name \"" + plandoStartingIsland + "\" is not recognized");
                return PlandomizerError::BAD_STARTING_ISLAND;
            }
            LOG_TO_DEBUG("  Setting starting island to " + plandoStartingIsland);
        }

        // Process random starting item pool
        if (randomStartingItemPool)
        {
            LOG_TO_DEBUG("  Starting Item Pool: ")
            for (const auto& item : randomStartingItemPool)
            {
                const std::string itemName = item.as<std::string>();
                const GameItem gameItem = nameToGameItem(itemName);
                if (gameItem == GameItem::INVALID)
                {
                    ErrorLog::getInstance().log("Plandomizer Error: Unknown item name \"" + itemName + "\" in random starting item pool");
                    return PlandomizerError::UNKNOWN_ITEM_NAME;
                }
                else if (!getSupportedStartingItems().contains(gameItem))
                {
                    ErrorLog::getInstance().log("Plandomizer Error: The item \"" + itemName + "\" is currently not supported as a starting item");
                    return PlandomizerError::BAD_STARTING_ITEM;
                }
                plandomizer.randomStartingItemPool.push_back(gameItem);
                LOG_TO_DEBUG(std::string("    ") + itemName);
            }
        }

        // Process Locations
        if (plandoLocations)
        {
            for (const auto& locationObject : plandoLocations)
            {
                if (locationObject.first.IsNull())
                {
                    ErrorLog::getInstance().log("Plandomizer Error: One of the plando items is missing a location");
                    return PlandomizerError::MISSING_PLANDO_LOCATION;
                }
                // If the location object has children instead of a value, then parse
                // the item name and potential world id from those children.
                // If no world id is given, then the current world's id will be used.
                int plandoWorldId = i;
                std::string itemName;
                if (locationObject.second.IsMap())
                {
                    if (locationObject.second["item"])
                    {
                        itemName = locationObject.second["item"].as<std::string>();
                    }
                    else
                    {
                        ErrorLog::getInstance().log("Plandomizer Error: Missing key \"item\" in location \"" + locationObject.first.as<std::string>() + "\"");
                        return PlandomizerError::MISSING_ITEM_KEY;
                    }
                    if (locationObject.second["world"])
                    {
                        plandoWorldId = std::stoi(locationObject.second["world"].as<std::string>());
                        if (static_cast<size_t>(plandoWorldId) > numWorlds || plandoWorldId < 1)
                        {
                            ErrorLog::getInstance().log("Plandomizer Error: Bad World ID \"" + std::to_string(plandoWorldId) + "\"");
                            ErrorLog::getInstance().log("Only " + std::to_string(numWorlds) + " worlds are being generated");
                            ErrorLog::getInstance().log("Valid World IDs: 1-" + std::to_string(numWorlds));
                            return PlandomizerError::INVALID_WORLD_ID;
                        }
                        plandoWorldId--;
                    }
                }
                // Otherwise treat the value as an item for the same world as the location
                else
                {
                    itemName = locationObject.second.as<std::string>();
                    if (itemName == "\n" || itemName.empty())
                    {
                        ErrorLog::getInstance().log("Plandomizer Error: Location \"" + locationObject.first.as<std::string>() + "\" has no plandomized item.");
                        return PlandomizerError::NO_ITEM_AT_LOCATION;
                    }
                }

                // Get location name
                const std::string locationName = locationObject.first.as<std::string>();

                // Sanitize item name incase the user missed an apostraphe
                itemName = gameItemToName(nameToGameItem(itemName));

                // Get GameItem
                auto gameItem = nameToGameItem(itemName);

                plandomizer.locationsStr.insert({locationName, {gameItem, plandoWorldId}});
                LOG_TO_DEBUG("  Plandomizing " +  itemName + " [W" + std::to_string(plandoWorldId + 1) + "] to " + locationName + " [W" + std::to_string(i + 1) + "]");
            }
        }

        // Process Entrances
        if (plandoEntrances)
        {
            for (const auto entrance : plandoEntrances)
            {
                if (entrance.first.IsNull())
                {
                    ErrorLog::getInstance().log("Plandomizer Error: One of the plando entrances is missing a parent entrance");
                    return PlandomizerError::MISSING_PARENT_ENTRANCE;
                }
                // Process strings of each plando's entrance into their respective entrance pointers
                plandomizer.entrancesStr.insert({entrance.first.as<std::string>(), entrance.second.as<std::string>()});
            }
        }
    }

    return PlandomizerError::NONE;
}

std::string errorToName(PlandomizerError err)
{
    switch(err)
    {
        case PlandomizerError::NONE:
            return "NONE";
        case PlandomizerError::BAD_STARTING_ISLAND:
            return "BAD_STARTING_ISLAND";
        case PlandomizerError::BAD_STARTING_ITEM:
            return "BAD_STARTING_ITEM";
        case PlandomizerError::MISSING_ITEM_KEY:
            return "MISSING_ITEM_KEY";
        case PlandomizerError::NO_ITEM_AT_LOCATION:
            return "NO_ITEM_AT_LOCATION";
        case PlandomizerError::INVALID_WORLD_ID:
            return "INVALID_WORLD_ID";
        case PlandomizerError::MISSING_PARENT_ENTRANCE:
            return "MISSING_PARENT_ENTRANCE";
        case PlandomizerError::MISSING_PLANDO_LOCATION:
            return "MISSING_PLANDO_LOCATION";
        case PlandomizerError::UNKNOWN_ITEM_NAME:
            return "UNKNOWN_ITEM_NAME";
    }
    return "UNKNOWN";
}
