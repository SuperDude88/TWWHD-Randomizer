
#include "Area.hpp"
#include <unordered_map>
#include <array>

std::string roomIndexToIslandName(const uint8_t& startingIslandRoomIndex)
{
    // Island room number corresponds with index in the below array
    const std::array<std::string, 50> startingIslandAreaArray = {
        "INVALID",
        "Forsaken Fortress Sector",
        "Star Island",
        "Northern Fairy Island",
        "Gale Isle",
        "Crescent Moon Island",
        "Seven Star Isles",
        "Overlook Island",
        "Four Eye Reef",
        "Mother & Child Isles",
        "Spectacle Island",
        "Windfall Island",
        "Pawprint Isle",
        "Dragon Roost Island",
        "Flight Control Platform",
        "Western Fairy Island",
        "Rock Spire Isle",
        "Tingle Island",
        "Northern Triangle Island",
        "Eastern Fairy Island",
        "Fire Mountain",
        "Star Belt Archipelago",
        "Three Eye Reef",
        "Greatfish Isle",
        "Cyclops Reef",
        "Six Eye Reef",
        "Tower of the Gods Sector",
        "Eastern Triangle Island",
        "Thorned Fairy Island",
        "Needle Rock Isle",
        "Islet of Steel",
        "Stone Watcher Island",
        "Southern Triangle Island",
        "Private Oasis",
        "Bomb Island",
        "Birds Peak Rock",
        "Diamond Steppe Island",
        "Five Eye Reef",
        "Shark Island",
        "Southern Fairy Island",
        "Ice Ring Isle",
        "Forest Haven",
        "Cliff Plateau Isles",
        "Horseshoe Island",
        "Outset Island",
        "Headstone Island",
        "Two Eye Reef",
        "Angular Isles",
        "Boating Course",
        "Five Star Isles",
    };

    return startingIslandAreaArray[startingIslandRoomIndex];
}

uint8_t islandNameToRoomIndex(const std::string& islandName)
{
    static std::unordered_map<std::string, uint8_t> islandAreaMap = {
        {"Forsaken Fortress Sector", 1},
        {"Star Island", 2},
        {"Northern Fairy Island", 3},
        {"Gale Isle", 4},
        {"Crescent Moon Island", 5},
        {"Seven Star Isles", 6},
        {"Overlook Island", 7},
        {"Four Eye Reef", 8},
        {"Mother & Child Isles", 9},
        {"Spectacle Island", 10},
        {"Windfall Island", 11},
        {"Pawprint Isle", 12},
        {"Dragon Roost Island", 13},
        {"Flight Control Platform", 14},
        {"Western Fairy Island", 15},
        {"Rock Spire Isle", 16},
        {"Tingle Island", 17},
        {"Northern Triangle Island", 18},
        {"Eastern Fairy Island", 19},
        {"Fire Mountain", 20},
        {"Star Belt Archipelago", 21},
        {"Three Eye Reef", 22},
        {"Greatfish Isle", 23},
        {"Cyclops Reef", 24},
        {"Six Eye Reef", 25},
        {"Tower of the Gods Sector", 26},
        {"Eastern Triangle Island", 27},
        {"Thorned Fairy Island", 28},
        {"Needle Rock Isle", 29},
        {"Islet of Steel", 30},
        {"Stone Watcher Island", 31},
        {"Southern Triangle Island", 32},
        {"Private Oasis", 33},
        {"Bomb Island", 34},
        {"Birds Peak Rock", 35},
        {"Diamond Steppe Island", 36},
        {"Five Eye Reef", 37},
        {"Shark Island", 38},
        {"Southern Fairy Island", 39},
        {"Ice Ring Isle", 40},
        {"Forest Haven", 41},
        {"Cliff Plateau Isles", 42},
        {"Horseshoe Island", 43},
        {"Outset Island", 44},
        {"Headstone Island", 45},
        {"Two Eye Reef", 46},
        {"Angular Isles", 47},
        {"Boating Course", 48},
        {"Five Star Isles", 49},
    };

    if (islandAreaMap.count(islandName) == 0)
    {
        return 0;
    }

    return islandAreaMap[islandName];
}
