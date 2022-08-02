
#include "SpoilerLog.hpp"
#include "../options.hpp"
#include "../server/command/Log.hpp"
#include "../server/utility/platform.hpp"
#include "../server/filetypes/util/msbtMacros.hpp"
#include "../server/utility/stringUtil.hpp"

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
    size_t numSpaces = (longestNameLength - location->name.length()) + ((location->worldId >= 9) ? 0 : 1);
    std::string spaces (numSpaces, ' ');

    // Don't say which player the item is for if there's only 1 world
    std::string itemName = worlds.size() > 1 ? location->currentItem.getName() : gameItemToName(location->currentItem.getGameItemId());

    return location->name + worldNumber + ":" + spaces + itemName;
}

static std::string getSpoilerFormatHint(Location* location)
{
    // Get rid of commands in the hint text and then convert to UTF-8
    std::u16string hintText = location->hintText;
    for (auto eraseText : {TEXT_COLOR_RED, TEXT_COLOR_BLUE, TEXT_COLOR_CYAN, TEXT_COLOR_DEFAULT})
    {
        auto pos = std::string::npos;
        while ((pos = hintText.find(eraseText)) != std::string::npos)
        {
            hintText.erase(pos, eraseText.length());
        }
    }
    return Utility::Str::toUTF8(hintText);
}

// Compatator for sorting the chart mappings
struct chartComparator {
    bool operator()(const std::string& a, const std::string& b) const {
        if (a.length() != b.length())
        {
            return a.length() < b.length();
        }
        return a < b;
    }
};

static void printBasicInfo(std::ofstream& log, const WorldPool& worlds)
{
    log << "Program opened " << ProgramTime::getDateStr(); //time string ends with \n

    log << "Wind Waker HD Randomizer Version " << RANDOMIZER_VERSION << std::endl;
    log << "Seed: " << LogInfo::getConfig().seed << std::endl;

    // Print options selected for each world
    for (const auto& world : worlds)
    {
        log << ((worlds.size() > 1) ? "Selected options for world " + std::to_string(world.getWorldId() + 1) + ":" : "Selected options:") << std::endl << "\t";
        for (int settingInt = 1; settingInt < static_cast<int>(Option::COUNT); settingInt++)
        {
            Option setting = static_cast<Option>(settingInt);

            if (setting == Option::NumShards || setting == Option::NumRaceModeDungeons || setting == Option::DamageMultiplier || setting == Option::PigColor)
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
    std::ofstream log("./Spoiler Log.txt");

	  Utility::platformLog("Generating spoiler log...\n");
    printBasicInfo(log, worlds);

    // Playthroughs are stored in world 1 for the time being, regardless of how
    // many worlds there are.
    auto& playthroughSpheres = worlds[0].playthroughSpheres;
    auto& entranceSpheres = worlds[0].entranceSpheres;

    LOG_TO_DEBUG("Starting Island");
    // Print the random starting island if there is one
    for (auto& world : worlds)
    {
        if (world.getSettings().randomize_starting_island)
        {
            auto startingIsland = world.getArea("Link's Spawn").exits.front().getConnectedArea();
            log << "Starting Island" << ((worlds.size() > 1) ? " for world " + std::to_string(world.getWorldId() + 1) : "") << ": " << startingIsland << std::endl;
        }
    }
    log << std::endl;

    // Find the longest location/entrances names for formatting the file
    size_t longestNameLength = 0;
    size_t longestEntranceLength = 0;
    LOG_TO_DEBUG("Getting Name Lengths");
    for (auto sphereItr = playthroughSpheres.begin(); sphereItr != playthroughSpheres.end(); sphereItr++)
    {
        for (auto location : *sphereItr)
        {
            longestNameLength = std::max(longestNameLength, location->name.length());
        }
    }
    for (auto& world : worlds)
    {
        auto entrances = world.getShuffledEntrances(EntranceType::ALL, false);
        for (auto entrance : entrances)
        {
            longestEntranceLength = std::max(longestEntranceLength, entrance->getOriginalName().length());
        }
    }

    // Print the playthrough
    LOG_TO_DEBUG("Print Playthrough");
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
    LOG_TO_DEBUG("Print Entrance Playthrough");
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

    LOG_TO_DEBUG("Entrance Listing");
    for (auto& world : worlds)
    {
        auto entrances = world.getShuffledEntrances(EntranceType::ALL, !world.getSettings().decouple_entrances);
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
    LOG_TO_DEBUG("All Locations");
    // Update the longest location name considering all locations
    for (auto& world : worlds)
    {
        for (auto location : world.getLocations())
        {
            if (!location->categories.contains(LocationCategory::HoHoHint))
            {
                longestNameLength = std::max(longestNameLength, location->name.length());
            }
        }
    }

    for (auto& world : worlds)
    {
        for (auto location : world.getLocations())
        {
            if (!location->categories.contains(LocationCategory::HoHoHint))
            {
                log << "\t" << getSpoilerFormatLocation(location, longestNameLength, worlds) << std::endl;
            }
        }
    }
    log << std::endl;

    LOG_TO_DEBUG("Hints");
    for (auto& world : worlds)
    {
        log << std::endl << (worlds.size() == 1 ? "Hints:" : "Hints for world " + std::to_string(world.getWorldId()) + ":") << std::endl;
        if (!world.hohoHints.empty())
        {
            for (auto& [hohoLocation, hintLocations] : world.hohoHints)
            {
                log << "\t" << hohoLocation->name << ":" << std::endl;
                for (auto location : hintLocations)
                {
                    log << "\t\t" << getSpoilerFormatHint(location) << std::endl;
                }
            }
        }

        if (!world.korlHints.empty())
        {
            log << "\tKoRL Hints:" << std::endl;
            for (auto location : world.korlHints)
            {
                log << "\t\t" << getSpoilerFormatHint(location) << std::endl;
            }
        }
    }
    log << std::endl;

    LOG_TO_DEBUG("Chart Mappings");
    for (auto& world : worlds)
    {
        log << "Charts for world " << std::to_string(world.getWorldId() + 1) << ":" << std::endl;
        std::map<std::string, std::string> spoilerTriforceMappings = {};
        std::map<std::string, std::string, chartComparator> spoilerTreasureMappings = {};
        for (size_t islandRoom = 1; islandRoom < 50; islandRoom++)
        {
            auto chart = gameItemToName(world.chartMappings[islandRoom - 1]);
            auto island = roomIndexToIslandName(islandRoom);
            if (chart.find("Treasure") != std::string::npos)
            {
                spoilerTreasureMappings[chart] = island;
            }
            else
            {
                spoilerTriforceMappings[chart] = island;
            }

        }
        for (auto& [chart, island] : spoilerTriforceMappings)
        {
            log << "\t" << chart << ":\t" << island << std::endl;
        }
        for (auto& [chart, island] : spoilerTreasureMappings)
        {
            log << "\t" << chart << ":\t" << island << std::endl;
        }
    }

    log.close();
}

void generateNonSpoilerLog(WorldPool& worlds)
{
    BasicLog::getInstance().log("### Locations that may or may not have progress items in them on this run:");
    for (auto& world : worlds)
    {
        for (auto location : world.getLocations())
        {
            if (location->progression)
            {
                BasicLog::getInstance().log("\t" + location->name);
            }
        }
    }

    BasicLog::getInstance().log("### Locations that cannot have progress items in them on this run:");
    for (auto& world : worlds)
    {
        for (auto location : world.getLocations())
        {
            if (!location->progression)
            {
                BasicLog::getInstance().log("\t" + location->name);
            }
        }
    }
}
