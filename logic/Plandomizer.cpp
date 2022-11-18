#include "Plandomizer.hpp"

#include <command/Log.hpp>
#include <utility/file.hpp>

PlandomizerError loadPlandomizer(std::string& plandoFilepath, std::vector<Plandomizer>& plandos, size_t numWorlds)
{
    LOG_TO_DEBUG("Loading plandomzier file");

    std::string plandoStr;
    if (Utility::getFileContents(plandoFilepath, plandoStr) != 0)
    {
        Utility::platformLog("Will skip using plando file\n");
        return PlandomizerError::NONE;
    }

    // Go through and make plandomizer objects for each world
    for (size_t i = 0; i < numWorlds; i++)
    {
        LOG_TO_DEBUG("Loading Plando data for world " + std::to_string(i + 1))
        plandos[i] = Plandomizer();
        auto& plandomizer = plandos[i];
        Yaml::Node plandoTree;
        Yaml::Parse(plandoTree, plandoStr);
        Yaml::Node plandoLocations;
        Yaml::Node plandoEntrances;
        Yaml::Node extraProgressionLocations;
        Yaml::Node randomStartingItemPool;
        std::string plandoStartingIsland = "";
        std::string worldName = "World " + std::to_string(i + 1);
        // Grab the YAML object which holds the locations for this world.
        for (auto refIt = plandoTree.Begin(); refIt != plandoTree.End(); refIt++)
        {
            Yaml::Node& ref = (*refIt).second;
            if (ref[worldName].IsMap())
            {
                if (ref[worldName]["locations"].IsMap())
                {
                    plandoLocations = ref[worldName]["locations"];
                }

                if (ref[worldName]["entrances"].IsMap())
                {
                    plandoEntrances = ref[worldName]["entrances"];
                }
                if (ref[worldName]["starting island"].IsScalar())
                {
                    plandoStartingIsland = ref[worldName]["starting island"].As<std::string>();
                }
                if (ref[worldName]["extra progression locations"].IsSequence())
                {
                    extraProgressionLocations = ref[worldName]["extra progression locations"];
                }
                if (ref[worldName]["random starting item pool"].IsSequence())
                {
                    randomStartingItemPool = ref[worldName]["random starting item pool"];
                }
            }
        }

        // Process starting island
        if (plandoStartingIsland != "")
        {
            plandomizer.startingIslandRoomIndex = islandNameToRoomIndex(plandoStartingIsland);
            if (plandomizer.startingIslandRoomIndex == 0)
            {
                ErrorLog::getInstance().log("Plandomizer Error: Starting island name \"" + plandoStartingIsland + "\" is not recognized");
                return PlandomizerError::BAD_STARTING_ISLAND;
            }
            LOG_TO_DEBUG("  Setting starting island to " + plandoStartingIsland);
        }

        // Collect extra progression locations first
        if (!extraProgressionLocations.IsNone())
        {
            LOG_TO_DEBUG("  Extra Progress Locations: ")
            for (auto locationIt = extraProgressionLocations.Begin(); locationIt != extraProgressionLocations.End(); locationIt++)
            {
                const Yaml::Node& locationNode = (*locationIt).second;
                const std::string locationName = locationNode.As<std::string>();
                plandomizer.extraProgressionLocations.insert(locationName);
                LOG_TO_DEBUG(std::string("    ") + locationName);
            }
        }

        // Process random starting item pool
        if (!randomStartingItemPool.IsNone())
        {
            LOG_TO_DEBUG("  Starting Item Pool: ")
            for (auto itemIt = randomStartingItemPool.Begin(); itemIt != randomStartingItemPool.End(); itemIt++)
            {
                const Yaml::Node& itemNode = (*itemIt).second;
                const std::string itemName = itemNode.As<std::string>();
                const auto gameItem = nameToGameItem(itemName);
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
        if (!plandoLocations.IsNone())
        {
            for (auto locationIt = plandoLocations.Begin(); locationIt != plandoLocations.End(); locationIt++)
            {
                auto locationObject = *locationIt;
                if (locationObject.first.empty())
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
                    if (!locationObject.second["item"].IsNone())
                    {
                        itemName = locationObject.second["item"].As<std::string>();
                    }
                    else
                    {
                        ErrorLog::getInstance().log("Plandomizer Error: Missing key \"item\" in location \"" + locationObject.first + "\"");
                        return PlandomizerError::MISSING_ITEM_KEY;
                    }
                    if (!locationObject.second["world"].IsNone())
                    {
                        plandoWorldId = std::stoi(locationObject.second["world"].As<std::string>());
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
                    itemName = locationObject.second.As<std::string>();
                    if (itemName == "\n" || itemName == "")
                    {
                        ErrorLog::getInstance().log("Plandomizer Error: Location \"" + locationObject.first + "\" has no plandomized item.");
                        return PlandomizerError::NO_ITEM_AT_LOCATION;
                    }
                }

                // Get location name
                std::string locationName = locationObject.first;

                // Sanitize item name incase the user missed an apostraphe
                itemName = gameItemToName(nameToGameItem(itemName));

                // Get GameItem
                auto gameItem = nameToGameItem(itemName);

                plandomizer.locationsStr.insert({locationName, {gameItem, plandoWorldId}});
                LOG_TO_DEBUG("  Plandomizing " +  itemName + " [W" + std::to_string(plandoWorldId + 1) + "] to " + locationName + " [W" + std::to_string(i + 1) + "]");
            }
        }

        // Process Entrances
        if (!plandoEntrances.IsNone())
        {
            for (auto entranceIt = plandoEntrances.Begin(); entranceIt != plandoEntrances.End(); entranceIt++)
            {
                auto entranceObject = *entranceIt;
                if (entranceObject.first.empty())
                {
                    ErrorLog::getInstance().log("Plandomizer Error: One of the plando entrances is missing a parent entrance");
                    return PlandomizerError::MISSING_PARENT_ENTRANCE;
                }
                // Process strings of each plando's entrance into their respective entrance
                // pointers
                const std::string originalEntranceStr = entranceObject.first;
                const std::string replacementEntranceStr = entranceObject.second.As<std::string>();

                plandomizer.entrancesStr.insert({originalEntranceStr, replacementEntranceStr});
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
