
#pragma once

#include <logic/Entrance.hpp>
#include <logic/World.hpp>

enum struct EntranceShuffleError
{
    NONE = 0,
    BAD_LINKS_SPAWN,
    BAD_ENTRANCE_SHUFFLE_TABLE_ENTRY,
    RAN_OUT_OF_RETRIES,
    NO_MORE_VALID_ENTRANCES,
    ALL_LOCATIONS_NOT_REACHABLE,
    AMBIGUOUS_RACE_MODE_ISLAND,
    AMBIGUOUS_RACE_MODE_DUNGEON,
    NO_RACE_MODE_ISLAND,
    NOT_ENOUGH_SPHERE_ZERO_LOCATIONS,
    ATTEMPTED_SELF_CONNECTION,
    FAILED_TO_DISCONNECT_TARGET,
    DUNGEON_ENTRANCES_CONNECTED,
    PLANDOMIZER_ERROR,
};

struct EntranceInfo
{
    std::string parentArea = "INVALID";
    std::string connectedArea = "INVALID";
    std::string filepathStage = "";
    uint8_t filepathRoom = 0xFF;
    uint8_t sclsExitIndex = 0xFF;
    std::string stage = "";
    uint8_t room = 0xFF;
    uint8_t spawnId = 0xFF;
    std::string bossFilepathStage = "";
    std::string bossOutStage = "";
    uint8_t bossOutRoom = 0xFF;
    uint8_t bossOutSpawnId = 0xFF;
};

struct EntranceInfoPair
{
    EntranceType type;
    EntranceInfo forwardEntry;
    EntranceInfo returnEntry;
    bool savewarp = false;
    bool windWarp = false;
};

using EntrancePools = std::map<EntranceType, EntrancePool>;
using EntrancePair = std::pair<Entrance*, Entrance*>;

void changeConnections(Entrance* entrance, Entrance* targetEntrance);
void restoreConnections(Entrance* entrance, Entrance* targetEntrance);
EntranceShuffleError setAllEntrancesData(World& world);
EntrancePools createEntrancePools(World& world, std::set<EntranceType>& poolsToMix);
EntrancePools createTargetEntrances(EntrancePools& entrancePools);
EntranceShuffleError randomizeEntrances(WorldPool& worlds);
const std::string errorToName(EntranceShuffleError err);
