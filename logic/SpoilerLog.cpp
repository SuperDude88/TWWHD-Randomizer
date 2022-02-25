
#include "SpoilerLog.hpp"
#include "../options.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>

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
    log.open("spoiler.txt");

    printBasicInfo(log, worlds);

    // Find the longest location name for formatting the file
    size_t longestNameLength = 0;
    for (size_t sphere = 0; sphere < worlds[0].playthroughSpheres.size(); sphere++)
    {
        for (auto location : worlds[0].playthroughSpheres[sphere])
        {
            longestNameLength = std::max(longestNameLength, locationIdToName(location->locationId).length());
        }
    }

    // Print the playthrough
    for (size_t sphere = 0; sphere < worlds[0].playthroughSpheres.size(); sphere++)
    {
        log << "Sphere " << std::to_string(sphere) << ":" << std::endl;
        for (auto location : worlds[0].playthroughSpheres[sphere])
        {

            // Print the world number if more than 1 world
            std::string worldNumber = " [W";
            worldNumber = worlds.size() > 1 ? worldNumber + std::to_string(location->worldId + 1) + "]" : "";
                                                                         // Don't add an extra space if the world id is two digits long
            size_t numSpaces = (longestNameLength - locationIdToName(location->locationId).length()) + ((location->worldId >= 9) ? 0 : 1);
            std::string spaces (numSpaces, ' ');

            log << "\t" << locationIdToName(location->locationId) << worldNumber << ":" << spaces << location->currentItem.getName() << std::endl;
        }
    }

    log << std::endl << "All Locations:" << std::endl;

    for (auto& world : worlds)
    {
        for (auto location : world.getLocations())
        {
            log << "\t" << locationName(location) << ": " << location->currentItem.getName() << std::endl;
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
