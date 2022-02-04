
#include "SpoilerLog.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>

void generateSpoilerLog(WorldPool& worlds)
{
    // Find the longest location name for formatting the file
    size_t longestNameLength = 0;
    for (size_t sphere = 0; sphere < worlds[0].playthroughSpheres.size(); sphere++)
    {
        for (auto location : worlds[0].playthroughSpheres[sphere])
        {
            longestNameLength = std::max(longestNameLength, locationIdToName(location->locationId).length());
        }
    }

    std::ofstream log;
    log.open("spoiler.txt");

    for (size_t sphere = 0; sphere < worlds[0].playthroughSpheres.size(); sphere++)
    {
        log << "Sphere " << std::to_string(sphere) << ":" << std::endl;
        for (auto location : worlds[0].playthroughSpheres[sphere])
        {

            // Print the world number if more than 1 world
            std::string worldNumber = " [W";
            worldNumber = worlds.size() > 1 ? worldNumber + std::to_string(location->worldId + 1) + "]" : "";
                                                                         // Don't add an extra space if the world id is two digits long
            size_t numSpaces = (longestNameLength - locationIdToName(location->locationId).length()) + (location->worldId >= 9) ? 0 : 1;
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
