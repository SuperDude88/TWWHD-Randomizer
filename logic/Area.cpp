
#include "Area.hpp"
#include <unordered_map>

Area nameToArea(const std::string& name)
{
    static std::unordered_map<std::string, Area> nameAreaMap = {
        {"Root", Area::Root},
        {"LinksSpawn", Area::LinksSpawn},
        {"TheGreatSea", Area::TheGreatSea},
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
        {Area::OutsetGreatFairyFountain, "OutsetGreatFairyFountain",},
        {Area::OutsetSavageLabyrinth, "OutsetSavageLabyrinth"},
        {Area::OutsetJabunsCave, "OutsetJabunsCave"},
    };

    if (areaNameMap.count(area) == 0)
    {
        return "INVALID AREA";
    }
    return areaNameMap.at(area);
}
