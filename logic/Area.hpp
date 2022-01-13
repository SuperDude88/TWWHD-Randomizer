
#pragma once

#include <string>

enum struct Area : uint32_t
{
    INVALID = 0,
    Root,
    LinksSpawn,
    TheGreatSea,
    OutsetIsland,
    OutsetNearBridge,
    OutsetAcrossBridge,
    OutsetNearSavageHeadstone,
    OutsetLinksHouse,
    OutsetUnderLinksHouse,
    OutsetOrcasHouse,
    OutsetSturgeonsHouse,
    OutsetRosesHouse,
    OutsetRosesAttic,
    OutsetMesasHouse,
    OutsetForestofFairies,
    OutsetGreatFairyFountain,
    OutsetSavageLabyrinth,
    OutsetJabunsCave,
    COUNT
};

Area nameToArea(const std::string& name);

std::string areaToName(const Area& area);
