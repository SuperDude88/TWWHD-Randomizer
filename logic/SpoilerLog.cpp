#include "SpoilerLog.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>

#include <version.hpp>
#include <options.hpp>
#include <command/Log.hpp>
#include <logic/World.hpp>
#include <filetypes/util/msbtMacros.hpp>
#include <utility/platform.hpp>
#include <utility/string.hpp>
#include <utility/time.hpp>

static std::string getSpoilerFormatEntrance(Entrance* entrance, const size_t& longestEntranceLength, const WorldPool& worlds)
{
    // Print the world number if more than 1 world
    std::string worldNumber = " [W";
    worldNumber = worlds.size() > 1 ? worldNumber + std::to_string(entrance->getWorldId() + 1) + "]" : "";

    auto currentEntranceName = entrance->getOriginalName(true);
    // Add an extra space if the world id is only 1 digit
    size_t numSpaces = (longestEntranceLength - currentEntranceName.length()) + ((entrance->getWorldId() >= 9) ? 0 : 1);
    std::string spaces (numSpaces, ' ');
    
    auto replacement = entrance->getReplaces()->getOriginalName();
    // Parse out the parent and connection for a more friendly formatting
    auto pos = replacement.find(" -> ");
    // The parent area is the first one
    auto parent = replacement.substr(0, pos);
    // Then just delete the parent plus the ' -> ' and we're left with
    // only the connected area
    replacement.erase(0, pos+4);

    return currentEntranceName + worldNumber + ": " + spaces + replacement + " from " + parent;
}

static std::string getSpoilerFormatLocation(Location* location, const size_t& longestNameLength, const WorldPool& worlds)
{
    // Print the world number if more than 1 world
    std::string locWorldNumber = worlds.size() > 1 ? " [W" + std::to_string(location->world->getWorldId() + 1) + "]" : "";
                                                                 // Don't add an extra space if the world id is two digits long
    size_t numSpaces = (longestNameLength - location->getName().length()) + ((location->world->getWorldId() >= 9) ? 0 : 1);
    std::string spaces (numSpaces, ' ');

    // Don't say which player the item is for if there's only 1 world
    std::string itemWorldNumber = worlds.size() > 1 ? " [W" + std::to_string(location->currentItem.getWorld()->getWorldId() + 1) + "]" : "";
    std::string itemName = location->currentItem.getName() + (worlds.size() > 1 ? itemWorldNumber : "");

    return location->getName() + locWorldNumber + ":" + spaces + itemName;
}

static std::string getSpoilerFormatHint(Location* location)
{
    // Get rid of commands in the hint text and then convert to UTF-8
    std::u16string hintText = location->hint.text["English"];
    for (const std::u16string& eraseText : {TEXT_COLOR_RED, TEXT_COLOR_BLUE, TEXT_COLOR_CYAN, TEXT_COLOR_DEFAULT, TEXT_COLOR_GREEN, TEXT_COLOR_GRAY, TEXT_COLOR_YELLOW})
    {
        auto pos = std::string::npos;
        while ((pos = hintText.find(eraseText)) != std::string::npos)
        {
            hintText.erase(pos, eraseText.length());
        }
    }
    return Utility::Str::toUTF8(hintText);
}

// Comparator for sorting the chart mappings
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
    log << "# Wind Waker HD Randomizer Version " << RANDOMIZER_VERSION << std::endl;
    log << "# Program opened " << ProgramTime::getDateStr(); //time string ends with \n
    log << std::endl;

    log << "# Settings" << std::endl;
    Config config = LogInfo::getConfig();
    log << "Permalink: " << config.getPermalink() << std::endl;
    log << config.settingsToYaml();
    log << std::endl << std::endl;
}

void generateSpoilerLog(WorldPool& worlds)
{
    std::ofstream spoilerLog(Utility::get_logs_path() / (LogInfo::getSeedHash() + " Spoiler Log.txt"));

    Utility::platformLog("Generating spoiler log...");
    printBasicInfo(spoilerLog, worlds);

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
            auto startingIsland = world.getArea("Link's Spawn")->exits.front().getConnectedArea()->name;
            spoilerLog << "Starting Island" << ((worlds.size() > 1) ? " for world " + std::to_string(world.getWorldId() + 1) : "") << ": " << startingIsland << std::endl;
        }
    }
    spoilerLog << std::endl;

    LOG_TO_DEBUG("Starting Inventory");
    // Print starting inventory items
    for (auto& world : worlds)
    {
        if (!world.getSettings().starting_gear.empty())
        {
            spoilerLog << "Starting Inventory" << ((worlds.size() > 1) ? " for world " + std::to_string(world.getWorldId() + 1) : "") << ":" << std::endl;
            for (auto& gameItem : world.getSettings().starting_gear)
            {
                spoilerLog << "    " << gameItemToName(gameItem) << std::endl;
            }
            spoilerLog << std::endl;
        }
    }

    // Find the longest location/entrances names for formatting the file
    size_t longestNameLength = 0;
    size_t longestEntranceLength = 0;
    LOG_TO_DEBUG("Getting Name Lengths");
    for (const std::list<Location*>& playthroughSphere : playthroughSpheres)
    {
        for (const Location* location : playthroughSphere)
        {
            longestNameLength = std::max(longestNameLength, location->getName().length());
        }
    }
    for (World& world : worlds)
    {
        const EntrancePool entrances = world.getShuffledEntrances(EntranceType::ALL, false);
        for (const Entrance* entrance : entrances)
        {
            longestEntranceLength = std::max(longestEntranceLength, entrance->getOriginalName().length());
        }
    }

    // Print the playthrough
    LOG_TO_DEBUG("Print Playthrough");
    spoilerLog << "Playthrough:" << std::endl;
    int sphere = 0;
    for (auto sphereItr = playthroughSpheres.begin(); sphereItr != playthroughSpheres.end(); sphereItr++, sphere++)
    {
        spoilerLog << "    Sphere " << std::to_string(sphere) << ":" << std::endl;
        auto& sphereLocations = *sphereItr;
        sphereLocations.sort(PointerLess<Location>());
        for (auto location : sphereLocations)
        {
            spoilerLog << "        " << getSpoilerFormatLocation(location, longestNameLength, worlds) << std::endl;
        }
    }
    spoilerLog << std::endl;


    // Print the randomized entrances/playthrough
    LOG_TO_DEBUG("Print Entrance Playthrough");
    if (longestEntranceLength != 0)
    {
        spoilerLog << "Entrance Playthrough:" << std::endl;
    }
    sphere = 0;
    for (auto sphereItr = entranceSpheres.begin(); sphereItr != entranceSpheres.end(); sphereItr++, sphere++)
    {
        // Don't print empty spheres in the entrance playthrough
        if (sphereItr->empty())
        {
            continue;
        }
        spoilerLog << "    Sphere " << std::to_string(sphere) << ":" << std::endl;
        auto& sphereEntrances = *sphereItr;
        for (auto entrance : sphereEntrances)
        {
            spoilerLog << "        " << getSpoilerFormatEntrance(entrance, longestEntranceLength, worlds) << std::endl;
        }
    }
    spoilerLog << std::endl;

    LOG_TO_DEBUG("Entrance Listing");
    for (auto& world : worlds)
    {
        auto entrances = world.getShuffledEntrances(EntranceType::ALL, !world.getSettings().decouple_entrances);
        if (entrances.empty())
        {
            continue;
        }

        spoilerLog << "Entrances for world " << std::to_string(world.getWorldId()) << ":" << std::endl;
        std::ranges::sort(entrances, [](auto lhs, auto rhs){return lhs->getOriginalName() < rhs->getOriginalName();});
        for (auto entrance : entrances)
        {
            spoilerLog << "    " << getSpoilerFormatEntrance(entrance, longestEntranceLength, worlds) << std::endl;
        }
        spoilerLog << std::endl;
    }


    spoilerLog << std::endl << "All Locations:" << std::endl;
    LOG_TO_DEBUG("All Locations");
    // Update the longest location name considering all locations
    for (auto& world : worlds)
    {
        for (auto location : world.getLocations())
        {
            if (!location->categories.contains(LocationCategory::HoHoHint))
            {
                longestNameLength = std::max(longestNameLength, location->getName().length());
            }
        }
    }

    for (auto& world : worlds)
    {
        for (auto location : world.getLocations())
        {
            if (!location->categories.contains(LocationCategory::HoHoHint))
            {
                spoilerLog << "    " << getSpoilerFormatLocation(location, longestNameLength, worlds) << std::endl;
            }
        }
    }
    spoilerLog << std::endl;

    LOG_TO_DEBUG("Hints");
    for (auto& world : worlds)
    {
        // Don't print "Hints" if there are none
        if (world.hohoHints.empty() && world.korlHints.empty() && world.bigOctoFairyHintLocation == nullptr)
        {
            continue;
        }

        spoilerLog << std::endl << (worlds.size() == 1 ? "Hints:" : "Hints for world " + std::to_string(world.getWorldId()) + ":") << std::endl;
        if (!world.hohoHints.empty())
        {
            for (auto& [hohoLocation, hintLocations] : world.hohoHints)
            {
                spoilerLog << "    " << hohoLocation->getName() << ":" << std::endl;
                for (auto location : hintLocations)
                {
                    spoilerLog << "        " << getSpoilerFormatHint(location);
                    // Show what item/location was being referred to with each path hint
                    if (location->hint.type == HintType::PATH) 
                    {
                        spoilerLog << " (" << location->currentItem.getName() << " at " << location->getName() << ")";
                    }
                    spoilerLog << std::endl;
                }
            }
        }

        if (!world.korlHints.empty())
        {
            spoilerLog << "    KoRL Hints:" << std::endl;
            for (auto location : world.korlHints)
            {
                spoilerLog << "        " << getSpoilerFormatHint(location);
                // Show what item/location was being referred to with each path hint
                if (location->hint.type == HintType::PATH) 
                {
                    spoilerLog << " (" << location->currentItem.getName() << " at " << location->getName() << ")";
                }
                spoilerLog << std::endl;
            }
        }

        if (world.bigOctoFairyHintLocation != nullptr)
        {
            spoilerLog << "    Big Octo Great Fairy:" << std::endl;
            std::u16string hintText = world.bigOctoFairyHintLocation->hint.text["English"];
            for (const std::u16string& eraseText : {TEXT_COLOR_RED, TEXT_COLOR_BLUE, TEXT_COLOR_CYAN, TEXT_COLOR_DEFAULT, TEXT_COLOR_GREEN, TEXT_COLOR_GRAY, TEXT_COLOR_YELLOW})
            {
                auto pos = std::string::npos;
                while ((pos = hintText.find(eraseText)) != std::string::npos)
                {
                    hintText.erase(pos, eraseText.length());
                }
            }
            spoilerLog << "        " << Utility::Str::toUTF8(hintText) << std::endl; 
        }
    }
    spoilerLog << std::endl;

    LOG_TO_DEBUG("Chart Mappings");
    for (auto& world : worlds)
    {
        spoilerLog << "Charts for world " << std::to_string(world.getWorldId() + 1) << ":" << std::endl;
        std::map<std::string, std::string> spoilerTriforceMappings = {};
        std::map<std::string, std::string, chartComparator> spoilerTreasureMappings = {};
        for (size_t islandRoom = 1; islandRoom < 50; islandRoom++)
        {
            auto chart = gameItemToName(world.chartMappings[islandRoom]);
            auto island = roomNumToIslandName(islandRoom);
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
            spoilerLog << "    " << chart << ":\t" << island << std::endl;
        }
        for (auto& [chart, island] : spoilerTreasureMappings)
        {
            spoilerLog << "    " << chart << ":\t" << island << std::endl;
        }
    }

    spoilerLog.close();
}

void generateNonSpoilerLog(WorldPool& worlds)
{
    std::ofstream nonSpoilerLog(Utility::get_logs_path() / (LogInfo::getSeedHash() + " Non-Spoiler Log.txt"));

    Utility::platformLog("Generating non-spoiler log...");
    printBasicInfo(nonSpoilerLog, worlds);

    nonSpoilerLog << "### Locations that may or may not have progress items in them on this run:" << std::endl;
    for (auto& world : worlds)
    {
        for (auto location : world.getLocations())
        {
            if (location->progression)
            {
                nonSpoilerLog << "#   " + location->getName() << std::endl;
            }
        }
    }

    nonSpoilerLog << std::endl << "### Locations that cannot have progress items in them on this run:" << std::endl;
    for (auto& world : worlds)
    {
        for (auto location : world.getLocations())
        {
            // Don't print blue chu chu locations (yet) or Ho Ho Hint Locations 
            if (!location->progression && !location->categories.contains(LocationCategory::BlueChuChu) && !location->categories.contains(LocationCategory::HoHoHint))
            {
                nonSpoilerLog << "#   " + location->getName() << std::endl;
            }
        }
    }

    nonSpoilerLog.close();
}
