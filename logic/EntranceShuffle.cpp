
#include "EntranceShuffle.hpp"
#include "Random.hpp"
#include "PoolFunctions.hpp"
#include "Debug.hpp"
#include "Search.hpp"
#include <map>
#include <utility>
#include <iostream>

#define ENTRANCE_SHUFFLE_ERROR_CHECK(err) if (err != EntranceShuffleError::NONE) {debugLog("Error: " + errorToName(err)); return err;}
#define GET_COMPLETE_ITEM_POOL(itemPool, worlds) for (auto& world : worlds) {addElementsToPool(itemPool, world.getItemPool());}
#define GET_COMPLETE_PROGRESSION_LOCATION_POOL(locationPool, worlds) for (auto& world : worlds) {addElementsToPool(locationPool, world.getLocations(true));}

using EntrancePools = std::map<EntranceType, EntrancePool>;
using EntrancePair = std::pair<Entrance*, Entrance*>;

static std::list<EntranceInfoPair> entranceShuffleTable = {                                                     //----File path info------|---entrance info---//
                            // Parent Area                            Connected Area                            stage,   room, scls index,  stage,  room, spawn, boss stage and out info (stage, room, spawn)
    {EntranceType::DUNGEON, {Area::DragonRoostPondPastStatues,        Area::DRCFirstRoom,                       "Adanmae",  0,          2, "M_NewD2",  0,     0},
    /*Dragon Roost Cavern*/ {Area::DRCFirstRoom,                      Area::DragonRoostPondPastStatues,         "M_NewD2",  0,          0, "Adanmae",  0,     2, "M_DragB", "sea",  13, 211}},
    {EntranceType::DUNGEON, {Area::FWEntrancePlatform,                Area::FWFirstRoom,                        "sea",     41,          6, "kindan",   0,     0},
    /*Forbidden Woods*/     {Area::FWFirstRoom,                       Area::FWEntrancePlatform,                 "kindan",   0,          0, "sea",     41,     6, "kinBOSS", "Omori", 0, 215}},
    {EntranceType::DUNGEON, {Area::TowerOfTheGods,                    Area::TOTGEntranceRoom,                   "sea",     26,          0, "Siren",    0,     0},
    /*Tower of the Gods*/   {Area::TOTGEntranceRoom,                  Area::TowerOfTheGods,                     "Siren",    0,          0, "sea",     26,     2, "SirenB",  "sea",  26,   1}},
    {EntranceType::DUNGEON, {Area::HeadstoneIslandInterior,           Area::ETFirstRoom,                        "Edaichi",  0,          0, "M_Dai",  255,     0},
    /*Earth Temple*/        {Area::ETFirstRoom,                       Area::HeadstoneIslandInterior,            "M_Dai",    0,          0, "Edaichi",  0,     1, "M_DaiB",  "sea",  45, 229}},
    {EntranceType::DUNGEON, {Area::GaleIsleInterior,                  Area::WTFirstRoom,                        "Ekaze",    0,          0, "kaze",    15,    15},
    /*Wind Temple*/         {Area::WTFirstRoom,                       Area::GaleIsleInterior,                   "kaze",    15,          0, "Ekaze",    0,     1, "kazeB",   "sea",   4, 232}},

    {EntranceType::CAVE,    {Area::OutsetNearSavageHeadstone,         Area::OutsetSavageLabyrinth,              "sea",     44,          8, "Cave09",   0,     0},
                            {Area::OutsetSavageLabyrinth,             Area::OutsetNearSavageHeadstone,          "Cave09",   0,          1, "sea",     44,    10}},
    {EntranceType::CAVE,    {Area::DragonRoostIsland,                 Area::DragonRoostIslandCave,              "sea",     13,          2, "TF_06",    0,     0},
                            {Area::DragonRoostIslandCave,             Area::DragonRoostIsland,                  "TF_06",    0,          0, "sea",     13,     5}},
    {EntranceType::CAVE,    {Area::FireMountain,                      Area::FireMountainInterior,               "sea",     20,          0, "MiniKaz",  0,     0},
                            {Area::FireMountainInterior,              Area::FireMountain,                       "MiniKaz",  0,          0, "sea",     20,     0}},
    {EntranceType::CAVE,    {Area::IceRingIsle,                       Area::IceRingInterior,                    "sea",     40,          0, "MiniHyo",  0,     0},
                            {Area::IceRingInterior,                   Area::IceRingIsle,                        "MiniHyo",  0,          0, "sea",     40,     0}},
    {EntranceType::CAVE,    {Area::TheCabana,                         Area::CabanaLabyrinth,                    "Abesso",   0,          1, "TF_04",    0,     0},
                            {Area::CabanaLabyrinth,                   Area::TheCabana,                          "TF_04",    0,          0, "Abesso",   0,     1}},
    {EntranceType::CAVE,    {Area::NeedleRockIsle,                    Area::NeedleRockCave,                     "sea",     29,          0, "SubD42",   0,     0},
                            {Area::NeedleRockCave,                    Area::NeedleRockIsle,                     "SubD42",   0,          0, "sea",     29,     5}},
    {EntranceType::CAVE,    {Area::AngularIslesSmallIsle,             Area::AngularIslesCave,                   "sea",     47,          1, "SubD43",   0,     0},
                            {Area::AngularIslesCave,                  Area::AngularIslesSmallIsle,              "SubD43",   0,          0, "sea",     47,     5}},
    {EntranceType::CAVE,    {Area::BoatingCourseSmallIsle,            Area::BoatingCourseCave,                  "sea",     48,          0, "SubD71",   0,     0},
                            {Area::BoatingCourseCave,                 Area::BoatingCourseSmallIsle,             "SubD71",   0,          0, "sea",     48,     5}},
    {EntranceType::CAVE,    {Area::StoneWatcherIsland,                Area::StoneWatcherCave,                   "sea",     31,          0, "TF_01",    0,     0},
                            {Area::StoneWatcherCave,                  Area::StoneWatcherIsland,                 "TF_01",    0,          0, "sea",     31,     1}},
    {EntranceType::CAVE,    {Area::OverlookIslandUpperIsles,          Area::OverlookCave,                       "sea",      7,          0, "TF_02",    0,     0},
                            {Area::OverlookCave,                      Area::OverlookIslandUpperIsles,           "TF_02",    0,          0, "sea",      7,     1}},
    {EntranceType::CAVE,    {Area::BirdsPeakRockBehindBars,           Area::BirdsPeakRockCave,                  "sea",     35,          0, "TF_03",    0,     0},
                            {Area::BirdsPeakRockCave,                 Area::BirdsPeakRockBehindBars,            "TF_03",    0,          0, "sea",     35,     1}},
    {EntranceType::CAVE,    {Area::PawprintIsle,                      Area::PawprintChuChuCave,                 "sea",     12,          0, "TyuTyu",   0,     0},
                            {Area::PawprintChuChuCave,                Area::PawprintIsle,                       "TyuTyu",   0,          0, "sea",     12,     1}},
    {EntranceType::CAVE,    {Area::PawprintWizzrobeCaveIsle,          Area::PawprintWizzrobeCave,               "sea",     12,          1, "Cave07",   0,     0},
                            {Area::PawprintWizzrobeCave,              Area::PawprintWizzrobeCaveIsle,           "Cave07",   0,          0, "sea",     12,     5}},
    {EntranceType::CAVE,    {Area::DiamondSteppeUpperIsland,          Area::DiamondSteppeWarpMaze,              "sea",     36,          0, "WarpD",    0,     0},
                            {Area::DiamondSteppeWarpMaze,             Area::DiamondSteppeUpperIsland,           "WarpD",    0,          0, "sea",     36,     1}},
    {EntranceType::CAVE,    {Area::BombIsland,                        Area::BombIslandCave,                     "sea",     34,          0, "Cave01",   0,     0},
                            {Area::BombIslandCave,                    Area::BombIsland,                         "Cave01",   0,          0, "sea",     34,     1}},
    {EntranceType::CAVE,    {Area::RockSpireUpperLedges,              Area::RockSpireCave,                      "sea",     16,          0, "Cave04",   0,     0},
                            {Area::RockSpireCave,                     Area::RockSpireUpperLedges,               "Cave04",   0,          0, "sea",     16,     1}},
    {EntranceType::CAVE,    {Area::SharkIsland,                       Area::SharkIslandCave,                    "sea",     38,          0, "ITest63",  0,     0},
                            {Area::SharkIslandCave,                   Area::SharkIsland,                        "ITest63",  0,          0, "sea",     38,     5}},
    {EntranceType::CAVE,    {Area::CliffPlateauIsles,                 Area::CliffPlateauCave,                   "sea",     42,          0, "Cave03",   0,     0},
                            {Area::CliffPlateauCave,                  Area::CliffPlateauIsles,                  "Cave03",   0,          0, "sea",     42,     2}},
    {EntranceType::CAVE,    {Area::CliffPlateauHighestIsle,           Area::CliffPlateauCavePastWoodenBarrier,  "sea",     42,          1, "Cave03",   0,     1},
                            {Area::CliffPlateauCavePastWoodenBarrier, Area::CliffPlateauHighestIsle,            "Cave03",   0,          1, "sea",     42,     1}},
    {EntranceType::CAVE,    {Area::HorseshoeIslePastTentacles,        Area::HorseshoeCave,                      "sea",     43,          0, "Cave05",   0,     0},
                            {Area::HorseshoeCave,                     Area::HorseshoeIslePastTentacles,         "Cave05",   0,          0, "sea",     43,     5}},
    {EntranceType::CAVE,    {Area::StarIsland,                        Area::StarIslandCave,                     "sea",      2,          0, "Cave02",   0,     0},
                            {Area::StarIslandCave,                    Area::StarIsland,                         "Cave02",   0,          0, "sea",      2,     1}},

    {EntranceType::DOOR,    {Area::WindfallIsland,                    Area::WindfallJail,                       "sea",     11,          6, "Pnezumi",  0,     0},
                            {Area::WindfallJail,                      Area::WindfallIsland,                     "Pnezumi",  0,          0, "sea",     11,    13}},
    {EntranceType::DOOR,    {Area::WindfallIsland,                    Area::WindfallSchoolOfJoy,                "sea",     11,         12, "Nitiyou",  0,     0},
                            {Area::WindfallSchoolOfJoy,               Area::WindfallIsland,                     "Nitiyou",  0,          0, "sea",     11,    12}},
    {EntranceType::DOOR,    {Area::WindfallIsland,                    Area::WindfallLenzosHouseFromBottomEntry, "sea",     11,         10, "Ocmera",   0,     0},
                            {Area::WindfallLenzosHouseLower,          Area::WindfallIsland,                     "Ocmera",   0,          0, "sea",     11,    10}},
    {EntranceType::DOOR,    {Area::WindfallLenzosUpperLedge,          Area::WindfallLenzosHouseUpper,           "sea",     11,         11, "Ocmera",   0,     1},
                            {Area::WindfallLenzosHouseUpper,          Area::WindfallLenzosUpperLedge,           "Ocmera",   0,          1, "sea",     11,    11}},
    {EntranceType::DOOR,    {Area::WindfallIsland,                    Area::WindfallCafeBar,                    "sea",     11,          7, "Opub",     0,     0},
                            {Area::WindfallCafeBar,                   Area::WindfallIsland,                     "Opub",     0,          0, "sea",     11,     6}},
    {EntranceType::DOOR,    {Area::WindfallIsland,                    Area::WindfallBattleSquidInterior,        "sea",     11,          9, "Kaisen",   0,     0},
                            {Area::WindfallBattleSquidInterior,       Area::WindfallIsland,                     "Kaisen",   0,          0, "sea",     11,     9}},
    {EntranceType::DOOR,    {Area::WindfallBattleSquidUpperLedge,     Area::WindfallBattleSquidInterior,        "sea",     11,          8, "Kaisen",   0,     1},
                            {Area::WindfallBattleSquidInterior,       Area::WindfallBattleSquidUpperLedge,      "Kaisen",   0,          1, "sea",     11,     8}},
    {EntranceType::DOOR,    {Area::WindfallIsland,                    Area::WindfallHouseOfWealthLower,         "sea",     11,          3, "Orichh",   0,     0},
                            {Area::WindfallHouseOfWealthLower,        Area::WindfallIsland,                     "Orichh",   0,          1, "sea",     11,     3}},
    {EntranceType::DOOR,    {Area::WindfallIsland,                    Area::WindfallHouseOfWealthUpper,         "sea",     11,          4, "Orichh",   0,     1},
                            {Area::WindfallHouseOfWealthUpper,        Area::WindfallIsland,                     "Orichh",   0,          2, "sea",     11,     4}},
    {EntranceType::DOOR,    {Area::WindfallIsland,                    Area::WindfallPotionShop,                 "sea",     11,          5, "Pdrgsh",   0,     0},
                            {Area::WindfallPotionShop,                Area::WindfallIsland,                     "Pdrgsh",   0,          0, "sea",     11,     7}},
    {EntranceType::DOOR,    {Area::WindfallIsland,                    Area::WindfallBombShop,                   "sea",     11,          1, "Obombh",   0,     0},
                            {Area::WindfallBombShop,                  Area::WindfallIsland,                     "Obombh",   0,          1, "sea",     11,     1}},
//  {EntranceType::DOOR,    {Area::WindfallIsland,                    Area::WindfallPirateShip,                 "",        -1,         -1, "",        -1,    -1},
//                          {Area::WindfallPirateShip,                Area::WindfallIsland,                     "",        -1,         -1, "",        -1,    -1}},
    {EntranceType::DOOR,    {Area::DragonRoostRitoAerie,              Area::DragonRoostKomalisRoom,             "Atorizk",  0,          4, "Comori",   0,     0},
                            {Area::DragonRoostKomalisRoom,            Area::DragonRoostRitoAerie,               "Comori",   0,          1, "Atorizk",  0,     4}},
    {EntranceType::DOOR,    {Area::PrivateOasis,                      Area::TheCabana,                          "sea",     33,          0, "Abesso",   0,     0},
                            {Area::TheCabana,                         Area::PrivateOasis,                       "Abesso",   0,          0, "sea",     33,     1}},
    {EntranceType::DOOR,    {Area::OutsetIsland,                      Area::OutsetLinksHouse,                   "sea",     44,          0, "LinkRM",   0,     1},
                            {Area::OutsetLinksHouse,                  Area::OutsetIsland,                       "LinkRM",   0,          0, "sea",     44,     1}},
    {EntranceType::DOOR,    {Area::OutsetIsland,                      Area::OutsetOrcasHouse,                   "sea",     44,          1, "Ojhous",   0,     0},
                            {Area::OutsetOrcasHouse,                  Area::OutsetIsland,                       "Ojhous",   0,          0, "sea",     44,     3}},
    {EntranceType::DOOR,    {Area::OutsetIsland,                      Area::OutsetSturgeonsHouse,               "sea",     44,          2, "Ojhous2",  1,     0},
                            {Area::OutsetSturgeonsHouse,              Area::OutsetIsland,                       "Ojhous2",  1,          0, "sea",     44,     2}},
    {EntranceType::DOOR,    {Area::OutsetIsland,                      Area::OutsetRosesHouse,                   "sea",     44,          4, "Onobuta",  0,     0},
                            {Area::OutsetRosesHouse,                  Area::OutsetIsland,                       "Onobuta",  0,          0, "sea",     44,     4}},
    {EntranceType::DOOR,    {Area::OutsetIsland,                      Area::OutsetMesasHouse,                   "sea",     44,          3, "Omasao",   0,     0},
                            {Area::OutsetMesasHouse,                  Area::OutsetIsland,                       "Omasao",   0,          0, "sea",     44,     7}},

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
    {EntranceType::MISC_RESTRICTIVE, {Area::GaleIsle,                          Area::GaleIsleInterior,                   "sea",      4, 0, "Ekaze",    0,     0},
                                     {Area::GaleIsleInterior,                  Area::GaleIsle,                           "Ekaze",    0, 1, "sea",      4,     1}},
    {EntranceType::MISC_CRAWLSPACE,  {Area::WindfallIsland,                    Area::WindfallBombShopUpperLedge,         "sea",     11, 2, "Obombh",   0,     1},
                                     {Area::WindfallBombShopUpperLedge,        Area::WindfallIsland,                     "Obombh",   0, 2, "sea",     11,     2}},
    {EntranceType::MISC,             {Area::DragonRoostIsland,                 Area::DragonRoostRitoAerie,               "sea",     13, 0, "Atorizk",  0,     0},
                                     {Area::DragonRoostRitoAerie,              Area::DragonRoostIsland,                  "Atorizk",  0, 0, "sea",     13,     1}},
    {EntranceType::MISC,             {Area::DragonRoostIslandFlightDeck,       Area::DragonRoostRitoAerie,               "sea",     13, 1, "Atorizk",  0,     1},
                                     {Area::DragonRoostRitoAerie,              Area::DragonRoostIslandFlightDeck,        "Atorizk",  0, 1, "sea",     13,     2}},
    {EntranceType::MISC,             {Area::DragonRoostPond,                   Area::DragonRoostRitoAerie,               "Adanmae",  0, 0, "Atorizk",  0,     2},
                                     {Area::DragonRoostRitoAerie,              Area::DragonRoostPond,                    "Atorizk",  0, 2, "Adanmae",  0,     0}},
    {EntranceType::MISC_RESTRICTIVE, {Area::DragonRoostRitoAerie,              Area::DragonRoostPondUpperLedge,          "Atorizk",  0, 3, "Adanmae",  0,     1},
                                     {Area::DragonRoostPondUpperLedge,         Area::DragonRoostRitoAerie,               "Adanmae",  0, 1, "Atorizk",  0,     3}},
    {EntranceType::MISC_RESTRICTIVE, {Area::IsletOfSteel,                      Area::IsletOfSteelInterior,               "sea",     30, 0, "ShipD",    0,     0},
                                     {Area::IsletOfSteelInterior,              Area::IsletOfSteel,                       "Ship",     0, 0, "sea",     30,     1}},
    {EntranceType::MISC,             {Area::ForestHaven,                       Area::ForestHavenInterior,                "sea",     41, 5, "Omori",    0,     5},
                                     {Area::ForestHavenInterior,               Area::ForestHaven,                        "Omori",    0, 5, "sea",     41,     5}},
    {EntranceType::MISC_RESTRICTIVE, {Area::ForestHaven,                       Area::ForestHavenWaterfallCave,           "sea",     41, 7, "Otkura",   0,     0},
                                     {Area::ForestHavenWaterfallCave,          Area::ForestHaven,                        "Otkura",   0, 0, "sea",     41,     9}},
    {EntranceType::MISC_RESTRICTIVE, {Area::ForestHavenInterior,               Area::ForestPotionShop,                   "Omori",    0, 0, "Ocrogh",   0,     0},
                                     {Area::ForestPotionShop,                  Area::ForestHavenInterior,                "Ocrogh",   0, 0, "Omori",    0,     0}},
    {EntranceType::MISC,             {Area::ForestHavenExteriorNorthLedge,     Area::ForestHavenInteriorNorthLedge,      "sea",     41, 1, "Omori",    0,     1},
                                     {Area::ForestHavenInteriorNorthLedge,     Area::ForestHavenExteriorNorthLedge,      "Omori",    0, 1, "sea",     41,     1}},
    {EntranceType::MISC,             {Area::ForestHavenExteriorWestLowerLedge, Area::ForestHavenInteriorWestLowerLedge,  "sea",     41, 3, "Omori",    0,     3},
                                     {Area::ForestHavenInteriorWestLowerLedge, Area::ForestHavenExteriorWestLowerLedge,  "Omori",    0, 3, "sea",     41,     3}},
    {EntranceType::MISC,             {Area::ForestHavenExteriorWestUpperLedge, Area::ForestHavenInteriorWestUpperLedge,  "sea",     41, 2, "Omori",    0,     2},
                                     {Area::ForestHavenInteriorWestUpperLedge, Area::ForestHavenExteriorWestUpperLedge,  "Omori",    0, 2, "sea",     41,     2}},
    {EntranceType::MISC,             {Area::ForestHavenExteriorSouthLedge,     Area::ForestHavenInteriorSouthLedge,      "sea",     41, 4, "Omori",    0,     4},
                                     {Area::ForestHavenInteriorSouthLedge,     Area::ForestHavenExteriorSouthLedge,      "Omori",    0, 4, "sea",     41,     4}},
    {EntranceType::MISC_CRAWLSPACE,  {Area::OutsetIsland,                      Area::OutsetUnderLinksHouse,              "sea",     44, 9, "LinkUG",   0,     1},
                                     {Area::OutsetUnderLinksHouse,             Area::OutsetIsland,                       "LinkUG",   0, 0, "sea",     44,    11}},
    {EntranceType::MISC_RESTRICTIVE, {Area::OutsetAcrossBridge,                Area::OutsetForestOfFairies,              "sea",     44, 6, "A_mori",   0,     0},
                                     {Area::OutsetForestOfFairies,             Area::OutsetAcrossBridge,                 "A_mori",   0, 0, "sea",     44,     8}},
    {EntranceType::MISC_RESTRICTIVE, {Area::OutsetIsland,                      Area::OutsetJabunsCave,                   "sea",     44, 7, "Pjavdou",  0,     0},
                                     {Area::OutsetJabunsCave,                  Area::OutsetIsland,                       "Pjavdou",  0, 0, "sea",     44,     9}},
    {EntranceType::MISC_RESTRICTIVE, {Area::HeadstoneIsland,                   Area::HeadstoneIslandInterior,            "sea",     45, 0, "Edaichi",  0,     0},
                                     {Area::HeadstoneIslandInterior,           Area::HeadstoneIsland,                    "Edaichi",  0, 1, "sea",     45,     1}},
};

static void logEntrancePool(EntrancePool& entrancePool, const std::string& poolName)
{
    debugLog(poolName + ":");
    for (auto entrance : entrancePool)
    {
        debugLog("\t" + entrance->getOriginalName());
    }
}

static void logMissingLocations(WorldPool& worlds)
{
    static int identifier = 0;
    debugLog("Missing Locations:");
    for (auto& world : worlds)
    {
        for (auto& location : world.locationEntries)
        {
            if (!location.hasBeenFound && location.locationId != LocationId::INVALID)
            {
                debugLog("\t" + locationIdToName(location.locationId));
                if (identifier++ < 10)
                {
                    world.dumpWorldGraph(std::to_string(identifier));
                    debugLog("Now Dumping " + std::to_string(identifier));
                }
                break;
            }
        }
    }
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
        if (returnEntry.parentArea != Area::INVALID)
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
    if (targetEntrance->getConnectedArea() != Area::INVALID)
    {
        targetEntrance->disconnect();
    }
    if (targetEntrance->getParentArea() != Area::INVALID)
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
    debugLog("Validating World");
    // Ensure that all item locations are still reachable within the given world
    if (!allLocationsReachable(worlds, itemPool))
    {
        debugLog("Error: All locations not reachable");
        // logMissingLocations(worlds);
        return EntranceShuffleError::ALL_LOCATIONS_NOT_REACHABLE;
    }
    // Ensure that there's at least one sphere zero location available to place an item
    // for the beginning of the seed
    ItemPool noItems = {};
    LocationPool progLocations = {};
    GET_COMPLETE_PROGRESSION_LOCATION_POOL(progLocations, worlds);
    if (getAccessibleLocations(worlds, noItems, progLocations).size() < worlds.size())
    {
        debugLog("Error: Not enough sphere zero locations to place items");
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

                if (world.raceModeDungeons.contains(dungeon))
                {
                    if (dungeonIslands.size() > 1)
                    {
                        debugLog("Error: More than 1 island leading to race mode dungeon " + dungeonIdToName(dungeon));
                        return EntranceShuffleError::AMBIGUOUS_RACE_MODE_ISLAND;
                    }
                }

                if (dungeonIslands.size() == 1)
                {
                    auto dungeonIsland = *dungeonIslands.begin();
                    if (raceModeIslands.contains(dungeonIsland))
                    {
                        debugLog("Error: Island " + hintRegionToName(dungeonIsland) + " has an ambiguous race mode dungeon");
                        return EntranceShuffleError::AMBIGUOUS_RACE_MODE_DUNGEON;
                    }

                    if (world.raceModeDungeons.contains(dungeon))
                    {
                        raceModeIslands.insert(dungeonIsland);
                    }
                }
            }
        }
    }
    return EntranceShuffleError::NONE;
}

static EntranceShuffleError replaceEntrance(WorldPool& worlds, Entrance* entrance, Entrance* target, std::vector<EntrancePair>& rollbacks, ItemPool& itemPool)
{
    debugLog("Attempting to Connect " + entrance->getOriginalName() + " To " + target->getReplaces()->getOriginalName());
    EntranceShuffleError err = EntranceShuffleError::NONE;
    err = checkEntrancesCompatibility(entrance, target, rollbacks);
    ENTRANCE_SHUFFLE_ERROR_CHECK(err);
    changeConnections(entrance, target);
    err = validateWorld(worlds, entrance, itemPool);
    // If the attempted replacement produces an invalid world graph, then undo
    // the attempted connection and try again with a different target.
    if (err != EntranceShuffleError::NONE)
    {
        if (entrance->getConnectedArea() != Area::INVALID)
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

    // Place all entrances in the pool, validating worlds after each placement
    for (auto entrance : entrances)
    {
        EntranceShuffleError err;
        if (entrance->getConnectedArea() != Area::INVALID)
        {
            continue;
        }
        shufflePool(targetEntrances);

        for (auto target : targetEntrances)
        {
            // If the target has already been disconnected, then don't use it again
            if (target->getConnectedArea() == Area::INVALID)
            {
                continue;
            }
            err = replaceEntrance(worlds, entrance, target, rollbacks, completeItemPool);
            if (err == EntranceShuffleError::NONE)
            {
                break;
            }
        }

        if (entrance->getConnectedArea() == Area::INVALID)
        {
            debugLog("Could not connect " + entrance->getOriginalName() + ". Error: " + errorToName(err));
            return EntranceShuffleError::NO_MORE_VALID_ENTRANCES;
        }
    }

    return EntranceShuffleError::NONE;
}

static EntranceShuffleError shuffleEntrancePool(World& world, WorldPool& worlds, EntrancePool& entrancePool, EntrancePool& targetEntrances, int retryCount = 20)
{
    EntranceShuffleError err;

    while (retryCount > 0)
    {
        retryCount--;
        std::vector<EntrancePair> rollbacks = {};

        err = shuffleEntrances(worlds, entrancePool, targetEntrances, rollbacks);
        if (err != EntranceShuffleError::NONE)
        {
            debugLog("Failed to place all entrances in a pool for world " + std::to_string(world.getWorldId() + 1) + ". Will retry " + std::to_string(retryCount) + " more times.");
            debugLog("Last Error: " + errorToName(err));
            for (auto& [entrance, target] : rollbacks)
            {
                restoreConnections(entrance, target);
            }
            continue;
        }

        // Validate world

        for (auto& [entrance, target] : rollbacks)
        {
            confirmReplacement(entrance, target);
        }
        return EntranceShuffleError::NONE;
    }

    debugLog("Entrance placement attempt count exceeded for world " + std::to_string(world.getWorldId() + 1));
    return EntranceShuffleError::RAN_OUT_OF_RETRIES;
}

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

static Area roomIndexToStartingIslandArea(const uint8_t& startingIslandRoomIndex)
{
    // Island room number corresponds with index in the below array
    constexpr std::array<Area, 50> startingIslandAreaArray = {
        Area::INVALID,
        Area::ForsakenFortress,
        Area::StarIsland,
        Area::NorthernFairyIsland,
        Area::GaleIsle,
        Area::CrescentMoonIsland,
        Area::SevenStarIsles,
        Area::OverlookIsland,
        Area::FourEyeReef,
        Area::MotherAndChildIsles,
        Area::SpectacleIsland,
        Area::WindfallIsland,
        Area::PawprintIsle,
        Area::DragonRoostIsland,
        Area::FlightControlPlatform,
        Area::WesternFairyIsland,
        Area::RockSpireIsle,
        Area::TingleIsland,
        Area::NorthernTriangleIsland,
        Area::EasternFairyIsland,
        Area::FireMountain,
        Area::StarBeltArchipelago,
        Area::ThreeEyeReef,
        Area::GreatfishIsle,
        Area::CyclopsReef,
        Area::SixEyeReef,
        Area::TowerOfTheGods,
        Area::EasternTriangleIsland,
        Area::ThornedFairyIsland,
        Area::NeedleRockIsle,
        Area::IsletOfSteel,
        Area::StoneWatcherIsland,
        Area::SouthernTriangleIsland,
        Area::PrivateOasis,
        Area::BombIsland,
        Area::BirdsPeakRock,
        Area::DiamondSteppeIsland,
        Area::FiveEyeReef,
        Area::SharkIsland,
        Area::SouthernFairyIsland,
        Area::IceRingIsle,
        Area::ForestHaven,
        Area::CliffPlateauIsles,
        Area::HorseshoeIsle,
        Area::OutsetIsland,
        Area::HeadstoneIsland,
        Area::TwoEyeReef,
        Area::AngularIsles,
        Area::BoatingCourse,
        Area::FiveStarIsles,
    };

    return startingIslandAreaArray[startingIslandRoomIndex];
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
            auto startingIsland = roomIndexToStartingIslandArea(world.startingIslandRoomIndex);

            // Set the new starting island in the world graph
            auto& linksSpawn = world.getEntrance(Area::LinksSpawn, Area::OutsetIsland);
            linksSpawn.setConnectedArea(startingIsland);
            world.areaEntries[areaAsIndex(Area::OutsetIsland)].entrances.remove(&linksSpawn);
            world.areaEntries[areaAsIndex(startingIsland)].entrances.push_back(&linksSpawn);
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
                filterFromPool(entrancePools[EntranceType::CAVE], [](Entrance* e){return e->getParentArea() == Area::CliffPlateauHighestIsle;});
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
            auto miscRestrictiveEntrances = world.getShuffleableEntrances(EntranceType::MISC_RESTRICTIVE, true);
            addElementsToPool(entrancePools[EntranceType::MISC], miscRestrictiveEntrances);

            // Keep crawlspaces separate for the time-being
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

        // Create target pool to correspond with each pool
        for (auto& [type, entrancePool] : entrancePools)
        {
            targetEntrancePools[type] = assumeEntrancePool(entrancePool);
        }

        // Shuffle the entrances
        for (auto& [type, entrancePool] : entrancePools)
        {
            err = shuffleEntrancePool(world, worlds, entrancePool, targetEntrancePools[type]);
            if (err != EntranceShuffleError::NONE)
            {
                debugLog("| Encountered when shuffling pool of type " + entranceTypeToName(type));
                return err;
            }
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
        default:
            return "UNKNOWN";
    }

}
