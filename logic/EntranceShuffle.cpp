
#include "EntranceShuffle.hpp"
#include "../seedgen/random.hpp"
#include "PoolFunctions.hpp"
#include "../server/command/Log.hpp"
#include "Search.hpp"
#include <map>
#include <utility>

#define ENTRANCE_SHUFFLE_ERROR_CHECK(err) if (err != EntranceShuffleError::NONE) {DebugLog::getInstance().log("Error: " + errorToName(err)); return err;}
#define GET_COMPLETE_ITEM_POOL(itemPool, worlds) for (auto& world : worlds) {addElementsToPool(itemPool, world.getItemPool());}
#define GET_COMPLETE_PROGRESSION_LOCATION_POOL(locationPool, worlds) for (auto& world : worlds) {addElementsToPool(locationPool, world.getLocations(true));}

using EntrancePools = std::map<EntranceType, EntrancePool>;
using EntrancePair = std::pair<Entrance*, Entrance*>;

// The entrance randomization algorithm used here is heavily inspired by the entrance
// randomization algorithm from the Ocarina of Time Randomizer. While the algorithm
// has been adapted to work for The Wind Waker, credit also goes to those who developed
// it for Ocarina of Time.
//
// https://github.com/TestRunnerSRL/OoT-Randomizer/blob/Dev/EntranceShuffle.py

static std::list<EntranceInfoPair> entranceShuffleTable = {                                            //----File path info------|---entrance info-----|------boss stage info------//
                            // Parent Area                        Connected Area                        stage,   room, scls index,  stage,  room, spawn, boss stage, stage, room, spawn
    {EntranceType::DUNGEON, {"DragonRoostPondPastStatues",        "DRCFirstRoom",                       "Adanmae",  0,          2, "M_NewD2",  0,     0},
    /*Dragon Roost Cavern*/ {"DRCFirstRoom",                      "DragonRoostPondPastStatues",         "M_NewD2",  0,          0, "Adanmae",  0,     2, "M_DragB", "sea",  13, 211}},
    {EntranceType::DUNGEON, {"FWEntrancePlatform",                "FWFirstRoom",                        "sea",     41,          6, "kindan",   0,     0},
    /*Forbidden Woods*/     {"FWFirstRoom",                       "FWEntrancePlatform",                 "kindan",   0,          0, "sea",     41,     6, "kinBOSS", "Omori", 0, 215}},
    {EntranceType::DUNGEON, {"TotGSector",                        "TOTGEntranceRoom",                   "sea",     26,          0, "Siren",    0,     0},
    /*Tower of the Gods*/   {"TOTGEntranceRoom",                  "TotGSector",                         "Siren",    0,          0, "sea",     26,     2, "SirenB",  "sea",  26,   1}},
    {EntranceType::DUNGEON, {"HeadstoneIslandInterior",           "ETFirstRoom",                        "Edaichi",  0,          0, "M_Dai",  255,     0},
    /*Earth Temple*/        {"ETFirstRoom",                       "HeadstoneIslandInterior",            "M_Dai",    0,          0, "Edaichi",  0,     1, "M_DaiB",  "sea",  45, 229}},
    {EntranceType::DUNGEON, {"GaleIsleInterior",                  "WTFirstRoom",                        "Ekaze",    0,          0, "kaze",    15,    15},
    /*Wind Temple*/         {"WTFirstRoom",                       "GaleIsleInterior",                   "kaze",    15,          0, "Ekaze",    0,     1, "kazeB",   "sea",   4, 232}},

    {EntranceType::CAVE,    {"OutsetNearSavageHeadstone",         "OutsetSavageLabyrinth",              "sea",     44,          8, "Cave09",   0,     0},
                            {"OutsetSavageLabyrinth",             "OutsetNearSavageHeadstone",          "Cave09",   0,          1, "sea",     44,    10}},
    {EntranceType::CAVE,    {"DragonRoostIsland",                 "DragonRoostIslandCave",              "sea",     13,          2, "TF_06",    0,     0},
                            {"DragonRoostIslandCave",             "DragonRoostIsland",                  "TF_06",    0,          0, "sea",     13,     5}},
    {EntranceType::CAVE,    {"FireMountain",                      "FireMountainInterior",               "sea",     20,          0, "MiniKaz",  0,     0},
                            {"FireMountainInterior",              "FireMountain",                       "MiniKaz",  0,          0, "sea",     20,     0}},
    {EntranceType::CAVE,    {"IceRingIsle",                       "IceRingInterior",                    "sea",     40,          0, "MiniHyo",  0,     0},
                            {"IceRingInterior",                   "IceRingIsle",                        "MiniHyo",  0,          0, "sea",     40,     0}},
    {EntranceType::CAVE,    {"TheCabana",                         "CabanaLabyrinth",                    "Abesso",   0,          1, "TF_04",    0,     0},
                            {"CabanaLabyrinth",                   "TheCabana",                          "TF_04",    0,          0, "Abesso",   0,     1}},
    {EntranceType::CAVE,    {"NeedleRockIsle",                    "NeedleRockCave",                     "sea",     29,          0, "SubD42",   0,     0},
                            {"NeedleRockCave",                    "NeedleRockIsle",                     "SubD42",   0,          0, "sea",     29,     5}},
    {EntranceType::CAVE,    {"AngularIslesSmallIsle",             "AngularIslesCave",                   "sea",     47,          1, "SubD43",   0,     0},
                            {"AngularIslesCave",                  "AngularIslesSmallIsle",              "SubD43",   0,          0, "sea",     47,     5}},
    {EntranceType::CAVE,    {"BoatingCourseSmallIsle",            "BoatingCourseCave",                  "sea",     48,          0, "SubD71",   0,     0},
                            {"BoatingCourseCave",                 "BoatingCourseSmallIsle",             "SubD71",   0,          0, "sea",     48,     5}},
    {EntranceType::CAVE,    {"StoneWatcherIsland",                "StoneWatcherCave",                   "sea",     31,          0, "TF_01",    0,     0},
                            {"StoneWatcherCave",                  "StoneWatcherIsland",                 "TF_01",    0,          0, "sea",     31,     1}},
    {EntranceType::CAVE,    {"OverlookIslandUpperIsles",          "OverlookCave",                       "sea",      7,          0, "TF_02",    0,     0},
                            {"OverlookCave",                      "OverlookIslandUpperIsles",           "TF_02",    0,          0, "sea",      7,     1}},
    {EntranceType::CAVE,    {"BirdsPeakRockBehindBars",           "BirdsPeakRockCave",                  "sea",     35,          0, "TF_03",    0,     0},
                            {"BirdsPeakRockCave",                 "BirdsPeakRockBehindBars",            "TF_03",    0,          0, "sea",     35,     1}},
    {EntranceType::CAVE,    {"PawprintIsle",                      "PawprintChuChuCave",                 "sea",     12,          0, "TyuTyu",   0,     0},
                            {"PawprintChuChuCave",                "PawprintIsle",                       "TyuTyu",   0,          0, "sea",     12,     1}},
    {EntranceType::CAVE,    {"PawprintWizzrobeCaveIsle",          "PawprintWizzrobeCave",               "sea",     12,          1, "Cave07",   0,     0},
                            {"PawprintWizzrobeCave",              "PawprintWizzrobeCaveIsle",           "Cave07",   0,          0, "sea",     12,     5}},
    {EntranceType::CAVE,    {"DiamondSteppeUpperIsland",          "DiamondSteppeWarpMaze",              "sea",     36,          0, "WarpD",    0,     0},
                            {"DiamondSteppeWarpMaze",             "DiamondSteppeUpperIsland",           "WarpD",    0,          0, "sea",     36,     1}},
    {EntranceType::CAVE,    {"BombIsland",                        "BombIslandCave",                     "sea",     34,          0, "Cave01",   0,     0},
                            {"BombIslandCave",                    "BombIsland",                         "Cave01",   0,          0, "sea",     34,     1}},
    {EntranceType::CAVE,    {"RockSpireUpperLedges",              "RockSpireCave",                      "sea",     16,          0, "Cave04",   0,     0},
                            {"RockSpireCave",                     "RockSpireUpperLedges",               "Cave04",   0,          0, "sea",     16,     1}},
    {EntranceType::CAVE,    {"SharkIsland",                       "SharkIslandCave",                    "sea",     38,          0, "ITest63",  0,     0},
                            {"SharkIslandCave",                   "SharkIsland",                        "ITest63",  0,          0, "sea",     38,     5}},
    {EntranceType::CAVE,    {"CliffPlateauIsles",                 "CliffPlateauCave",                   "sea",     42,          0, "Cave03",   0,     0},
                            {"CliffPlateauCave",                  "CliffPlateauIsles",                  "Cave03",   0,          0, "sea",     42,     2}},
    {EntranceType::CAVE,    {"CliffPlateauHighestIsle",           "CliffPlateauCavePastWoodenBarrier",  "sea",     42,          1, "Cave03",   0,     1},
                            {"CliffPlateauCavePastWoodenBarrier", "CliffPlateauHighestIsle",            "Cave03",   0,          1, "sea",     42,     1}},
    {EntranceType::CAVE,    {"HorseshoeIslePastTentacles",        "HorseshoeCave",                      "sea",     43,          0, "Cave05",   0,     0},
                            {"HorseshoeCave",                     "HorseshoeIslePastTentacles",         "Cave05",   0,          0, "sea",     43,     5}},
    {EntranceType::CAVE,    {"StarIsland",                        "StarIslandCave",                     "sea",      2,          0, "Cave02",   0,     0},
                            {"StarIslandCave",                    "StarIsland",                         "Cave02",   0,          0, "sea",      2,     1}},

    {EntranceType::DOOR,    {"WindfallIsland",                    "WindfallJail",                       "sea",     11,          6, "Pnezumi",  0,     0},
                            {"WindfallJail",                      "WindfallIsland",                     "Pnezumi",  0,          0, "sea",     11,    13}},
    {EntranceType::DOOR,    {"WindfallIsland",                    "WindfallSchoolOfJoy",                "sea",     11,         12, "Nitiyou",  0,     0},
                            {"WindfallSchoolOfJoy",               "WindfallIsland",                     "Nitiyou",  0,          0, "sea",     11,    12}},
    {EntranceType::DOOR,    {"WindfallIsland",                    "WindfallLenzosHouseFromBottomEntry", "sea",     11,         10, "Ocmera",   0,     0},
                            {"WindfallLenzosHouseLower",          "WindfallIsland",                     "Ocmera",   0,          0, "sea",     11,    10}},
    {EntranceType::DOOR,    {"WindfallLenzosUpperLedge",          "WindfallLenzosHouseUpper",           "sea",     11,         11, "Ocmera",   0,     1},
                            {"WindfallLenzosHouseUpper",          "WindfallLenzosUpperLedge",           "Ocmera",   0,          1, "sea",     11,    11}},
    {EntranceType::DOOR,    {"WindfallIsland",                    "WindfallCafeBar",                    "sea",     11,          7, "Opub",     0,     0},
                            {"WindfallCafeBar",                   "WindfallIsland",                     "Opub",     0,          0, "sea",     11,     6}},
    {EntranceType::DOOR,    {"WindfallIsland",                    "WindfallBattleSquidInterior",        "sea",     11,          9, "Kaisen",   0,     0},
                            {"WindfallBattleSquidInterior",       "WindfallIsland",                     "Kaisen",   0,          0, "sea",     11,     9}},
    {EntranceType::DOOR,    {"WindfallBattleSquidUpperLedge",     "WindfallBattleSquidInterior",        "sea",     11,          8, "Kaisen",   0,     1},
                            {"WindfallBattleSquidInterior",       "WindfallBattleSquidUpperLedge",      "Kaisen",   0,          1, "sea",     11,     8}},
    {EntranceType::DOOR,    {"WindfallIsland",                    "WindfallHouseOfWealthLower",         "sea",     11,          3, "Orichh",   0,     0},
                            {"WindfallHouseOfWealthLower",        "WindfallIsland",                     "Orichh",   0,          1, "sea",     11,     3}},
    {EntranceType::DOOR,    {"WindfallIsland",                    "WindfallHouseOfWealthUpper",         "sea",     11,          4, "Orichh",   0,     1},
                            {"WindfallHouseOfWealthUpper",        "WindfallIsland",                     "Orichh",   0,          2, "sea",     11,     4}},
    {EntranceType::DOOR,    {"WindfallIsland",                    "WindfallPotionShop",                 "sea",     11,          5, "Pdrgsh",   0,     0},
                            {"WindfallPotionShop",                "WindfallIsland",                     "Pdrgsh",   0,          0, "sea",     11,     7}},
    {EntranceType::DOOR,    {"WindfallIsland",                    "WindfallBombShop",                   "sea",     11,          1, "Obombh",   0,     0},
                            {"WindfallBombShop",                  "WindfallIsland",                     "Obombh",   0,          1, "sea",     11,     1}},
//  {EntranceType::DOOR,    {"WindfallIsland",                    "WindfallPirateShip",                 "",        -1,         -1, "",        -1,    -1},
//                          {"WindfallPirateShip",                "WindfallIsland",                     "",        -1,         -1, "",        -1,    -1}},
    {EntranceType::DOOR,    {"DragonRoostRitoAerie",              "DragonRoostKomalisRoom",             "Atorizk",  0,          4, "Comori",   0,     0},
                            {"DragonRoostKomalisRoom",            "DragonRoostRitoAerie",               "Comori",   0,          0, "Atorizk",  0,     4}},
    {EntranceType::DOOR,    {"PrivateOasis",                      "TheCabana",                          "sea",     33,          0, "Abesso",   0,     0},
                            {"TheCabana",                         "PrivateOasis",                       "Abesso",   0,          0, "sea",     33,     1}},
    {EntranceType::DOOR,    {"OutsetIsland",                      "OutsetLinksHouse",                   "sea",     44,          0, "LinkRM",   0,     1},
                            {"OutsetLinksHouse",                  "OutsetIsland",                       "LinkRM",   0,          0, "sea",     44,     1}},
    {EntranceType::DOOR,    {"OutsetIsland",                      "OutsetOrcasHouse",                   "sea",     44,          1, "Ojhous",   0,     0},
                            {"OutsetOrcasHouse",                  "OutsetIsland",                       "Ojhous",   0,          0, "sea",     44,     3}},
    {EntranceType::DOOR,    {"OutsetIsland",                      "OutsetSturgeonsHouse",               "sea",     44,          2, "Ojhous2",  1,     0},
                            {"OutsetSturgeonsHouse",              "OutsetIsland",                       "Ojhous2",  1,          0, "sea",     44,     2}},
    {EntranceType::DOOR,    {"OutsetIsland",                      "OutsetRosesHouse",                   "sea",     44,          4, "Onobuta",  0,     0},
                            {"OutsetRosesHouse",                  "OutsetIsland",                       "Onobuta",  0,          0, "sea",     44,     4}},
    {EntranceType::DOOR,    {"OutsetIsland",                      "OutsetMesasHouse",                   "sea",     44,          3, "Omasao",   0,     0},
                            {"OutsetMesasHouse",                  "OutsetIsland",                       "Omasao",   0,          0, "sea",     44,     7}},

    // MISC Entrances are those which don't fit into any of the above categories,
    // but which are still interesting for entrance randomizer. These are all
    // the "open-air" entrances on the overworld (excluding Hyrule).
    //
    // MISC_RESTRICTIVE are entrances which mostly lead to dead-ends. When randomizing
    // these entrances we want to attempt randomization with the entrance which goes
    // *to* the dead-end, not *from* it. This significantly reduces the chances
    // of entrance placement failure.
    //
    // MISC_CRAWLSPACE is separated for the time being until potential softlocks
    // with mixing crawlspace/non-crawlspace entrances can be resolved.
    {EntranceType::MISC_RESTRICTIVE, {"GaleIsle",                          "GaleIsleInterior",                   "sea",      4, 0, "Ekaze",    0,     0},
                                     {"GaleIsleInterior",                  "GaleIsle",                           "Ekaze",    0, 1, "sea",      4,     1}},
    {EntranceType::MISC_CRAWLSPACE,  {"WindfallIsland",                    "WindfallBombShopUpperLedge",         "sea",     11, 2, "Obombh",   0,     1},
                                     {"WindfallBombShopUpperLedge",        "WindfallIsland",                     "Obombh",   0, 2, "sea",     11,     2}},
    {EntranceType::MISC,             {"DragonRoostIsland",                 "DragonRoostRitoAerie",               "sea",     13, 0, "Atorizk",  0,     0},
                                     {"DragonRoostRitoAerie",              "DragonRoostIsland",                  "Atorizk",  0, 0, "sea",     13,     1}},
    {EntranceType::MISC,             {"DragonRoostIslandFlightDeck",       "DragonRoostRitoAerie",               "sea",     13, 1, "Atorizk",  0,     1},
                                     {"DragonRoostRitoAerie",              "DragonRoostIslandFlightDeck",        "Atorizk",  0, 1, "sea",     13,     2}},
    {EntranceType::MISC,             {"DragonRoostPond",                   "DragonRoostRitoAerie",               "Adanmae",  0, 0, "Atorizk",  0,     2},
                                     {"DragonRoostRitoAerie",              "DragonRoostPond",                    "Atorizk",  0, 2, "Adanmae",  0,     0}},
    {EntranceType::MISC_RESTRICTIVE, {"DragonRoostRitoAerie",              "DragonRoostPondUpperLedge",          "Atorizk",  0, 3, "Adanmae",  0,     1},
                                     {"DragonRoostPondUpperLedge",         "DragonRoostRitoAerie",               "Adanmae",  0, 1, "Atorizk",  0,     3}},
    {EntranceType::MISC_RESTRICTIVE, {"IsletOfSteel",                      "IsletOfSteelInterior",               "sea",     30, 0, "ShipD",    0,     0},
                                     {"IsletOfSteelInterior",              "IsletOfSteel",                       "ShipD",     0, 0, "sea",     30,     1}},
    {EntranceType::MISC,             {"ForestHaven",                       "ForestHavenInterior",                "sea",     41, 5, "Omori",    0,     5},
                                     {"ForestHavenInterior",               "ForestHaven",                        "Omori",    0, 5, "sea",     41,     5}},
    {EntranceType::MISC_RESTRICTIVE, {"ForestHaven",                       "ForestHavenWaterfallCave",           "sea",     41, 7, "Otkura",   0,     0},
                                     {"ForestHavenWaterfallCave",          "ForestHaven",                        "Otkura",   0, 0, "sea",     41,     9}},
    {EntranceType::MISC_RESTRICTIVE, {"ForestHavenInterior",               "ForestPotionShop",                   "Omori",    0, 0, "Ocrogh",   0,     0},
                                     {"ForestPotionShop",                  "ForestHavenInterior",                "Ocrogh",   0, 0, "Omori",    0,     0}},
    {EntranceType::MISC,             {"ForestHavenExteriorNorthLedge",     "ForestHavenInteriorNorthLedge",      "sea",     41, 1, "Omori",    0,     1},
                                     {"ForestHavenInteriorNorthLedge",     "ForestHavenExteriorNorthLedge",      "Omori",    0, 1, "sea",     41,     1}},
    {EntranceType::MISC,             {"ForestHavenExteriorWestLowerLedge", "ForestHavenInteriorWestLowerLedge",  "sea",     41, 3, "Omori",    0,     3},
                                     {"ForestHavenInteriorWestLowerLedge", "ForestHavenExteriorWestLowerLedge",  "Omori",    0, 3, "sea",     41,     3}},
    {EntranceType::MISC,             {"ForestHavenExteriorWestUpperLedge", "ForestHavenInteriorWestUpperLedge",  "sea",     41, 2, "Omori",    0,     2},
                                     {"ForestHavenInteriorWestUpperLedge", "ForestHavenExteriorWestUpperLedge",  "Omori",    0, 2, "sea",     41,     2}},
    {EntranceType::MISC,             {"ForestHavenExteriorSouthLedge",     "ForestHavenInteriorSouthLedge",      "sea",     41, 4, "Omori",    0,     4},
                                     {"ForestHavenInteriorSouthLedge",     "ForestHavenExteriorSouthLedge",      "Omori",    0, 4, "sea",     41,     4}},
    {EntranceType::MISC_CRAWLSPACE,  {"OutsetIsland",                      "OutsetUnderLinksHouse",              "sea",     44, 9, "LinkUG",   0,     1},
                                     {"OutsetUnderLinksHouse",             "OutsetIsland",                       "LinkUG",   0, 0, "sea",     44,    11}},
    {EntranceType::MISC_RESTRICTIVE, {"OutsetAcrossBridge",                "OutsetForestOfFairies",              "sea",     44, 6, "A_mori",   0,     0},
                                     {"OutsetForestOfFairies",             "OutsetAcrossBridge",                 "A_mori",   0, 0, "sea",     44,     8}},
    {EntranceType::MISC_RESTRICTIVE, {"OutsetIsland",                      "OutsetJabunsCave",                   "sea",     44, 7, "Pjavdou",  0,     0},
                                     {"OutsetJabunsCave",                  "OutsetIsland",                       "Pjavdou",  0, 0, "sea",     44,     9}},
    {EntranceType::MISC_RESTRICTIVE, {"HeadstoneIsland",                   "HeadstoneIslandInterior",            "sea",     45, 0, "Edaichi",  0,     0},
                                     {"HeadstoneIslandInterior",           "HeadstoneIsland",                    "Edaichi",  0, 1, "sea",     45,     1}},
};

static void logEntrancePool(EntrancePool& entrancePool, const std::string& poolName)
{
    DebugLog::getInstance().log(poolName + ": [");
    for (auto entrance : entrancePool)
    {
        DebugLog::getInstance().log("\t" + entrance->getOriginalName());
    }
    DebugLog::getInstance().log("]");
}

static void logMissingLocations(WorldPool& worlds)
{
    static int identifier = 0;
    DebugLog::getInstance().log("Missing Locations: [");
    for (auto& world : worlds)
    {
        for (auto& location : world.locationEntries)
        {
            if (!location.hasBeenFound && location.locationId != LocationId::INVALID)
            {
                DebugLog::getInstance().log("\t" + locationIdToName(location.locationId));
                if (identifier++ < 10)
                {
                    // world.dumpWorldGraph(std::to_string(identifier));
                    // DebugLog::getInstance().log("Now Dumping " + std::to_string(identifier));
                }
                break;
            }
        }
    }
    DebugLog::getInstance().log("]");
}

static void setAllEntrancesData(World& world)
{
    for (auto& [type, forwardEntry, returnEntry] : entranceShuffleTable)
    {
        auto& forwardEntrance = world.getEntrance(forwardEntry.parentArea, forwardEntry.connectedArea);
        forwardEntrance.setFilepathStage(forwardEntry.filepathStage);
        forwardEntrance.setFilepathRoomNum(forwardEntry.filepathRoom);
        forwardEntrance.setSclsExitIndex(forwardEntry.sclsExitIndex);
        forwardEntrance.setStageName(forwardEntry.stage);
        forwardEntrance.setRoomNum(forwardEntry.room);
        forwardEntrance.setSpawnId(forwardEntry.spawnId);
        forwardEntrance.setBossFilepathStageName(forwardEntry.bossFilepathStage);
        forwardEntrance.setBossOutStageName(forwardEntry.bossOutStage);
        forwardEntrance.setBossOutRoomNum(forwardEntry.bossOutRoom);
        forwardEntrance.setBossOutSpawnId(forwardEntry.bossOutSpawnId);
        forwardEntrance.setEntranceType(type);
        forwardEntrance.setAsPrimary();
        if (returnEntry.parentArea != "INVALID")
        {
            auto& returnEntrance = world.getEntrance(returnEntry.parentArea, returnEntry.connectedArea);
            returnEntrance.setFilepathStage(returnEntry.filepathStage);
            returnEntrance.setFilepathRoomNum(returnEntry.filepathRoom);
            returnEntrance.setSclsExitIndex(returnEntry.sclsExitIndex);
            returnEntrance.setStageName(returnEntry.stage);
            returnEntrance.setRoomNum(returnEntry.room);
            returnEntrance.setSpawnId(returnEntry.spawnId);
            returnEntrance.setBossFilepathStageName(returnEntry.bossFilepathStage);
            returnEntrance.setBossOutStageName(returnEntry.bossOutStage);
            returnEntrance.setBossOutRoomNum(returnEntry.bossOutRoom);
            returnEntrance.setBossOutSpawnId(returnEntry.bossOutSpawnId);
            returnEntrance.setEntranceType(type);
            forwardEntrance.bindTwoWay(&returnEntrance);
        }
    }
}

// Disconnect each entrance in the pool and create a corresponding target entrance
// which is connected to the root of the world graph. This creates the assumption
// that we have access to all the connected areas of each entrance
static EntrancePool assumeEntrancePool(EntrancePool& entrancePool)
{
    EntrancePool assumedPool = {};
    for (auto entrance : entrancePool)
    {
        auto assumedForward = entrance->assumeReachable();
        if (entrance->getReverse() != nullptr && !entrance->getWorld()->getSettings().decouple_entrances)
        {
            auto assumedReturn = entrance->getReverse()->assumeReachable();
            assumedForward->bindTwoWay(assumedReturn);
        }
        assumedPool.push_back(assumedForward);
    }
    return assumedPool;
}

static EntranceShuffleError checkEntrancesCompatibility(Entrance* entrance, Entrance* target, std::vector<EntrancePair>& rollbacks)
{
    // While self-connections are funny, they get old pretty quickly and are more
    // prone to failure for the entrance placement algorithm. So we'll forbid them
    if (entrance->getReverse() != nullptr)
    {
        if (target->getReplaces() == entrance->getReverse())
        {
            return EntranceShuffleError::ATTEMPTED_SELF_CONNECTION;
        }
    }
    return EntranceShuffleError::NONE;
}

static void changeConnections(Entrance* entrance, Entrance* targetEntrance)
{
    entrance->connect(targetEntrance->disconnect());
    entrance->setReplaces(targetEntrance->getReplaces());
    if (entrance->getReverse() != nullptr && !entrance->getWorld()->getSettings().decouple_entrances)
    {
        targetEntrance->getReplaces()->getReverse()->connect(entrance->getReverse()->getAssumed()->disconnect());
        targetEntrance->getReplaces()->getReverse()->setReplaces(entrance->getReverse());
    }
}

static void restoreConnections(Entrance* entrance, Entrance* targetEntrance)
{
    DebugLog::getInstance().log("Restoring Connection for " + entrance->getOriginalName());
    targetEntrance->connect(entrance->disconnect());
    entrance->setReplaces(nullptr);
    if (entrance->getReverse() != nullptr && !entrance->getWorld()->getSettings().decouple_entrances)
    {
        entrance->getReverse()->getAssumed()->connect(targetEntrance->getReplaces()->getReverse()->disconnect());
        targetEntrance->getReplaces()->getReverse()->setReplaces(nullptr);
    }
}

static void deleteTargetEntrance(Entrance* targetEntrance)
{
    if (targetEntrance->getConnectedArea() != "INVALID")
    {
        targetEntrance->disconnect();
    }
    if (targetEntrance->getParentArea() != "INVALID")
    {
        targetEntrance->getWorld()->removeEntrance(targetEntrance);
    }
}

static void confirmReplacement(Entrance* entrance, Entrance* targetEntrance)
{
    deleteTargetEntrance(targetEntrance);
    if (entrance->getReverse() != nullptr && !entrance->getWorld()->getSettings().decouple_entrances)
    {
        deleteTargetEntrance(entrance->getReverse()->getAssumed());
    }
}

static EntranceShuffleError validateWorld(WorldPool& worlds, Entrance* entrancePlaced, ItemPool& itemPool)
{
    DebugLog::getInstance().log("Validating World");
    // Ensure that all item locations are still reachable within the given world
    if (!allLocationsReachable(worlds, itemPool))
    {
        DebugLog::getInstance().log("Error: All locations not reachable");
        #ifdef ENABLE_DEBUG
            logMissingLocations(worlds);
        #endif
        return EntranceShuffleError::ALL_LOCATIONS_NOT_REACHABLE;
    }
    // Ensure that there's at least one sphere zero location available to place an item
    // for the beginning of the seed
    ItemPool noItems = {};
    LocationPool progLocations = {};
    GET_COMPLETE_PROGRESSION_LOCATION_POOL(progLocations, worlds);
    if (getAccessibleLocations(worlds, noItems, progLocations).size() < worlds.size())
    {
        DebugLog::getInstance().log("Error: Not enough sphere zero locations to place items");
        return EntranceShuffleError::NOT_ENOUGH_SPHERE_ZERO_LOCATIONS;
    }
    // Ensure that all race mode dungeons are assigned to a single island and that
    // there aren't any other dungeons on those islands. Since quest markers for
    // race mode dungeons indicate an entire island, we don't want the there to be
    // multiple dungeons on an island
    for (auto& world : worlds)
    {
        if (world.getSettings().race_mode)
        {
            std::unordered_set<HintRegion> raceModeIslands = {};
            for (auto& dungeon : getDungeonList())
            {
                auto dungeonFirstRoom = dungeonIdToFirstRoom(dungeon);
                auto dungeonIslands = world.getIslands(dungeonFirstRoom);

                if (world.raceModeDungeons.count(dungeon) > 0) //contains() is c++20
                {
                    if (dungeonIslands.size() > 1)
                    {
                        DebugLog::getInstance().log("Error: More than 1 island leading to race mode dungeon " + dungeonIdToName(dungeon));
                        return EntranceShuffleError::AMBIGUOUS_RACE_MODE_ISLAND;
                    }
                }

                if (dungeonIslands.size() == 1)
                {
                    auto dungeonIsland = *dungeonIslands.begin();
                    if (raceModeIslands.count(dungeonIsland) > 0)
                    {
                        DebugLog::getInstance().log("Error: Island " + hintRegionToName(dungeonIsland) + " has an ambiguous race mode dungeon");
                        return EntranceShuffleError::AMBIGUOUS_RACE_MODE_DUNGEON;
                    }

                    if (world.raceModeDungeons.count(dungeon) > 0)
                    {
                        raceModeIslands.insert(dungeonIsland);
                    }
                }
            }
        }
    }
    return EntranceShuffleError::NONE;
}

// Attempt to connect the given entrance to the given target and verify that the
// new world graph is valid for this world's settings
static EntranceShuffleError replaceEntrance(WorldPool& worlds, Entrance* entrance, Entrance* target, std::vector<EntrancePair>& rollbacks, ItemPool& itemPool)
{
    DebugLog::getInstance().log("Attempting to Connect " + entrance->getOriginalName() + " To " + target->getReplaces()->getOriginalName());
    EntranceShuffleError err = EntranceShuffleError::NONE;
    err = checkEntrancesCompatibility(entrance, target, rollbacks);
    ENTRANCE_SHUFFLE_ERROR_CHECK(err);
    changeConnections(entrance, target);
    err = validateWorld(worlds, entrance, itemPool);
    // If the attempted replacement produces an invalid world graph, then undo
    // the attempted connection and try again with a different target.
    if (err != EntranceShuffleError::NONE)
    {
        if (entrance->getConnectedArea() != "INVALID")
        {
            restoreConnections(entrance, target);
        }
        return err;
    }
    rollbacks.push_back({entrance, target});
    return EntranceShuffleError::NONE;
}

static EntranceShuffleError shuffleEntrances(WorldPool& worlds, EntrancePool& entrances, EntrancePool& targetEntrances, std::vector<EntrancePair>& rollbacks)
{

    ItemPool completeItemPool = {};
    GET_COMPLETE_ITEM_POOL(completeItemPool, worlds)

    shufflePool(entrances);

    // Place all entrances in the pool, validating worlds after each placement.
    // We first choose a random entrance from the list of entrances and attempt
    // to connect it with random target entrances from the target pool until
    // one of the connections produces a valid world graph.
    for (auto entrance : entrances)
    {
        EntranceShuffleError err = EntranceShuffleError::NONE;
        if (entrance->getConnectedArea() != "INVALID")
        {
            continue;
        }
        shufflePool(targetEntrances);

        for (auto target : targetEntrances)
        {
            // If the target has already been disconnected, then don't use it again
            if (target->getConnectedArea() == "INVALID")
            {
                continue;
            }
            err = replaceEntrance(worlds, entrance, target, rollbacks, completeItemPool);
            if (err == EntranceShuffleError::NONE)
            {
                break;
            }
        }

        if (entrance->getConnectedArea() == "INVALID")
        {
            DebugLog::getInstance().log("Could not connect " + entrance->getOriginalName() + ". Error: " + errorToName(err));
            return EntranceShuffleError::NO_MORE_VALID_ENTRANCES;
        }
    }

    // Verify that all targets were disconnected and that we didn't create any closed root loops
    for (auto target : targetEntrances)
    {
        if (target->getConnectedArea() != "INVALID")
        {
            DebugLog::getInstance().log("Error: Target entrance " + target->getCurrentName() + " was never disconnected");
            return EntranceShuffleError::FAILED_TO_DISCONNECT_TARGET;
        }
    }

    return EntranceShuffleError::NONE;
}

// Attempt to shuffle the given entrance pool with the given pool of target entrances
// When choosing connections randomly, it's possible that the algorithm can lock itself
// out of producing a valid world which is why we give it some number of retries
// in hopes that a complete failure with valid settings is exceedingly rare.
static EntranceShuffleError shuffleEntrancePool(World& world, WorldPool& worlds, EntrancePool& entrancePool, EntrancePool& targetEntrances, int retryCount = 20)
{
    EntranceShuffleError err = EntranceShuffleError::NONE;

    while (retryCount > 0)
    {
        retryCount--;
        std::vector<EntrancePair> rollbacks = {};

        err = shuffleEntrances(worlds, entrancePool, targetEntrances, rollbacks);
        if (err != EntranceShuffleError::NONE)
        {
            DebugLog::getInstance().log("Failed to place all entrances in a pool for world " + std::to_string(world.getWorldId() + 1) + ". Will retry " + std::to_string(retryCount) + " more times.");
            DebugLog::getInstance().log("Last Error: " + errorToName(err));
            for (auto& [entrance, target] : rollbacks)
            {
                restoreConnections(entrance, target);
            }
            continue;
        }

        for (auto& [entrance, target] : rollbacks)
        {
            confirmReplacement(entrance, target);
        }
        return EntranceShuffleError::NONE;
    }

    DebugLog::getInstance().log("Entrance placement attempt count exceeded for world " + std::to_string(world.getWorldId() + 1));
    return EntranceShuffleError::RAN_OUT_OF_RETRIES;
}

// Helper function for getting the reverse entrances from a given entrance pool
static EntrancePool getReverseEntrances(EntrancePools& entrancePools, EntranceType type)
{
    EntrancePool reversePool = {};
    for (auto entrance : entrancePools[type])
    {
        reversePool.push_back(entrance->getReverse());
    }
    return reversePool;
}

// Set all entrances in the passed in pools as shuffled
static void SetShuffledEntrances(EntrancePools& entrancePools) {
    for (auto& [type, entrancePool] : entrancePools)
    {
        for (auto entrance : entrancePool)
        {
            entrance->setAsShuffled();
            if (entrance->getReverse() != nullptr)
            {
                entrance->getReverse()->setAsShuffled();
            }
        }
    }
}

EntranceShuffleError randomizeEntrances(WorldPool& worlds)
{
    EntranceShuffleError err = EntranceShuffleError::NONE;

    ItemPool completeItemPool;
    GET_COMPLETE_ITEM_POOL(completeItemPool, worlds);

    // Shuffle all entrances within their own world
    for (auto& world : worlds)
    {
        // Set random starting island
        if (world.getSettings().randomize_starting_island)
        {
            // Rooms 2 - 49 include every island except Forsaken Fortress
            world.startingIslandRoomIndex = Random(2, 50);
            auto startingIsland = roomIndexToIslandName(world.startingIslandRoomIndex);
            startingIsland.erase(std::remove(startingIsland.begin(), startingIsland.end(), ' '), startingIsland.end());

            // Set the new starting island in the world graph
            auto& linksSpawn = world.getEntrance("LinksSpawn", "OutsetIsland");
            linksSpawn.setConnectedArea(startingIsland);
            world.areaEntries["OutsetIsland"].entrances.remove(&linksSpawn);
            world.areaEntries[startingIsland].entrances.push_back(&linksSpawn);
        }

        // Set entrance data for all entrances, even those we aren't shuffling
        setAllEntrancesData(world);

        // Determine entrance pools based on settings, to be shuffled in the order we set them by
        EntrancePools entrancePools = {};
        EntrancePools targetEntrancePools = {};
        if (world.getSettings().randomize_dungeon_entrances)
        {
            entrancePools[EntranceType::DUNGEON] = world.getShuffleableEntrances(EntranceType::DUNGEON, true);
            if (world.getSettings().decouple_entrances)
            {
                entrancePools[EntranceType::DUNGEON_REVERSE] = getReverseEntrances(entrancePools, EntranceType::DUNGEON);
            }
        }

        if (world.getSettings().randomize_cave_entrances)
        {
            entrancePools[EntranceType::CAVE] = world.getShuffleableEntrances(EntranceType::CAVE, true);
            if (world.getSettings().decouple_entrances)
            {
                entrancePools[EntranceType::CAVE_REVERSE] = getReverseEntrances(entrancePools, EntranceType::CAVE);
            }
            // Don't randomize the cliff plateau upper isles grotto unless entrances are decoupled
            else
            {
                filterAndEraseFromPool(entrancePools[EntranceType::CAVE], [](Entrance* e){return e->getParentArea() == "CliffPlateauHighestIsle";});
            }
        }

        if (world.getSettings().randomize_door_entrances)
        {
            entrancePools[EntranceType::DOOR] = world.getShuffleableEntrances(EntranceType::DOOR, true);
            if (world.getSettings().decouple_entrances)
            {
                entrancePools[EntranceType::DOOR_REVERSE] = getReverseEntrances(entrancePools, EntranceType::DOOR);
            }
        }

        if (world.getSettings().randomize_misc_entrances)
        {
            // Allow both primary and non-primary entrances in the MISC pool unless
            // we're mixing the entrance pools and aren't decoupling entrances
            entrancePools[EntranceType::MISC] = world.getShuffleableEntrances(EntranceType::MISC, world.getSettings().mix_entrance_pools && !world.getSettings().decouple_entrances);
            auto miscRestrictiveEntrances = world.getShuffleableEntrances(EntranceType::MISC_RESTRICTIVE, !world.getSettings().decouple_entrances);
            addElementsToPool(entrancePools[EntranceType::MISC], miscRestrictiveEntrances);

            // Keep crawlspaces separate for the time-being since spawning in a crawlspace
            // entrance while standing up can potentially softlock
            entrancePools[EntranceType::MISC_CRAWLSPACE] = world.getShuffleableEntrances(EntranceType::MISC_CRAWLSPACE, true);
            if (world.getSettings().decouple_entrances)
            {
                entrancePools[EntranceType::MISC_CRAWLSPACE_REVERSE] = getReverseEntrances(entrancePools, EntranceType::MISC_CRAWLSPACE);
            }
        }

        SetShuffledEntrances(entrancePools);

        // Combine all entrance pools into one when mixing pools
        if (world.getSettings().mix_entrance_pools)
        {
            entrancePools[EntranceType::MIXED] = {};
            // For each entrance type, add the entrance to the mixed pool instead
            for (auto& [type, entrancePool] : entrancePools)
            {
                // Don't re-add the mixed pool to itself and don't mix crawlspaces
                if (type != EntranceType::MIXED && type != EntranceType::MISC_CRAWLSPACE && type != EntranceType::MISC_CRAWLSPACE_REVERSE)
                {
                    addElementsToPool(entrancePools[EntranceType::MIXED], entrancePool);
                    entrancePools[type].clear();
                }
            }
        }

        // This algorithm works similarly to the assumed fill algorithm used to
        // place items within the world. First, we disconnect all the entrances
        // that we're going to shuffle and "assume" that we have access to them
        // by creating a *target entrance* that corresponds to each disconnected
        // entrance. These target entrances are connected directly to the Root
        // of the world graph so that they have no requirements for access.

        // Create the pool of target entrances for each entrance type
        for (auto& [type, entrancePool] : entrancePools)
        {
            targetEntrancePools[type] = assumeEntrancePool(entrancePool);
            logEntrancePool(targetEntrancePools[type], "Targets for entrance type " + entranceTypeToName(type));
        }

        // Shuffle the entrances
        for (auto& [type, entrancePool] : entrancePools)
        {
            err = shuffleEntrancePool(world, worlds, entrancePool, targetEntrancePools[type]);
            if (err != EntranceShuffleError::NONE)
            {
                DebugLog::getInstance().log("| Encountered when shuffling pool of type " + entranceTypeToName(type));
                return err;
            }
        }

        // Now set the islands the race mode dungeons are in
        for (auto& [dungeonId, hintRegion] : world.raceModeDungeons)
        {
            hintRegion = *world.getIslands(dungeonIdToFirstRoom(dungeonId)).begin();
        }
    }

    // Validate the worlds one last time to ensure everything went okay
    err = validateWorld(worlds, nullptr, completeItemPool);
    ENTRANCE_SHUFFLE_ERROR_CHECK(err);

    return EntranceShuffleError::NONE;
}

const std::string errorToName(EntranceShuffleError err)
{
    switch (err)
    {
        case EntranceShuffleError::NONE:
            return "NONE";
        case EntranceShuffleError::RAN_OUT_OF_RETRIES:
            return "RAN_OUT_OF_RETRIES";
        case EntranceShuffleError::NO_MORE_VALID_ENTRANCES:
            return "NO_MORE_VALID_ENTRANCES";
        case EntranceShuffleError::ALL_LOCATIONS_NOT_REACHABLE:
            return "ALL_LOCATIONS_NOT_REACHABLE";
        case EntranceShuffleError::AMBIGUOUS_RACE_MODE_ISLAND:
            return "AMBIGUOUS_RACE_MODE_ISLAND";
        case EntranceShuffleError::AMBIGUOUS_RACE_MODE_DUNGEON:
            return "AMBIGUOUS_RACE_MODE_DUNGEON";
        case EntranceShuffleError::NOT_ENOUGH_SPHERE_ZERO_LOCATIONS:
            return "NOT_ENOUGH_SPHERE_ZERO_LOCATIONS";
        case EntranceShuffleError::ATTEMPTED_SELF_CONNECTION:
            return "ATTEMPTED_SELF_CONNECTION";
        case EntranceShuffleError::FAILED_TO_DISCONNECT_TARGET:
            return "FAILED_TO_DISCONNECT_TARGET";
        default:
            return "UNKNOWN";
    }

}
