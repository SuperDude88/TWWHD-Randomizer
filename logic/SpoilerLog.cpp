
#include "SpoilerLog.hpp"
#include "../options.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>

static std::string getSpoilerFormatEntrance(Entrance* entrance, const size_t& longestEntranceLength, const WorldPool& worlds)
{
    // Print the world number if more than 1 world
    std::string worldNumber = " [W";
    worldNumber = worlds.size() > 1 ? worldNumber + std::to_string(entrance->getWorldId() + 1) + "]" : "";
    // Add an extra space if the world id is only 1 digit
    size_t numSpaces = (longestEntranceLength - entrance->getOriginalName().length()) + ((entrance->getWorldId() >= 9) ? 0 : 1);
    std::string spaces (numSpaces, ' ');

    auto name = entrance->getOriginalName();
    auto replacement = entrance->getReplaces()->getOriginalName();
    // Parse out the parent and connection for a more friendly formatting
    auto pos = replacement.find(" -> ");
    // The parent area is the first one
    auto parent = replacement.substr(0, pos);
    // Then just delete the parent plus the ' -> ' and we're left with
    // only the connected area
    replacement.erase(0, pos+4);

    return name + worldNumber + ": " + spaces + replacement + " from " + parent;
}

static std::string getSpoilerFormatLocation(Location* location, const size_t& longestNameLength, const WorldPool& worlds)
{
    // Print the world number if more than 1 world
    std::string worldNumber = " [W";
    worldNumber = worlds.size() > 1 ? worldNumber + std::to_string(location->worldId + 1) + "]" : "";
                                                                 // Don't add an extra space if the world id is two digits long
    size_t numSpaces = (longestNameLength - locationIdToPrettyName(location->locationId).length()) + ((location->worldId >= 9) ? 0 : 1);
    std::string spaces (numSpaces, ' ');

    // Don't say which player the item is for if there's only 1 world
    std::string itemName = worlds.size() > 1 ? location->currentItem.getPrettyName() : gameItemToPrettyName(location->currentItem.getGameItemId());

    return locationIdToPrettyName(location->locationId) + worldNumber + ":" + spaces + itemName;
}

static void printBasicInfo(std::ofstream& log, const WorldPool& worlds)
{
    log << "Wind Waker HD Randomizer Version <insert version number here>" /*<< VERSION*/ << std::endl;
    log << "Permalink: <insert permalink here>" /*<< PERMALINK*/ << std::endl;
    log << "Seed: <insert seed here>" /*<< SEED*/ << std::endl;

    // Print options selected for each world
    for (const auto& world : worlds)
    {
        log << ((worlds.size() > 1) ? "Selected options for world " + std::to_string(world.getWorldId() + 1) + ":" : "Selected options:") << std::endl << "\t";
        for (int settingInt = 1; settingInt < static_cast<int>(Option::COUNT); settingInt++)
        {
            Option setting = static_cast<Option>(settingInt);

            if (setting == Option::NumShards || setting == Option::NumRaceModeDungeons || setting == Option::DamageMultiplier)
            {
                log << settingToName(setting) << ": " << std::to_string(getSetting(world.getSettings(), setting)) << ", ";
            }
            else
            {
                log << (getSetting(world.getSettings(), setting) ? settingToName(setting) + ", " : "");
            }
        }
        log << std::endl;
    }
    log << std::endl;
}

void generateSpoilerLog(WorldPool& worlds)
{
    std::ofstream log;
    log.open("spoiler.txt"); // Rventually add seed/hash to filename

    printBasicInfo(log, worlds);

    // Playthroughs are stored in world 1 for the time being, regardless of how
    // many worlds there are.
    auto& playthroughSpheres = worlds[0].playthroughSpheres;
    auto& entranceSpheres = worlds[0].entranceSpheres;

    // Print the random starting island if there is one
    for (auto& world : worlds)
    {
        if (world.getSettings().randomize_starting_island)
        {
            auto startingIsland = areaToName(world.getArea(Area::LinksSpawn).exits.front().getConnectedArea());
            log << "Starting Island" << ((worlds.size() > 1) ? " for world " + std::to_string(world.getWorldId() + 1) : "") << ": " << startingIsland << std::endl;
        }
    }
    log << std::endl;

    // Find the longest location/entrances names for formatting the file
    size_t longestNameLength = 0;
    size_t longestEntranceLength = 0;
    for (auto sphereItr = playthroughSpheres.begin(); sphereItr != playthroughSpheres.end(); sphereItr++)
    {
        for (auto location : *sphereItr)
        {
            longestNameLength = std::max(longestNameLength, locationIdToPrettyName(location->locationId).length());
        }
    }

    for (auto sphereItr = entranceSpheres.begin(); sphereItr != entranceSpheres.end(); sphereItr++)
    {
        for (auto entrance : *sphereItr)
        {
            if (entrance->isShuffled())
            {
                longestEntranceLength = std::max(longestEntranceLength, entrance->getOriginalName().length());
            }
        }
    }


    // Print the playthrough
    log << "Playthrough:" << std::endl;
    int sphere = 0;
    for (auto sphereItr = playthroughSpheres.begin(); sphereItr != playthroughSpheres.end(); sphereItr++, sphere++)
    {
        log << "\tSphere " << std::to_string(sphere) << ":" << std::endl;
        auto& sphereLocations = *sphereItr;
        sphereLocations.sort([](Location* a, Location* b){return *a < *b;});
        for (auto location : sphereLocations)
        {
            log << "\t\t" << getSpoilerFormatLocation(location, longestNameLength, worlds) << std::endl;
        }
    }
    log << std::endl;


    // Print the randomized entrances/playthrough
    if (longestEntranceLength != 0)
    {
        log << "Entrance Playthrough:" << std::endl;
    }
    sphere = 0;
    for (auto sphereItr = entranceSpheres.begin(); sphereItr != entranceSpheres.end(); sphereItr++, sphere++)
    {
        // Don't print empty spheres in the entrance playthrough
        if (sphereItr->empty())
        {
            continue;
        }
        log << "\tSphere " << std::to_string(sphere) << ":" << std::endl;
        auto& sphereEntrances = *sphereItr;
        sphereEntrances.sort([](Entrance* a, Entrance* b){return *a < *b;});
        for (auto entrance : sphereEntrances)
        {
            log << "\t\t" << getSpoilerFormatEntrance(entrance, longestEntranceLength, worlds) << std::endl;
        }
    }
    log << std::endl;


    for (auto& world : worlds)
    {
        auto entrances = world.getShuffledEntrances(EntranceType::ALL, world.getSettings().decouple_entrances);
        if (entrances.empty())
        {
            continue;
        }

        log << "Entrances for world " << std::to_string(world.getWorldId()) << ":" << std::endl;
        std::sort(entrances.begin(), entrances.end(), [](Entrance* a, Entrance* b){return *a < *b;});
        for (auto entrance : entrances)
        {
            log << "\t" << getSpoilerFormatEntrance(entrance, longestEntranceLength, worlds) << std::endl;
        }
        log << std::endl;
    }


    log << std::endl << "All Locations:" << std::endl;
    // Update the longest location name considering all locations
    for (auto& world : worlds)
    {
        for (auto location : world.getLocations())
        {
            longestNameLength = std::max(longestNameLength, locationIdToPrettyName(location->locationId).length());
        }
    }

    for (auto& world : worlds)
    {
        for (auto location : world.getLocations())
        {

            log << "\t" << getSpoilerFormatLocation(location, longestNameLength, worlds) << std::endl;
        }
    }

    log.close();
}

void generateNonSpoilerLog(WorldPool& worlds)
{
    std::ofstream log;
    log.open("nonspoiler.txt");

    printBasicInfo(log, worlds);
    log << "### Locations that may or may not have progress items in them on this run:" << std::endl;
    for (auto& world : worlds)
    {
        for (auto location : world.getLocations())
        {
            if (location->progression)
            {
                log << "\t" << locationName(location) << std::endl;
            }
        }
    }

    log << "### Locations that cannot have progress items in them on this run:" << std::endl;
    for (auto& world : worlds)
    {
        for (auto location : world.getLocations())
        {
            if (!location->progression)
            {
                log << "\t" << locationName(location) << std::endl;
            }
        }
    }

    log.close();
}
