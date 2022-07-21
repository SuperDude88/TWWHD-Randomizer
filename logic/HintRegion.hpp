
#pragma once

#include <string>

enum struct HintRegion
{
    NONE = 0,
    Mailbox,
    TheGreatSea,
    ForsakenFortressSector,
    StarIsland,
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
    TotGSector,
    EasternTriangleIsland,
    ThornedFairyIsland,
    NeedleRockIsle,
    IsletOfSteel,
    StoneWatcherIsland,
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
    HeadstoneIsland,
    TwoEyeReef,
    AngularIsles,
    BoatingCourse,
    FiveStarIsles,
    GhostShip,
    DragonRoostCavern,
    ForbiddenWoods,
    TowerOfTheGods,
    ForsakenFortress,
    EarthTemple,
    WindTemple,
    Hyrule,
    GanonsTower,
    COUNT,
    INVALID
};

HintRegion nameToHintRegion(const std::string& name);
std::string hintRegionToName(const HintRegion& hintRegion);
