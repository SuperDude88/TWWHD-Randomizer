
#include "Area.hpp"
#include <unordered_map>

Area nameToArea(const std::string& name)
{
    static std::unordered_map<std::string, Area> nameAreaMap = {
        {"Root", Area::Root},
        {"LinksSpawn", Area::LinksSpawn},
        {"TheGreatSea", Area::TheGreatSea},
        {"ForsakenFortress", Area::ForsakenFortress},
        {"StarIsland", Area::StarIsland},
        {"StarIslandCave", Area::StarIslandCave},
        {"NorthernFairyIsland", Area::NorthernFairyIsland},
        {"GaleIsle", Area::GaleIsle},
        {"CrescentMoonIsland", Area::CrescentMoonIsland},
        {"SevenStarIsles", Area::SevenStarIsles},
        {"OverlookIsland", Area::OverlookIsland},
        {"FourEyeReef", Area::FourEyeReef},
        {"MotherAndChildIsles", Area::MotherAndChildIsles},
        {"SpectacleIsland", Area::SpectacleIsland},
        {"WindfallIsland", Area::WindfallIsland},
        {"PawprintIsle", Area::PawprintIsle},
        {"DragonRoostIsland", Area::DragonRoostIsland},
        {"FlightControlPlatform", Area::FlightControlPlatform},
        {"WesternFairyIsland", Area::WesternFairyIsland},
        {"RockSpireIsle", Area::RockSpireIsle},
        {"TingleIsland", Area::TingleIsland},
        {"NorthernTriangleIsland", Area::NorthernTriangleIsland},
        {"EasternFairyIsland", Area::EasternFairyIsland},
        {"FireMountain", Area::FireMountain},
        {"StarBeltArchipelago", Area::StarBeltArchipelago},
        {"ThreeEyeReef", Area::ThreeEyeReef},
        {"GreatfishIsle", Area::GreatfishIsle},
        {"CyclopsReef", Area::CyclopsReef},
        {"SixEyeReef", Area::SixEyeReef},
        {"TowerOfTheGods", Area::TowerOfTheGods},
        {"EasternTriangleIsland", Area::EasternTriangleIsland},
        {"ThornedFairyIsland", Area::ThornedFairyIsland},
        {"NeedleRockIsle", Area::NeedleRockIsle},
        {"IsletOfSteel", Area::IsletOfSteel},
        {"StonewatcherIsland", Area::StonewatcherIsland},
        {"SouthernTriangleIsland", Area::SouthernTriangleIsland},
        {"PrivateOasis", Area::PrivateOasis},
        {"BombIsland", Area::BombIsland},
        {"BirdsPeakRock", Area::BirdsPeakRock},
        {"DiamondSteppeIsland", Area::DiamondSteppeIsland},
        {"FiveEyeReef", Area::FiveEyeReef},
        {"SharkIsland", Area::SharkIsland},
        {"SouthernFairyIsland", Area::SouthernFairyIsland},
        {"IceRingIsle", Area::IceRingIsle},
        {"ForestHaven", Area::ForestHaven},
        {"CliffPlateauIsles", Area::CliffPlateauIsles},
        {"HorseshoeIsle", Area::HorseshoeIsle},
        {"OutsetIsland", Area::OutsetIsland},
        {"OutsetNearBridge", Area::OutsetNearBridge},
        {"OutsetAcrossBridge", Area::OutsetAcrossBridge},
        {"OutsetNearSavageHeadstone", Area::OutsetNearSavageHeadstone},
        {"OutsetLinksHouse", Area::OutsetLinksHouse},
        {"OutsetUnderLinksHouse", Area::OutsetUnderLinksHouse},
        {"OutsetOrcasHouse", Area::OutsetOrcasHouse},
        {"OutsetSturgeonsHouse", Area::OutsetSturgeonsHouse},
        {"OutsetRosesHouse", Area::OutsetRosesHouse},
        {"OutsetRosesAttic", Area::OutsetRosesAttic},
        {"OutsetMesasHouse", Area::OutsetMesasHouse},
        {"OutsetForestofFairies", Area::OutsetForestofFairies},
        {"OutsetGreatFairyFountain", Area::OutsetGreatFairyFountain},
        {"OutsetSavageLabyrinth", Area::OutsetSavageLabyrinth},
        {"OutsetJabunsCave", Area::OutsetJabunsCave},
        {"HeadstoneIsland", Area::HeadstoneIsland},
        {"TwoEyeReef", Area::TwoEyeReef},
        {"AngularIsles", Area::AngularIsles},
        {"BoatingCourse", Area::BoatingCourse},
        {"FiveStarIsles", Area::FiveStarIsles},
    };

    if (nameAreaMap.count(name) == 0)
    {
        return Area::INVALID;
    }
    return nameAreaMap.at(name);
}

std::string areaToName(const Area& area)
{
    static std::unordered_map<Area, std::string> areaNameMap = {
        {Area::Root, "Root"},
        {Area::LinksSpawn, "LinksSpawn"},
        {Area::TheGreatSea, "TheGreatSea"},
        {Area::ForsakenFortress, "ForsakenFortress"},
        {Area::StarIsland, "StarIsland"},
        {Area::StarIslandCave, "StarIslandCave"},
        {Area::NorthernFairyIsland, "NorthernFairyIsland"},
        {Area::GaleIsle, "GaleIsle"},
        {Area::CrescentMoonIsland, "CrescentMoonIsland" },
        {Area::SevenStarIsles, "SevenStarIsles"},
        {Area::OverlookIsland, "OverlookIsland"},
        {Area::FourEyeReef, "FourEyeReef"},
        {Area::MotherAndChildIsles, "MotherAndChildIsles"},
        {Area::SpectacleIsland, "SpectacleIsland"},
        {Area::WindfallIsland, "WindfallIsland"},
        {Area::PawprintIsle, "PawprintIsle"},
        {Area::DragonRoostIsland, "DragonRoostIsland"},
        {Area::FlightControlPlatform, "FlightControlPlatform"},
        {Area::WesternFairyIsland, "WesternFairyIsland"},
        {Area::RockSpireIsle, "RockSpireIsle"},
        {Area::TingleIsland, "TingleIsland"},
        {Area::NorthernTriangleIsland, "NorthernTriangleIsland"},
        {Area::EasternFairyIsland, "EasternFairyIsland"},
        {Area::FireMountain, "FireMountain"},
        {Area::StarBeltArchipelago, "StarBeltArchipelago"},
        {Area::ThreeEyeReef, "ThreeEyeReef"},
        {Area::GreatfishIsle, "GreatfishIsle"},
        {Area::CyclopsReef, "CyclopsReef"},
        {Area::SixEyeReef, "SixEyeReef"},
        {Area::TowerOfTheGods, "TowerOfTheGods"},
        {Area::EasternTriangleIsland, "EasternTriangleIsland"},
        {Area::ThornedFairyIsland, "ThornedFairyIsland"},
        {Area::NeedleRockIsle, "NeedleRockIsle"},
        {Area::IsletOfSteel, "IsletOfSteel"},
        {Area::StonewatcherIsland, "StonewatcherIsland"},
        {Area::SouthernTriangleIsland, "SouthernTriangleIsland"},
        {Area::PrivateOasis, "PrivateOasis"},
        {Area::BombIsland, "BombIsland"},
        {Area::BirdsPeakRock, "BirdsPeakRock"},
        {Area::DiamondSteppeIsland, "DiamondSteppeIsland"},
        {Area::FiveEyeReef, "FiveEyeReef"},
        {Area::SharkIsland, "SharkIsland"},
        {Area::SouthernFairyIsland, "SouthernFairyIsland"},
        {Area::IceRingIsle, "IceRingIsle"},
        {Area::ForestHaven, "ForestHaven"},
        {Area::CliffPlateauIsles, "CliffPlateauIsles"},
        {Area::HorseshoeIsle, "HorseshoeIsle"},
        {Area::OutsetIsland, "OutsetIsland"},
        {Area::OutsetNearBridge, "OutsetNearBridge"},
        {Area::OutsetAcrossBridge, "OutsetAcrossBridge"},
        {Area::OutsetNearSavageHeadstone, "OutsetNearSavageHeadstone"},
        {Area::OutsetLinksHouse, "OutsetLinksHouse"},
        {Area::OutsetUnderLinksHouse, "OutsetUnderLinksHouse"},
        {Area::OutsetOrcasHouse, "OutsetOrcasHouse"},
        {Area::OutsetSturgeonsHouse, "OutsetSturgeonsHouse"},
        {Area::OutsetRosesHouse, "OutsetRosesHouse"},
        {Area::OutsetRosesAttic, "OutsetRosesAttic"},
        {Area::OutsetMesasHouse, "OutsetMesasHouse"},
        {Area::OutsetForestofFairies, "OutsetForestofFairies"},
        {Area::OutsetGreatFairyFountain, "OutsetGreatFairyFountain"},
        {Area::OutsetSavageLabyrinth, "OutsetSavageLabyrinth"},
        {Area::OutsetJabunsCave, "OutsetJabunsCave"},
        {Area::HeadstoneIsland, "HeadstoneIsland"},
        {Area::TwoEyeReef, "TwoEyeReef"},
        {Area::AngularIsles, "AngularIsles"},
        {Area::BoatingCourse, "BoatingCourse"},
        {Area::FiveStarIsles, "FiveStarIsles"},
    };

    if (areaNameMap.count(area) == 0)
    {
        return "INVALID AREA";
    }
    return areaNameMap.at(area);
}

uint32_t areaAsIndex(Area area)
{
    return static_cast<std::underlying_type_t<Area>>(area);
}

Area indexAsArea(uint32_t index)
{
    if (index >= AREA_COUNT) return Area::INVALID;
    return static_cast<Area>(index);
}
