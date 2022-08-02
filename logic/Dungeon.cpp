
#include "Dungeon.hpp"
#include <unordered_set>

// Helper function to make sure there are no dungeon name typos
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
