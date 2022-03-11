
#pragma once

#include "Entrance.hpp"
#include "World.hpp"

enum struct EntranceShuffleError
{
    NONE = 0,
    RAN_OUT_OF_RETRIES,
    NO_MORE_VALID_ENTRANCES,
    ALL_LOCATIONS_NOT_REACHABLE,
};

struct EntranceInfo
{
    Area parentArea = Area::INVALID;
    Area connectedArea = Area::INVALID;
    std::string stageName;
    uint8_t roomNum;
    uint8_t sclsExitIndex;
    uint8_t spawnId;
    std::string warpOutStageName = "";
    uint8_t warpOutRoomNum;
    uint8_t warpOutSpawnId;
};

struct EntranceInfoPair
{
    EntranceType type;
    EntranceInfo forwardEntry;
    EntranceInfo returnEntry;
};

EntranceShuffleError randomizeEntrances(WorldPool& worlds);
const std::string errorToName(EntranceShuffleError err);
