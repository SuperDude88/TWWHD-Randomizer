
#pragma once

#include <string>

enum struct Area : uint32_t
{
    INVALID = 0,
    Root,
    LinksSpawn,
    TheGreatSea,
    ForsakenFortress,
    StarIsland,
    StarIslandCave,
    NorthernFairyIsland,
    GaleIsle,
    CrescentMoonIsland,
    SevenStarIsles,
    OverlookIsland,
    FourEyeReef,
    MotherAndChildIsles,
    SpectacleIsland,
    WindfallIsland,
    PawprintIsle,
    DragonRoostIsland,
    FlightControlPlatform,
    WesternFairyIsland,
    RockSpireIsle,
    TingleIsland,
    NorthernTriangleIsland,
    EasternFairyIsland,
    FireMountain,
    StarBeltArchipelago,
    ThreeEyeReef,
    GreatfishIsle,
    CyclopsReef,
    SixEyeReef,
    TowerOfTheGods,
    EasternTriangleIsland,
    ThornedFairyIsland,
    NeedleRockIsle,
    IsletOfSteel,
    StonewatcherIsland,
    SouthernTriangleIsland,
    PrivateOasis,
    BombIsland,
    BirdsPeakRock,
    DiamondSteppeIsland,
    FiveEyeReef,
    SharkIsland,
    SouthernFairyIsland,
    IceRingIsle,
    ForestHaven,
    CliffPlateauIsles,
    HorseshoeIsle,
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
    HeadstoneIsland,
    TwoEyeReef,
    AngularIsles,
    BoatingCourse,
    FiveStarIsles,
    COUNT
};

#define AREA_COUNT static_cast<std::underlying_type_t<Area>>(Area::COUNT)

Area nameToArea(const std::string& name);
std::string areaToName(const Area& area);
uint32_t areaAsIndex(Area area);
Area indexAsArea(uint32_t index);
