
#include "Entrance.hpp"
#include "World.hpp"

Entrance::Entrance() {}

Entrance::Entrance(const Area& parentArea_, const Area& connectedArea_, World* world_)
{
    parentArea = parentArea_;
    connectedArea = connectedArea_;
    world = world_;
    worldId = world->getWorldId();
    requirement = {RequirementType::HAS_ITEM, {GameItem::NOTHING}};
    setOriginalName();
}

Area Entrance::getParentArea() const
{
    return parentArea;
}

void Entrance::setParentArea(Area newParentArea)
{
    parentArea = newParentArea;
}

Area Entrance::getConnectedArea() const
{
    return connectedArea;
}

void Entrance::setConnectedArea(Area newConnectedArea)
{
    connectedArea = newConnectedArea;
}

Requirement& Entrance::getRequirement()
{
    return requirement;
}

void Entrance::setRequirement(const Requirement newRequirement)
{
    requirement = std::move(newRequirement);
}

EntranceType Entrance::getEntranceType() const
{
    return type;
}

void Entrance::setEntranceType(EntranceType& newType)
{
    type = newType;
}

bool Entrance::isPrimary() const
{
    return primary;
}

void Entrance::setAsPrimary()
{
    primary = true;
}

std::string Entrance::getStageName() const
{
    return stageName;
}

void Entrance::setStageName(std::string newStageName)
{
    stageName = std::move(newStageName);
}

std::string Entrance::getOriginalName() const
{
    return originalName;
}

void Entrance::setOriginalName()
{
    if (!alreadySetOriginalName)
    {
        originalName = areaToName(parentArea) + " -> " + areaToName(connectedArea);
        alreadySetOriginalName = true;
    }
}

std::string Entrance::getCurrentName() const
{
    return areaToName(parentArea) + " -> " + areaToName(connectedArea);
}

uint8_t Entrance::getRoomNum() const
{
    return roomNum;
}

void Entrance::setRoomNum(uint8_t& newRoomNum)
{
    roomNum = newRoomNum;
}

uint8_t Entrance::getSclsExitIndex() const
{
    return sclsExitIndex;
}

void Entrance::setSclsExitIndex(uint8_t& newExitIndex)
{
    sclsExitIndex = newExitIndex;
}

uint8_t Entrance::getSpawnId() const
{
    return spawnId;
}

void Entrance::setSpawnId(uint8_t& newSpawnId)
{
    spawnId = newSpawnId;
}

std::string Entrance::getWarpOutStageName() const
{
    return warpOutStageName;
}

void Entrance::setWarpOutStageName(std::string newWarpOutStageName)
{
    warpOutStageName = std::move(newWarpOutStageName);
}

uint8_t Entrance::getWarpOutRoomNum() const
{
    return warpOutRoomNum;
}

void Entrance::setWarpOutRoomNum(uint8_t& newWarpOutRoomNum)
{
    warpOutRoomNum = newWarpOutRoomNum;
}

uint8_t Entrance::getWarpOutSpawnId() const
{
    return warpOutSpawnId;
}

void Entrance::setWarpOutSpawnId(uint8_t& newWarpOutSpawnId)
{
    warpOutSpawnId = newWarpOutSpawnId;
}

int Entrance::getWorldId() const
{
    return worldId;
}

void Entrance::setWorldId(int& newWorldId)
{
    worldId = newWorldId;
}

Entrance* Entrance::getReverse()
{
    return reverse;
}

void Entrance::setReverse(Entrance* reverseEntrance)
{
    reverse = reverseEntrance;
}

Entrance* Entrance::getReplaces()
{
    return replaces;
}

void Entrance::setReplaces(Entrance* replacedEntrance)
{
    replaces = replacedEntrance;
}

Entrance* Entrance::getAssumed()
{
    return assumed;
}

bool Entrance::isShuffled() const
{
    return shuffled;
}

void Entrance::setAsShuffled()
{
    shuffled = true;
}

World* Entrance::getWorld()
{
    return world;
}

void Entrance::setWorld(World* newWorld)
{
    world = newWorld;
}

std::unordered_set<HintRegion> Entrance::getIslands()
{
    return world->getIslands(parentArea);
}

void Entrance::connect(const Area newConnectedArea)
{
    connectedArea = newConnectedArea;
    world->areaEntries[areaAsIndex(connectedArea)].entrances.push_back(this);
}

Area Entrance::disconnect()
{
    world->areaEntries[areaAsIndex(connectedArea)].entrances.remove(this);
    Area previouslyConnected = connectedArea;
    connectedArea = Area::INVALID;
    return previouslyConnected;
}

void Entrance::bindTwoWay(Entrance* otherEntrance)
{
    reverse = otherEntrance;
    otherEntrance->setReverse(this);
}

Entrance* Entrance::getNewTarget()
{
    auto& root = world->areaEntries[areaAsIndex(Area::Root)];
    root.exits.emplace_back(Area::Root, connectedArea, world);
    Entrance& targetEntrance = root.exits.back();
    targetEntrance.connect(connectedArea);
    targetEntrance.setReplaces(this);
    return &targetEntrance;
}

Entrance* Entrance::assumeReachable()
{
    if (assumed == nullptr)
    {
        assumed = getNewTarget();
        disconnect();
    }
    return assumed;
}

std::string entranceTypeToName(const EntranceType& type)
{
    std::unordered_map<EntranceType, std::string> typeNameMap = {
        {EntranceType::NONE, "NONE"},
        {EntranceType::DUNGEON, "DUNGEON"},
        {EntranceType::DUNGEON_REVERSE, "DUNGEON_REVERSE"},
        {EntranceType::CAVE, "CAVE"},
        {EntranceType::CAVE_REVERSE, "CAVE_REVERSE"},
        {EntranceType::DOOR, "DOOR"},
        {EntranceType::DOOR_REVERSE, "DOOR_REVERSE"},
        {EntranceType::MISC, "MISC"},
        {EntranceType::MISC_REVERSE, "MISC_REVERSE"},
        {EntranceType::MISC_RESTRICTIVE, "MISC_RESTRICTIVE"},
        {EntranceType::MISC_RESTRICTIVE_REVERSE, "MISC_RESTRICTIVE_REVERSE"},
        {EntranceType::MISC_CRAWLSPACE, "MISC_CRAWLSPACE"},
        {EntranceType::MISC_CRAWLSPACE_REVERSE, "MISC_CRAWLSPACE_REVERSE"},
        {EntranceType::MIXED, "MIXED"},
        {EntranceType::ALL, "ALL"},
    };

    if (typeNameMap.count(type) == 0)
    {
        return "INVALID ENTRANCE TYPE";
    }
    return typeNameMap.at(type);
}
