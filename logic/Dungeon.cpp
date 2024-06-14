
#include "Dungeon.hpp"
#include <unordered_set>

// Helper function to make sure there are no dungeon name typos
// Note that Ganon's Tower is also listed as a dungeon location by the
// data files, however it doesn't really fit the properties of the dungeon
// struct, so we don't consider it one here.
bool isValidDungeon(const std::string& dungeonName)
{
    static const std::unordered_set<std::string> dungeonNames = {
        "Dragon Roost Cavern",
        "Forbidden Woods",
        "Tower of the Gods",
        "Forsaken Fortress",
        "Earth Temple",
        "Wind Temple"
    };

    return dungeonNames.contains(dungeonName);
}
