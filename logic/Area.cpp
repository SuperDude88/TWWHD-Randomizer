
#include "Area.hpp"
#include <unordered_map>
#include <array>

static std::unordered_map<std::string, std::string> areaPrettyNameMap;
static std::unordered_map<std::string, std::string> prettyNameAreaMap;

void storeNewAreaPrettyName(const std::string& area, const std::string& prettyName)
{
    areaPrettyNameMap.emplace(area, prettyName);
    prettyNameAreaMap.emplace(prettyName, area);
}

std::string areaToPrettyName(const std::string& area)
{
    if (areaPrettyNameMap.count(area) == 0)
    {
        return "INVALID AREA OR NO PRETTY NAME";
    }
    return areaPrettyNameMap.at(area);
}

std::string prettyNameToArea(const std::string& prettyName)
{
    if (prettyNameAreaMap.count(prettyName) == 0)
    {
        return "UNKNOWN PRETTY NAME";
    }
    return prettyNameAreaMap.at(prettyName);
}

std::string roomIndexToIslandName(const uint8_t& startingIslandRoomIndex)
{
    // Island room number corresponds with index in the below array
    const std::array<std::string, 50> startingIslandAreaArray = {
        "INVALID",
        "ForsakenFortressSector",
        "StarIsland",
        "NorthernFairyIsland",
        "GaleIsle",
        "CrescentMoonIsland",
        "SevenStarIsles",
        "OverlookIsland",
        "FourEyeReef",
        "MotherAndChildIsles",
        "SpectacleIsland",
        "WindfallIsland",
        "PawprintIsle",
        "DragonRoostIsland",
        "FlightControlPlatform",
        "WesternFairyIsland",
        "RockSpireIsle",
        "TingleIsland",
        "NorthernTriangleIsland",
        "EasternFairyIsland",
        "FireMountain",
        "StarBeltArchipelago",
        "ThreeEyeReef",
        "GreatfishIsle",
        "CyclopsReef",
        "SixEyeReef",
        "TotGSector",
        "EasternTriangleIsland",
        "ThornedFairyIsland",
        "NeedleRockIsle",
        "IsletOfSteel",
        "StoneWatcherIsland",
        "SouthernTriangleIsland",
        "PrivateOasis",
        "BombIsland",
        "BirdsPeakRock",
        "DiamondSteppeIsland",
        "FiveEyeReef",
        "SharkIsland",
        "SouthernFairyIsland",
        "IceRingIsle",
        "ForestHaven",
        "CliffPlateauIsles",
        "HorseshoeIsle",
        "OutsetIsland",
        "HeadstoneIsland",
        "TwoEyeReef",
        "AngularIsles",
        "BoatingCourse",
        "FiveStarIsles",
    };

    return startingIslandAreaArray[startingIslandRoomIndex];
}

uint8_t islandNameToRoomIndex(const std::string& islandName)
{
    static std::unordered_map<std::string, uint8_t> islandAreaMap = {
        {"ForsakenFortressSector", 1},
        {"StarIsland", 2},
        {"NorthernFairyIsland", 3},
        {"GaleIsle", 4},
        {"CrescentMoonIsland", 5},
        {"SevenStarIsles", 6},
        {"OverlookIsland", 7},
        {"FourEyeReef", 8},
        {"MotherAndChildIsles", 9},
        {"SpectacleIsland", 10},
        {"WindfallIsland", 11},
        {"PawprintIsle", 12},
        {"DragonRoostIsland", 13},
        {"FlightControlPlatform", 14},
        {"WesternFairyIsland", 15},
        {"RockSpireIsle", 16},
        {"TingleIsland", 17},
        {"NorthernTriangleIsland", 18},
        {"EasternFairyIsland", 19},
        {"FireMountain", 20},
        {"StarBeltArchipelago", 21},
        {"ThreeEyeReef", 22},
        {"GreatfishIsle", 23},
        {"CyclopsReef", 24},
        {"SixEyeReef", 25},
        {"TotGSector", 26},
        {"EasternTriangleIsland", 27},
        {"ThornedFairyIsland", 28},
        {"NeedleRockIsle", 29},
        {"IsletOfSteel", 30},
        {"StoneWatcherIsland", 31},
        {"SouthernTriangleIsland", 32},
        {"PrivateOasis", 33},
        {"BombIsland", 34},
        {"BirdsPeakRock", 35},
        {"DiamondSteppeIsland", 36},
        {"FiveEyeReef", 37},
        {"SharkIsland", 38},
        {"SouthernFairyIsland", 39},
        {"IceRingIsle", 40},
        {"ForestHaven", 41},
        {"CliffPlateauIsles", 42},
        {"HorseshoeIsle", 43},
        {"OutsetIsland", 44},
        {"HeadstoneIsland", 45},
        {"TwoEyeReef", 46},
        {"AngularIsles", 47},
        {"BoatingCourse", 48},
        {"FiveStarIsles", 49},
    };

    return islandAreaMap[islandName];
}
