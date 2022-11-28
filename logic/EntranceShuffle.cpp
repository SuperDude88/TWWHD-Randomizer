
#include "EntranceShuffle.hpp"

#include <map>
#include <utility>

#include <logic/PoolFunctions.hpp>
#include <logic/Search.hpp>
#include <logic/Fill.hpp>
#include <seedgen/random.hpp>
#include <command/Log.hpp>

#define ENTRANCE_SHUFFLE_ERROR_CHECK(err) if (err != EntranceShuffleError::NONE) {LOG_TO_DEBUG("Error: " + errorToName(err)); return err;}
#define CHECK_MIXED_POOL(name, type) if (settings.name) { poolsToMix.insert(type); if (settings.decouple_entrances) { poolsToMix.insert(type##_REVERSE); } }

using EntrancePools = std::map<EntranceType, EntrancePool>;
using EntrancePair = std::pair<Entrance*, Entrance*>;

// The entrance randomization algorithm used here is heavily inspired by the entrance
// randomization algorithm from the Ocarina of Time Randomizer. While the algorithm
// has been adapted to work for The Wind Waker, credit also goes to those who developed
// it for Ocarina of Time.
//
// https://github.com/TestRunnerSRL/OoT-Randomizer/blob/Dev/EntranceShuffle.py

static std::list<EntranceInfoPair> entranceShuffleTable = {                                                       //-----File path info-----|----entrance info----|------boss stage info------//
                            // Parent Area                              Connected Area                             stage,   room, scls index,  stage,  room, spawn, boss stage, stage, room, spawn
    {EntranceType::DUNGEON, {"Dragon Roost Pond Past Statues",         "DRC First Room",                           "Adanmae",  0,          2, "M_NewD2",  0,     0},
    /*Dragon Roost Cavern*/ {"DRC First Room",                         "Dragon Roost Pond Past Statues",           "M_NewD2",  0,          0, "Adanmae",  0,     2, "M_DragB", "sea",  13, 211}},
    {EntranceType::DUNGEON, {"FW Entrance Platform",                   "FW First Room",                            "sea",     41,          6, "kindan",   0,     0},
    /*Forbidden Woods*/     {"FW First Room",                          "FW Entrance Platform",                     "kindan",   0,          0, "sea",     41,     6, "kinBOSS", "sea",  41,   0}},
    {EntranceType::DUNGEON, {"Tower of the Gods Sector",               "TOTG Entrance Room",                       "sea",     26,          0, "Siren",    0,     0},
    /*Tower of the Gods*/   {"TOTG Entrance Room",                     "Tower of the Gods Sector",                 "Siren",    0,          1, "sea",     26,     2, "SirenB",  "sea",  26,   1}},
    {EntranceType::DUNGEON, {"Headstone Island Interior",              "ET First Room",                            "Edaichi",  0,          0, "M_Dai",  255,     0},
    /*Earth Temple*/        {"ET First Room",                          "Headstone Island Interior",                "M_Dai",    0,          0, "Edaichi",  0,     1, "M_DaiB",  "sea",  45, 229}},
    {EntranceType::DUNGEON, {"Gale Isle Interior",                     "WT First Room",                            "Ekaze",    0,          0, "kaze",    15,    15},
    /*Wind Temple*/         {"WT First Room",                          "Gale Isle Interior",                       "kaze",    15,          0, "Ekaze",    0,     1, "kazeB",   "sea",   4, 232}},

    {EntranceType::CAVE,    {"Outset Near Savage Headstone",           "Outset Savage Labyrinth",                  "sea",     44,          8, "Cave09",   0,     0},
                            {"Outset Savage Labyrinth",                "Outset Near Savage Headstone",             "Cave09",   0,          1, "sea",     44,    10}},
    {EntranceType::CAVE,    {"Dragon Roost Island",                    "Dragon Roost Island Cave",                 "sea",     13,          2, "TF_06",    0,     0},
                            {"Dragon Roost Island Cave",               "Dragon Roost Island",                      "TF_06",    0,          0, "sea",     13,     5}},
    {EntranceType::CAVE,    {"Fire Mountain",                          "Fire Mountain Interior",                   "sea",     20,          0, "MiniKaz",  0,     0},
                            {"Fire Mountain Interior",                 "Fire Mountain",                            "MiniKaz",  0,          0, "sea",     20,     0}},
    {EntranceType::CAVE,    {"Ice Ring Isle",                          "Ice Ring Interior",                        "sea",     40,          0, "MiniHyo",  0,     0},
                            {"Ice Ring Interior",                      "Ice Ring Isle",                            "MiniHyo",  0,          0, "sea",     40,     0}},
    {EntranceType::CAVE,    {"The Cabana",                             "Cabana Labyrinth",                         "Abesso",   0,          1, "TF_04",    0,     0},
                            {"Cabana Labyrinth",                       "The Cabana",                               "TF_04",    0,          0, "Abesso",   0,     1}},
    {EntranceType::CAVE,    {"Needle Rock Isle",                       "Needle Rock Cave",                         "sea",     29,          0, "SubD42",   0,     0},
                            {"Needle Rock Cave",                       "Needle Rock Isle",                         "SubD42",   0,          0, "sea",     29,     5}},
    {EntranceType::CAVE,    {"Angular Isles Small Isle",               "Angular Isles Cave",                       "sea",     47,          1, "SubD43",   0,     0},
                            {"Angular Isles Cave",                     "Angular Isles Small Isle",                 "SubD43",   0,          0, "sea",     47,     5}},
    {EntranceType::CAVE,    {"Boating Course Small Isle",              "Boating Course Cave",                      "sea",     48,          0, "SubD71",   0,     0},
                            {"Boating Course Cave",                    "Boating Course Small Isle",                "SubD71",   0,          0, "sea",     48,     5}},
    {EntranceType::CAVE,    {"Stone Watcher Island",                   "Stone Watcher Cave",                       "sea",     31,          0, "TF_01",    0,     0},
                            {"Stone Watcher Cave",                     "Stone Watcher Island",                     "TF_01",    0,          0, "sea",     31,     1}},
    {EntranceType::CAVE,    {"Overlook Island Upper Isles",            "Overlook Cave",                            "sea",      7,          0, "TF_02",    0,     0},
                            {"Overlook Cave",                          "Overlook Island Upper Isles",              "TF_02",    0,          0, "sea",      7,     1}},
    {EntranceType::CAVE,    {"Birds Peak Rock Behind Bars",            "Birds Peak Rock Cave",                     "sea",     35,          0, "TF_03",    0,     0},
                            {"Birds Peak Rock Cave",                   "Birds Peak Rock Behind Bars",              "TF_03",    0,          0, "sea",     35,     1}},
    {EntranceType::CAVE,    {"Pawprint Isle",                          "Pawprint Chu Chu Cave",                    "sea",     12,          0, "TyuTyu",   0,     0},
                            {"Pawprint Chu Chu Cave",                  "Pawprint Isle",                            "TyuTyu",   0,          0, "sea",     12,     1}},
    {EntranceType::CAVE,    {"Pawprint Wizzobe Cave Isle",             "Pawprint Wizzobe Cave",                    "sea",     12,          1, "Cave07",   0,     0},
                            {"Pawprint Wizzobe Cave",                  "Pawprint Wizzobe Cave Isle",               "Cave07",   0,          0, "sea",     12,     5}},
    {EntranceType::CAVE,    {"Diamond Steppe Upper Island",            "Diamond Steppe Warp Maze",                 "sea",     36,          0, "WarpD",    0,     0},
                            {"Diamond Steppe Warp Maze",               "Diamond Steppe Upper Island",              "WarpD",    0,          0, "sea",     36,     1}},
    {EntranceType::CAVE,    {"Bomb Island",                            "Bomb Island Cave",                         "sea",     34,          0, "Cave01",   0,     0},
                            {"Bomb Island Cave",                       "Bomb Island",                              "Cave01",   0,          0, "sea",     34,     1}},
    {EntranceType::CAVE,    {"Rock Spire Upper Ledges",                "Rock Spire Cave",                          "sea",     16,          0, "Cave04",   0,     0},
                            {"Rock Spire Cave",                        "Rock Spire Upper Ledges",                  "Cave04",   0,          0, "sea",     16,     1}},
    {EntranceType::CAVE,    {"Shark Island",                           "Shark Island Cave",                        "sea",     38,          0, "ITest63",  0,     0},
                            {"Shark Island Cave",                      "Shark Island",                             "ITest63",  0,          0, "sea",     38,     5}},
    {EntranceType::CAVE,    {"Cliff Plateau Isles",                    "Cliff Plateau Cave",                       "sea",     42,          0, "Cave03",   0,     0},
                            {"Cliff Plateau Cave",                     "Cliff Plateau Isles",                      "Cave03",   0,          0, "sea",     42,     2}},
    {EntranceType::CAVE,    {"Cliff Plateau Highest Isle",             "Cliff Plateau Cave Past Wooden Barrier",   "sea",     42,          1, "Cave03",   0,     1},
                            {"Cliff Plateau Cave Past Wooden Barrier", "Cliff Plateau Highest Isle",               "Cave03",   0,          1, "sea",     42,     1}},
    {EntranceType::CAVE,    {"Horseshoe Island Past Tentacles",        "Horseshoe Cave",                           "sea",     43,          0, "Cave05",   0,     0},
                            {"Horseshoe Cave",                         "Horseshoe Island Past Tentacles",          "Cave05",   0,          0, "sea",     43,     5}},
    {EntranceType::CAVE,    {"Star Island",                            "Star Island Cave",                         "sea",      2,          0, "Cave02",   0,     0},
                            {"Star Island Cave",                       "Star Island",                              "Cave02",   0,          0, "sea",      2,     1}},

    {EntranceType::DOOR,    {"Windfall Island",                        "Windfall Jail",                            "sea",     11,          6, "Pnezumi",  0,     0},
                            {"Windfall Jail",                          "Windfall Island",                          "Pnezumi",  0,          0, "sea",     11,    13}},
    {EntranceType::DOOR,    {"Windfall Island",                        "Windfall School of Joy",                   "sea",     11,         12, "Nitiyou",  0,     0},
                            {"Windfall School of Joy",                 "Windfall Island",                          "Nitiyou",  0,          0, "sea",     11,    12}},
    {EntranceType::DOOR,    {"Windfall Island",                        "Windfall Lenzo's House from Bottom Entry", "sea",     11,         10, "Ocmera",   0,     0},
                            {"Windfall Lenzo's House Lower",           "Windfall Island",                          "Ocmera",   0,          0, "sea",     11,    10}},
    {EntranceType::DOOR,    {"Windfall Lenzo's House Upper Ledge",     "Windfall Lenzo's House Upper",             "sea",     11,         11, "Ocmera",   0,     1},
                            {"Windfall Lenzo's House Upper",           "Windfall Lenzo's House Upper Ledge",       "Ocmera",   0,          1, "sea",     11,    11}},
    {EntranceType::DOOR,    {"Windfall Island",                        "Windfall Cafe Bar",                        "sea",     11,          7, "Opub",     0,     0},
                            {"Windfall Cafe Bar",                      "Windfall Island",                          "Opub",     0,          0, "sea",     11,     6}},
    {EntranceType::DOOR,    {"Windfall Island",                        "Windfall Battle Squid Interior",           "sea",     11,          9, "Kaisen",   0,     0},
                            {"Windfall Battle Squid Interior",         "Windfall Island",                          "Kaisen",   0,          0, "sea",     11,     9}},
    {EntranceType::DOOR,    {"Windfall Battle Squid Upper Ledge",      "Windfall Battle Squid Interior",           "sea",     11,          8, "Kaisen",   0,     1},
                            {"Windfall Battle Squid Interior",         "Windfall Battle Squid Upper Ledge",        "Kaisen",   0,          1, "sea",     11,     8}},
    {EntranceType::DOOR,    {"Windfall Island",                        "Windfall House of Wealth Lower",           "sea",     11,          3, "Orichh",   0,     0},
                            {"Windfall House of Wealth Lower",         "Windfall Island",                          "Orichh",   0,          1, "sea",     11,     3}},
    {EntranceType::DOOR,    {"Windfall Island",                        "Windfall House of Wealth Upper",           "sea",     11,          4, "Orichh",   0,     1},
                            {"Windfall House of Wealth Upper",         "Windfall Island",                          "Orichh",   0,          2, "sea",     11,     4}},
    {EntranceType::DOOR,    {"Windfall Island",                        "Windfall Potion Shop",                     "sea",     11,          5, "Pdrgsh",   0,     0},
                            {"Windfall Potion Shop",                   "Windfall Island",                          "Pdrgsh",   0,          0, "sea",     11,     7}},
    {EntranceType::DOOR,    {"Windfall Island",                        "Windfall Bomb Shop",                       "sea",     11,          1, "Obombh",   0,     0},
                            {"Windfall Bomb Shop",                     "Windfall Island",                          "Obombh",   0,          1, "sea",     11,     1}},
//  {EntranceType::DOOR,    {"Windfall Island",                        "Windfall Pirate Ship",                     "",        -1,         -1, "",        -1,    -1},
//                          {"Windfall Pirate Ship",                   "Windfall Island",                          "",        -1,         -1, "",        -1,    -1}},
    {EntranceType::DOOR,    {"Dragon Roost Rito Aerie",                "Dragon Roost Komali's Room",               "Atorizk",  0,          4, "Comori",   0,     0},
                            {"Dragon Roost Komali's Room",             "Dragon Roost Rito Aerie",                  "Comori",   0,          0, "Atorizk",  0,     4}},
    {EntranceType::DOOR,    {"Private Oasis",                          "The Cabana",                               "sea",     33,          0, "Abesso",   0,     0},
                            {"The Cabana",                             "Private Oasis",                            "Abesso",   0,          0, "sea",     33,     1}},
    {EntranceType::DOOR,    {"Outset Island",                          "Outset Link's House",                      "sea",     44,          0, "LinkRM",   0,     1},
                            {"Outset Link's House",                    "Outset Island",                            "LinkRM",   0,          0, "sea",     44,     1}},
    {EntranceType::DOOR,    {"Outset Island",                          "Outset Orca's House",                      "sea",     44,          1, "Ojhous",   0,     0},
                            {"Outset Orca's House",                    "Outset Island",                            "Ojhous",   0,          0, "sea",     44,     3}},
    {EntranceType::DOOR,    {"Outset Island",                          "Outset Sturgeon's House",                  "sea",     44,          2, "Ojhous2",  1,     0},
                            {"Outset Sturgeon's House",                "Outset Island",                            "Ojhous2",  1,          0, "sea",     44,     2}},
    {EntranceType::DOOR,    {"Outset Island",                          "Outset Rose's House",                      "sea",     44,          4, "Onobuta",  0,     0},
                            {"Outset Rose's House",                    "Outset Island",                            "Onobuta",  0,          0, "sea",     44,     4}},
    {EntranceType::DOOR,    {"Outset Island",                          "Outset Mesa's House",                      "sea",     44,          3, "Omasao",   0,     0},
                            {"Outset Mesa's House",                    "Outset Island",                            "Omasao",   0,          0, "sea",     44,     7}},

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
    {EntranceType::MISC_RESTRICTIVE, {"Gale Isle",                              "Gale Isle Interior",                      "sea",      4, 0, "Ekaze",    0,     0},
                                     {"Gale Isle Interior",                     "Gale Isle",                               "Ekaze",    0, 1, "sea",      4,     1}},
    {EntranceType::MISC_CRAWLSPACE,  {"Windfall Island",                        "Windfall Bomb Shop Upper Ledge",          "sea",     11, 2, "Obombh",   0,     1},
                                     {"Windfall Bomb Shop Upper Ledge",         "Windfall Island",                         "Obombh",   0, 2, "sea",     11,     2}},
    {EntranceType::MISC,             {"Dragon Roost Island",                    "Dragon Roost Rito Aerie",                 "sea",     13, 0, "Atorizk",  0,     0},
                                     {"Dragon Roost Rito Aerie",                "Dragon Roost Island",                     "Atorizk",  0, 0, "sea",     13,     1}},
    {EntranceType::MISC,             {"Dragon Roost Island Flight Deck",        "Dragon Roost Rito Aerie",                 "sea",     13, 1, "Atorizk",  0,     1},
                                     {"Dragon Roost Rito Aerie",                "Dragon Roost Island Flight Deck",         "Atorizk",  0, 1, "sea",     13,     2}},
    {EntranceType::MISC,             {"Dragon Roost Pond",                      "Dragon Roost Rito Aerie",                 "Adanmae",  0, 0, "Atorizk",  0,     2},
                                     {"Dragon Roost Rito Aerie",                "Dragon Roost Pond",                       "Atorizk",  0, 2, "Adanmae",  0,     0}},
    {EntranceType::MISC_RESTRICTIVE, {"Dragon Roost Rito Aerie",                "Dragon Roost Pond Upper Ledge",           "Atorizk",  0, 3, "Adanmae",  0,     1},
                                     {"Dragon Roost Pond Upper Ledge",          "Dragon Roost Rito Aerie",                 "Adanmae",  0, 1, "Atorizk",  0,     3}},
    {EntranceType::MISC_RESTRICTIVE, {"Islet of Steel",                         "Islet of Steel Interior",                 "sea",     30, 0, "ShipD",    0,     0},
                                     {"Islet of Steel Interior",                "Islet of Steel",                          "ShipD",    0, 0, "sea",     30,     1}},
    {EntranceType::MISC,             {"Forest Haven",                           "Forest Haven Interior",                   "sea",     41, 5, "Omori",    0,     5},
                                     {"Forest Haven Interior",                  "Forest Haven",                            "Omori",    0, 5, "sea",     41,     5}},
    {EntranceType::MISC_RESTRICTIVE, {"Forest Haven",                           "Forest Haven Waterfall Cave",             "sea",     41, 7, "Otkura",   0,     0},
                                     {"Forest Haven Waterfall Cave",            "Forest Haven",                            "Otkura",   0, 0, "sea",     41,     9}},
    {EntranceType::MISC_RESTRICTIVE, {"Forest Haven Interior",                  "Forest Potion Shop",                      "Omori",    0, 0, "Ocrogh",   0,     0},
                                     {"Forest Potion Shop",                     "Forest Haven Interior",                   "Ocrogh",   0, 0, "Omori",    0,     0}},
    {EntranceType::MISC,             {"Forest Haven Exterior North Ledge",      "Forest Haven Interior North Ledge",       "sea",     41, 1, "Omori",    0,     1},
                                     {"Forest Haven Interior North Ledge",      "Forest Haven Exterior North Ledge",       "Omori",    0, 1, "sea",     41,     1}},
    {EntranceType::MISC,             {"Forest Haven Exterior West Lower Ledge", "Forest Haven Interior West Lower Ledge",  "sea",     41, 3, "Omori",    0,     3},
                                     {"Forest Haven Interior West Lower Ledge", "Forest Haven Exterior West Lower Ledge",  "Omori",    0, 3, "sea",     41,     3}},
    {EntranceType::MISC,             {"Forest Haven Exterior West Upper Ledge", "Forest Haven Interior West Upper Ledge",  "sea",     41, 2, "Omori",    0,     2},
                                     {"Forest Haven Interior West Upper Ledge", "Forest Haven Exterior West Upper Ledge",  "Omori",    0, 2, "sea",     41,     2}},
    {EntranceType::MISC,             {"Forest Haven Exterior South Ledge",      "Forest Haven Interior South Ledge",       "sea",     41, 4, "Omori",    0,     4},
                                     {"Forest Haven Interior South Ledge",      "Forest Haven Exterior South Ledge",       "Omori",    0, 4, "sea",     41,     4}},
    {EntranceType::MISC_CRAWLSPACE,  {"Outset Island",                          "Outset Under Link's House",               "sea",     44, 9, "LinkUG",   0,     1},
                                     {"Outset Under Link's House",              "Outset Island",                           "LinkUG",   0, 0, "sea",     44,    11}},
    {EntranceType::MISC_RESTRICTIVE, {"Outset Across Bridge",                   "Outset Forest of Fairies",                "sea",     44, 6, "A_mori",   0,     0},
                                     {"Outset Forest of Fairies",               "Outset Across Bridge",                    "A_mori",   0, 0, "sea",     44,     8}},
    {EntranceType::MISC_RESTRICTIVE, {"Outset Island",                          "Outset Jabun's Cave",                     "sea",     44, 7, "Pjavdou",  0,     0},
                                     {"Outset Jabun's Cave",                    "Outset Island",                           "Pjavdou",  0, 0, "sea",     44,     9}},
    {EntranceType::MISC_RESTRICTIVE, {"Headstone Island",                       "Headstone Island Interior",               "sea",     45, 0, "Edaichi",  0,     0},
                                     {"Headstone Island Interior",              "Headstone Island",                        "Edaichi",  0, 1, "sea",     45,     1}},
};

#ifdef ENABLE_DEBUG
static void logEntrancePool(EntrancePool& entrancePool, const std::string& poolName)
{
    LOG_TO_DEBUG(poolName + ": [");
    for (auto entrance : entrancePool)
    {
        LOG_TO_DEBUG("\t" + entrance->getOriginalName());
    }
    LOG_TO_DEBUG("]");
}

static void logMissingLocations(WorldPool& worlds)
{
    static int identifier = 0;
    LOG_TO_DEBUG("Missing Locations: [");
    for (auto& world : worlds)
    {
        for (auto& [name, location] : world.locationEntries)
        {
            if (!location.hasBeenFound)
            {
                LOG_TO_DEBUG("\t" + location.getName());
                if (identifier++ < 10)
                {
                    // world.dumpWorldGraph(std::to_string(identifier));
                    // LOG_TO_DEBUG("Now Dumping " + std::to_string(identifier));
                }
                break;
            }
        }
    }
    LOG_TO_DEBUG("]");
}
#endif

static EntranceShuffleError setAllEntrancesData(World& world)
{
    for (auto& [type, forwardEntry, returnEntry] : entranceShuffleTable)
    {
        auto forwardEntrance = world.getEntrance(forwardEntry.parentArea, forwardEntry.connectedArea);
        if (forwardEntrance == nullptr)
        {
            return EntranceShuffleError::BAD_ENTRANCE_SHUFFLE_TABLE_ENTRY;
        }
        forwardEntrance->setFilepathStage(forwardEntry.filepathStage);
        forwardEntrance->setFilepathRoomNum(forwardEntry.filepathRoom);
        forwardEntrance->setSclsExitIndex(forwardEntry.sclsExitIndex);
        forwardEntrance->setStageName(forwardEntry.stage);
        forwardEntrance->setRoomNum(forwardEntry.room);
        forwardEntrance->setSpawnId(forwardEntry.spawnId);
        forwardEntrance->setBossFilepathStageName(forwardEntry.bossFilepathStage);
        forwardEntrance->setBossOutStageName(forwardEntry.bossOutStage);
        forwardEntrance->setBossOutRoomNum(forwardEntry.bossOutRoom);
        forwardEntrance->setBossOutSpawnId(forwardEntry.bossOutSpawnId);
        forwardEntrance->setEntranceType(type);
        forwardEntrance->setAsPrimary();
        if (returnEntry.parentArea != "")
        {
            auto returnEntrance = world.getEntrance(returnEntry.parentArea, returnEntry.connectedArea);
            if (returnEntrance == nullptr)
            {
                return EntranceShuffleError::BAD_ENTRANCE_SHUFFLE_TABLE_ENTRY;
            }
            returnEntrance->setFilepathStage(returnEntry.filepathStage);
            returnEntrance->setFilepathRoomNum(returnEntry.filepathRoom);
            returnEntrance->setSclsExitIndex(returnEntry.sclsExitIndex);
            returnEntrance->setStageName(returnEntry.stage);
            returnEntrance->setRoomNum(returnEntry.room);
            returnEntrance->setSpawnId(returnEntry.spawnId);
            returnEntrance->setBossFilepathStageName(returnEntry.bossFilepathStage);
            returnEntrance->setBossOutStageName(returnEntry.bossOutStage);
            returnEntrance->setBossOutRoomNum(returnEntry.bossOutRoom);
            returnEntrance->setBossOutSpawnId(returnEntry.bossOutSpawnId);
            returnEntrance->setEntranceType(entranceTypeToReverse(type));
            forwardEntrance->bindTwoWay(returnEntrance);
        }
    }
    return EntranceShuffleError::NONE;
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
    LOG_TO_DEBUG("Restoring Connection for " + entrance->getOriginalName());
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
    if (targetEntrance->getConnectedArea() != "")
    {
        targetEntrance->disconnect();
    }
    if (targetEntrance->getParentArea() != "")
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
    LOG_TO_DEBUG("Validating World");
    // Ensure that all item locations are still reachable within the given world
    if (!allLocationsReachable(worlds, itemPool))
    {
        #ifdef ENABLE_DEBUG
            LOG_TO_DEBUG("Error: All locations not reachable");
            logMissingLocations(worlds);
        #endif
        return EntranceShuffleError::ALL_LOCATIONS_NOT_REACHABLE;
    }
    // Ensure that there's at least one sphere zero location available to place an item
    // for the beginning of the seed
    ItemPool noItems = {};
    LocationPool progLocations = {};
    GET_COMPLETE_PROGRESSION_LOCATION_POOL(progLocations, worlds);
    auto locs = getAccessibleLocations(worlds, noItems, progLocations);
    #ifdef ENABLE_DEBUG
        LOG_TO_DEBUG(std::to_string(locs.size()) + " Sphere 0 locations: [");
        // uncomment to see all sphere zero locations
        // for (auto location : locs)
        // {
        //     LOG_TO_DEBUG(location->getName());
        // }
        LOG_TO_DEBUG("]");
    #endif
    if (locs.size() < worlds.size())
    {
        LOG_TO_DEBUG("Error: Not enough sphere zero locations to place items");
        return EntranceShuffleError::NOT_ENOUGH_SPHERE_ZERO_LOCATIONS;
    }

    if (entrancePlaced == nullptr)
    {
        // Check to make sure there's enough sphere 0 locations for the items that
        // are necessary to place at the beginning
        ItemPool itemPool2;
        LocationPool progressionLocations;
        GET_COMPLETE_ITEM_POOL(itemPool2, worlds);
        GET_COMPLETE_PROGRESSION_LOCATION_POOL(progressionLocations, worlds);
        determineMajorItems(worlds, itemPool2, progressionLocations);
        filterAndEraseFromPool(itemPool2, [&](const Item& item){return !item.isMajorItem();});
        auto err = forwardFillUntilMoreFreeSpace(worlds, itemPool2, progressionLocations, 2);
        clearWorlds(worlds);
        if (err != FillError::NONE)
        {
            ErrorLog::getInstance().log("Not enough sphere 0 locations to place necessary items. Please enable more locations, or start with more items");
            return EntranceShuffleError::NOT_ENOUGH_SPHERE_ZERO_LOCATIONS;
        }
    }
    // Ensure that all race mode dungeons are assigned to a single island and that
    // there aren't any other dungeons on those islands. Since quest markers for
    // race mode dungeons indicate an entire island, we don't want the there to be
    // multiple dungeons on an island, or multiple islands that lead to the same
    // race mode dungeon
    for (auto& world : worlds)
    {
        if (world.getSettings().race_mode)
        {
            std::unordered_set<std::string> raceModeIslands = {};
            for (auto& [name, dungeon] : world.dungeons)
            {
                auto& dungeonEntranceRoom = dungeon.entranceRoom;
                auto dungeonIslands = world.getIslands(dungeonEntranceRoom);

                if (dungeon.isRaceModeDungeon)
                {
                    if (dungeonIslands.size() > 1)
                    {
                        #ifdef ENABLE_DEBUG
                            LOG_TO_DEBUG("Error: More than 1 island leading to race mode dungeon " + name);
                            for (auto& island : dungeonIslands)
                            {
                                LOG_TO_DEBUG("\t" + island);
                            }
                        #endif
                        return EntranceShuffleError::AMBIGUOUS_RACE_MODE_ISLAND;
                    }
                }

                if (dungeonIslands.size() == 1)
                {
                    auto dungeonIsland = *dungeonIslands.begin();
                    if (raceModeIslands.contains(dungeonIsland))
                    {
                        LOG_TO_DEBUG("Error: Island " + dungeonIsland + " has an ambiguous race mode dungeon");
                        return EntranceShuffleError::AMBIGUOUS_RACE_MODE_DUNGEON;
                    }

                    if (dungeon.isRaceModeDungeon)
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
    LOG_TO_DEBUG("Attempting to Connect " + entrance->getOriginalName() + " To " + target->getReplaces()->getOriginalName());
    EntranceShuffleError err = EntranceShuffleError::NONE;
    err = checkEntrancesCompatibility(entrance, target, rollbacks);
    ENTRANCE_SHUFFLE_ERROR_CHECK(err);
    changeConnections(entrance, target);
    err = validateWorld(worlds, entrance, itemPool);
    // If the attempted replacement produces an invalid world graph, then undo
    // the attempted connection and try again with a different target.
    if (err != EntranceShuffleError::NONE)
    {
        if (entrance->getConnectedArea() != "")
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
        if (entrance->getConnectedArea() != "")
        {
            continue;
        }
        shufflePool(targetEntrances);

        for (auto target : targetEntrances)
        {
            // If the target has already been disconnected, then don't use it again
            if (target->getConnectedArea() == "")
            {
                continue;
            }
            err = replaceEntrance(worlds, entrance, target, rollbacks, completeItemPool);
            if (err == EntranceShuffleError::NONE)
            {
                break;
            }
        }

        if (entrance->getConnectedArea() == "")
        {
            LOG_TO_DEBUG("Could not connect " + entrance->getOriginalName() + ". Error: " + errorToName(err));
            return EntranceShuffleError::NO_MORE_VALID_ENTRANCES;
        }
    }

    // Verify that all targets were disconnected and that we didn't create any closed root loops
    for (auto target : targetEntrances)
    {
        if (target->getConnectedArea() != "")
        {
            LOG_TO_DEBUG("Error: Target entrance " + target->getCurrentName() + " was never disconnected");
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
            LOG_TO_DEBUG("Failed to place all entrances in a pool for world " + std::to_string(world.getWorldId() + 1) + ". Will retry " + std::to_string(retryCount) + " more times.");
            LOG_TO_DEBUG("Last Error: " + errorToName(err));
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

    LOG_TO_DEBUG("Entrance placement attempt count exceeded for world " + std::to_string(world.getWorldId() + 1));
    return EntranceShuffleError::RAN_OUT_OF_RETRIES;
}

static EntranceShuffleError processPlandomizerEntrances(World& world)
{
    // Sanity check that the entrances put in for plandomizer are good
    for (auto& [originalEntranceStr, replacementEntranceStr] : world.plandomizer.entrancesStr)
    {
        const std::string originalTok = " -> ";
        const std::string replacementTok = " from ";

        // Verify that the format of each one is correct
        auto arrowPos = originalEntranceStr.find(originalTok);
        if (arrowPos == std::string::npos)
        {
            ErrorLog::getInstance().log("Plandomizer Error: Entrance plandomizer string \"" + originalEntranceStr + "\" is not properly formatted.");
            return EntranceShuffleError::PLANDOMIZER_ERROR;
        }
        auto fromPos = replacementEntranceStr.find(replacementTok);
        if (fromPos == std::string::npos)
        {
            ErrorLog::getInstance().log("Plandomizer Error: Entrance plandomizer string \"" + replacementEntranceStr + "\" is not properly formatted.");
            return EntranceShuffleError::PLANDOMIZER_ERROR;
        }

        // Separate out all the area names
        std::string originalEntranceParent = originalEntranceStr.substr(0, arrowPos);
        std::string originalEntranceConnection = originalEntranceStr.substr(arrowPos + originalTok.size());
        std::string replacementEntranceParent = replacementEntranceStr.substr(fromPos + replacementTok.size());
        std::string replacementEntranceConnection = replacementEntranceStr.substr(0, fromPos);

        Entrance* originalEntrance = world.getEntrance(originalEntranceParent, originalEntranceConnection);
        Entrance* replacementEntrance = world.getEntrance(replacementEntranceParent, replacementEntranceConnection);
        // Sanity check the entrance pointers
        if (originalEntrance == nullptr)
        {
            ErrorLog::getInstance().log("Plandomizer Error: Entrance plandomizer string \"" + originalEntranceStr + "\" is incorrect.");
            return EntranceShuffleError::PLANDOMIZER_ERROR;
        }
        if (replacementEntrance == nullptr)
        {
            ErrorLog::getInstance().log("Plandomizer Error: Entrance plandomizer string \"" + replacementEntranceStr + "\" is incorrect.");
            return EntranceShuffleError::PLANDOMIZER_ERROR;
        }

        world.plandomizer.entrances.insert({originalEntrance, replacementEntrance});
        LOG_TO_DEBUG("Plandomizer Entrance for world " + std::to_string(world.getWorldId() + 1) + " - " + originalEntranceStr + ": " + replacementEntranceStr);
    }

    return EntranceShuffleError::NONE;
}

static EntranceShuffleError setPlandomizerEntrances(World& world, WorldPool& worlds, EntrancePools& entrancePools, EntrancePools& targetEntrancePools, std::set<EntranceType>& poolsToMix)
{
    LOG_TO_DEBUG("Now placing plandomized entrances");
    ItemPool completeItemPool = {};
    GET_COMPLETE_ITEM_POOL(completeItemPool, worlds)

    // Now attempt to connect plandomized entrances
    for (auto& [entrance, target] : world.plandomizer.entrances)
    {
        std::string fullConnectionName = "\"" + entrance->getOriginalName() + "\" to \"" + target->getOriginalConnectedArea() + " from " + target->getParentArea() + "\"";
        LOG_TO_DEBUG("Attempting to set plandomized entrance " + fullConnectionName);
        Entrance* entranceToConnect = entrance;
        Entrance* targetToConnect = target;

        auto type = entrance->getEntranceType();
        // If the entrance doesn't have a type, it's not shuffable
        if (type == EntranceType::NONE)
        {
            ErrorLog::getInstance().log("Entrance \"" + entrance->getOriginalName() + "\" cannot be shuffled.");
            return EntranceShuffleError::PLANDOMIZER_ERROR;
        }
        // Change misc restrictive to misc since restrictive entrances are still in the misc pool
        else if (type == EntranceType::MISC_RESTRICTIVE)
        {
            type = EntranceType::MISC;
        }

        // Check to make sure this type of entrance is being shuffled
        if (!entrancePools.contains(type))
        {
            // Check if its reverse is being shuffled if decoupled entrances are off
            if (!world.getSettings().decouple_entrances && entrance->getReverse() != nullptr && entrancePools.contains(entrance->getReverse()->getEntranceType()))
            {
                // If so, take the reverse of the entrance and target and attempt to connect them instead
                entranceToConnect = entrance->getReverse();
                targetToConnect = target->getReverse();
                type = entranceToConnect->getEntranceType();
                LOG_TO_DEBUG("Trying Reverse");
            }
            else
            {
                ErrorLog::getInstance().log("Entrance \"" + entrance->getOriginalName() + "\"'s type is not being shuffled and thus can't be plandomized.");
                return EntranceShuffleError::PLANDOMIZER_ERROR;
            }
        }

        // Get the appropriate pools (depending on if the pool is being mixed)
        auto& entrancePool = entrancePools[poolsToMix.contains(type) ? EntranceType::MIXED : type];
        auto& targetPool = targetEntrancePools[poolsToMix.contains(type) ? EntranceType::MIXED : type];

        if (elementInPool(entranceToConnect, entrancePool))
        {
            bool validTargetFound = false;
            for (auto targetEntrance : targetPool)
            {
                // Loop through until we find the *actual* target entrance with the valid
                // replacement for the entrance the user wants to connect
                if (targetToConnect == targetEntrance->getReplaces())
                {
                    std::vector<EntrancePair> dummyRollbacks = {};
                    EntranceShuffleError err = replaceEntrance(worlds, entranceToConnect, targetEntrance, dummyRollbacks, completeItemPool);
                    if (err != EntranceShuffleError::NONE)
                    {
                        ErrorLog::getInstance().log("Plandomizer Error when attempting to connect " + fullConnectionName + ": " + errorToName(err));
                        return EntranceShuffleError::PLANDOMIZER_ERROR;
                    }
                    validTargetFound = true;
                    // Remove the entrance and target from their pools when it's done
                    removeElementFromPool(entrancePool, entranceToConnect);
                    removeElementFromPool(targetPool, targetEntrance);
                    LOG_TO_DEBUG("Success");
                    break;
                }
            }
            if (!validTargetFound)
            {
                ErrorLog::getInstance().log("Entrance \"" + target->getOriginalConnectedArea() + " from " + target->getParentArea() + "\" is not a valid target for \"" + entrance->getOriginalName() + "\".");
                return EntranceShuffleError::PLANDOMIZER_ERROR;
            }
        }
        else
        {
            ErrorLog::getInstance().log("Entrance \"" + entrance->getOriginalName() + "\" is not being shuffled and thus can't be plandomized.");
            return EntranceShuffleError::PLANDOMIZER_ERROR;
        }

    }
    LOG_TO_DEBUG("All plandomizer entrances have been placed.");
    return EntranceShuffleError::NONE;
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
        auto& settings = world.getSettings();
        // Set random starting island (either if the settings is set, or one is plandomized)
        if (settings.randomize_starting_island || world.plandomizer.startingIslandRoomIndex > 0)
        {

            // Set plandomizer island if there is one
            if (world.plandomizer.startingIslandRoomIndex > 0 && world.plandomizer.startingIslandRoomIndex < 50)
            {
                world.startingIslandRoomIndex = world.plandomizer.startingIslandRoomIndex;
            }
            else
            {
                // Rooms 2 - 49 include every island except Forsaken Fortress
                world.startingIslandRoomIndex = Random(2, 50);
            }

            auto startingIsland = roomIndexToIslandName(world.startingIslandRoomIndex);
            LOG_TO_DEBUG("starting island: \"" + startingIsland + "\" index: " + std::to_string(world.startingIslandRoomIndex));

            // Set the new starting island in the world graph
            auto linksSpawn = world.getEntrance("Link's Spawn", "Outset Island");
            if (linksSpawn == nullptr)
            {
                return EntranceShuffleError::BAD_LINKS_SPAWN;
            }
            linksSpawn->setConnectedArea(startingIsland);
            world.getArea("Outset Island").entrances.remove(linksSpawn);
            world.getArea(startingIsland).entrances.push_back(linksSpawn);
        }

        // Set entrance data for all entrances, even those we aren't shuffling
        err = setAllEntrancesData(world);
        ENTRANCE_SHUFFLE_ERROR_CHECK(err);

        // Process plandomizer entrance data
        err = processPlandomizerEntrances(world);
        ENTRANCE_SHUFFLE_ERROR_CHECK(err);

        // Determine how many mixed pools there will be
        int totalMixedPools = (settings.mix_dungeons ? 1 : 0) + (settings.mix_caves ? 1 : 0) + (settings.mix_doors ? 1 : 0) + (settings.mix_misc ? 1 : 0);

        // Determine entrance pools based on settings, to be shuffled in the order we set them by
        EntrancePools entrancePools = {};
        EntrancePools targetEntrancePools = {};
        if (settings.randomize_dungeon_entrances)
        {
            entrancePools[EntranceType::DUNGEON] = world.getShuffleableEntrances(EntranceType::DUNGEON, true);
            if (settings.decouple_entrances)
            {
                entrancePools[EntranceType::DUNGEON_REVERSE] = getReverseEntrances(entrancePools, EntranceType::DUNGEON);
            }
        }

        if (settings.randomize_cave_entrances)
        {
            entrancePools[EntranceType::CAVE] = world.getShuffleableEntrances(EntranceType::CAVE, true);
            if (settings.decouple_entrances)
            {
                entrancePools[EntranceType::CAVE_REVERSE] = getReverseEntrances(entrancePools, EntranceType::CAVE);
            }
            // Don't randomize the cliff plateau upper isles grotto unless entrances are decoupled
            else
            {
                filterAndEraseFromPool(entrancePools[EntranceType::CAVE], [](Entrance* e){return e->getParentArea() == "Cliff Plateau Highest Isle";});
            }
        }

        if (settings.randomize_door_entrances)
        {
            entrancePools[EntranceType::DOOR] = world.getShuffleableEntrances(EntranceType::DOOR, true);
            if (settings.decouple_entrances)
            {
                entrancePools[EntranceType::DOOR_REVERSE] = getReverseEntrances(entrancePools, EntranceType::DOOR);
            }
        }

        if (settings.randomize_misc_entrances)
        {
            // Allow both primary and non-primary entrances in the MISC pool unless
            // we're mixing the entrance pools and aren't decoupling entrances
            entrancePools[EntranceType::MISC] = world.getShuffleableEntrances(EntranceType::MISC, settings.mix_misc && totalMixedPools > 1 && !settings.decouple_entrances);
            auto miscRestrictiveEntrances = world.getShuffleableEntrances(EntranceType::MISC_RESTRICTIVE, !settings.decouple_entrances);
            addElementsToPool(entrancePools[EntranceType::MISC], miscRestrictiveEntrances);

            // Keep crawlspaces separate for the time-being since spawning in a crawlspace
            // entrance while standing up can potentially softlock
            entrancePools[EntranceType::MISC_CRAWLSPACE] = world.getShuffleableEntrances(EntranceType::MISC_CRAWLSPACE, true);
            if (settings.decouple_entrances)
            {
                entrancePools[EntranceType::MISC_CRAWLSPACE_REVERSE] = getReverseEntrances(entrancePools, EntranceType::MISC_CRAWLSPACE);
            }
        }

        SetShuffledEntrances(entrancePools);

        // Combine entrance pools if mixing pools. Only continue if more than one pool is selected
        std::set<EntranceType> poolsToMix;
        if (totalMixedPools > 1)
        {
            CHECK_MIXED_POOL(mix_dungeons, EntranceType::DUNGEON);
            CHECK_MIXED_POOL(mix_doors, EntranceType::DOOR);
            CHECK_MIXED_POOL(mix_caves, EntranceType::CAVE);
            if (settings.mix_misc)
            {
                poolsToMix.insert(EntranceType::MISC);
            }
            entrancePools[EntranceType::MIXED] = {};
            // For each entrance type, add the entrance to the mixed pool instead
            for (auto& [type, entrancePool] : entrancePools)
            {
                // Don't re-add the mixed pool to itself and don't mix crawlspaces
                if (poolsToMix.contains(type) && type != EntranceType::MIXED && type != EntranceType::MISC_CRAWLSPACE && type != EntranceType::MISC_CRAWLSPACE_REVERSE)
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
            #ifdef ENABLE_DEBUG
                logEntrancePool(targetEntrancePools[type], "Targets for entrance type " + entranceTypeToName(type));
            #endif
        }

        // Shuffle Plandomized entrances at this point
        err = setPlandomizerEntrances(world, worlds, entrancePools, targetEntrancePools, poolsToMix);
        if (err != EntranceShuffleError::NONE)
        {
            LOG_TO_DEBUG("| Encountered when setting plandomizer entrances");
            return err;
        }

        // Shuffle the entrances
        for (auto& [type, entrancePool] : entrancePools)
        {
            err = shuffleEntrancePool(world, worlds, entrancePool, targetEntrancePools[type]);
            if (err != EntranceShuffleError::NONE)
            {
                LOG_TO_DEBUG("| Encountered when shuffling pool of type " + entranceTypeToName(type));
                return err;
            }
        }

        // Now set the islands the race mode dungeons are in
        for (auto& [name, dungeon] : world.dungeons)
        {
            auto islands = world.getIslands(dungeon.entranceRoom);
            if (islands.empty())
            {
                ErrorLog::getInstance().log("ERROR: No Island for dungeon " + name);
                LOG_ERR_AND_RETURN(EntranceShuffleError::NO_RACE_MODE_ISLAND);
            }
            dungeon.island = *islands.begin();
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
        case EntranceShuffleError::BAD_LINKS_SPAWN:
            return "BAD_LINKS_SPAWN";
        case EntranceShuffleError::BAD_ENTRANCE_SHUFFLE_TABLE_ENTRY:
            return "BAD_ENTRANCE_SHUFFLE_TABLE_ENTRY";
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
        case EntranceShuffleError::PLANDOMIZER_ERROR:
            return "PLANDOMIZER_ERROR";
        default:
            return "UNKNOWN";
    }

}
