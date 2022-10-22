
#include "Entrance.hpp"

#include <logic/World.hpp>
#include <command/Log.hpp>

Entrance::Entrance() {}

Entrance::Entrance(const std::string& parentArea_, const std::string& connectedArea_, World* world_)
{
    parentArea = parentArea_;
    connectedArea = connectedArea_;
    originalConnectedArea = connectedArea_;
    alreadySetOriginalConnectedArea = true;
    world = world_;
    worldId = world->getWorldId();
    requirement = {RequirementType::NOTHING, {}};
    setOriginalName();
}

std::string Entrance::getParentArea() const
{
    return parentArea;
}

void Entrance::setParentArea(const std::string& newParentArea)
{
    parentArea = newParentArea;
}

std::string Entrance::getConnectedArea() const
{
    return connectedArea;
}

void Entrance::setConnectedArea(const std::string& newConnectedArea)
{
    connectedArea = newConnectedArea;
    if (!alreadySetOriginalConnectedArea)
    {
        originalConnectedArea = newConnectedArea;
    }
}

std::string Entrance::getOriginalConnectedArea() const
{
    return originalConnectedArea;
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

void Entrance::setEntranceType(EntranceType newType)
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

std::string Entrance::getOriginalName() const
{
    return originalName;
}

void Entrance::setOriginalName()
{
    if (!alreadySetOriginalName)
    {
        originalName = parentArea + " -> " + connectedArea;
        alreadySetOriginalName = true;
    }
}

std::string Entrance::getCurrentName() const
{
    return parentArea + " -> " + connectedArea;
}

std::string Entrance::getFilepathStage() const
{
    return filepathStage;
}

void Entrance::setFilepathStage(std::string newFilepathStage)
{
    filepathStage = std::move(newFilepathStage);
}

uint8_t Entrance::getFilepathRoomNum() const
{
    return filepathRoom;
}

void Entrance::setFilepathRoomNum(uint8_t& newFilepathRoomNum)
{
    filepathRoom = newFilepathRoomNum;
}

uint8_t Entrance::getSclsExitIndex() const
{
    return sclsExitIndex;
}

void Entrance::setSclsExitIndex(uint8_t& newExitIndex)
{
    sclsExitIndex = newExitIndex;
}

std::string Entrance::getStageName() const
{
    return stage;
}

void Entrance::setStageName(std::string newStageName)
{
    stage = std::move(newStageName);
}


uint8_t Entrance::getRoomNum() const
{
    return room;
}

void Entrance::setRoomNum(uint8_t& newRoomNum)
{
    room = newRoomNum;
}

uint8_t Entrance::getSpawnId() const
{
    return spawnId;
}

void Entrance::setSpawnId(uint8_t& newSpawnId)
{
    spawnId = newSpawnId;
}

std::string Entrance::getBossFilepathStageName() const
{
    return bossFilepathStage;
}

void Entrance::setBossFilepathStageName(std::string newBossOutStageName)
{
    bossFilepathStage = std::move(newBossOutStageName);
}

std::string Entrance::getBossOutStageName() const
{
    return bossOutStage;
}

void Entrance::setBossOutStageName(std::string newBossOutStageName)
{
    bossOutStage = std::move(newBossOutStageName);
}

uint8_t Entrance::getBossOutRoomNum() const
{
    return bossOutRoom;
}

void Entrance::setBossOutRoomNum(uint8_t& newBossOutRoomNum)
{
    bossOutRoom = newBossOutRoomNum;
}

uint8_t Entrance::getBossOutSpawnId() const
{
    return bossOutSpawnId;
}

void Entrance::setBossOutSpawnId(uint8_t& newBossOutSpawnId)
{
    bossOutSpawnId = newBossOutSpawnId;
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

std::unordered_set<std::string> Entrance::getIslands()
{
    return world->getIslands(parentArea);
}

void Entrance::connect(const std::string& newConnectedArea)
{
    connectedArea = newConnectedArea;
    world->getArea(connectedArea).entrances.push_back(this);
}

std::string Entrance::disconnect()
{
    world->getArea(connectedArea).entrances.remove(this);
    std::string previouslyConnected = connectedArea;
    connectedArea = "";
    return previouslyConnected;
}

void Entrance::bindTwoWay(Entrance* otherEntrance)
{
    reverse = otherEntrance;
    otherEntrance->setReverse(this);
}

Entrance* Entrance::getNewTarget()
{
    auto& root = world->getArea("Root");
    root.exits.emplace_back("Root", connectedArea, world);
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

bool Entrance::operator<(const Entrance& rhs) const
{
    if (this->worldId != rhs.worldId)
    {
        return this->worldId < rhs.worldId;
    }

    return this->parentArea < rhs.parentArea;
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
        {EntranceType::MISC_RESTRICTIVE, "MISC_RESTRICTIVE"},
        {EntranceType::MISC_CRAWLSPACE, "MISC_CRAWLSPACE"},
        {EntranceType::MISC_CRAWLSPACE_REVERSE, "MISC_CRAWLSPACE_REVERSE"},
        {EntranceType::MIXED, "MIXED"},
        {EntranceType::ALL, "ALL"},
    };

    if (!typeNameMap.contains(type))
    {
        return "INVALID ENTRANCE TYPE";
    }
    return typeNameMap.at(type);
}

EntranceType entranceTypeToReverse(const EntranceType& type)
{
    std::unordered_map<EntranceType, EntranceType> typeReverseMap = {
        {EntranceType::NONE, EntranceType::NONE},
        {EntranceType::DUNGEON, EntranceType::DUNGEON_REVERSE},
        {EntranceType::DUNGEON_REVERSE, EntranceType::DUNGEON},
        {EntranceType::CAVE, EntranceType::CAVE_REVERSE},
        {EntranceType::CAVE_REVERSE, EntranceType::CAVE},
        {EntranceType::DOOR, EntranceType::DOOR_REVERSE},
        {EntranceType::DOOR_REVERSE, EntranceType::DOOR},
        {EntranceType::MISC, EntranceType::MISC},
        {EntranceType::MISC_RESTRICTIVE, EntranceType::MISC_RESTRICTIVE},
        {EntranceType::MISC_CRAWLSPACE, EntranceType::MISC_CRAWLSPACE_REVERSE},
        {EntranceType::MISC_CRAWLSPACE_REVERSE, EntranceType::MISC_CRAWLSPACE},
        {EntranceType::MIXED, EntranceType::MIXED},
        {EntranceType::ALL, EntranceType::ALL},
    };

    if (!typeReverseMap.contains(type))
    {
        return EntranceType::NONE;
    }
    return typeReverseMap.at(type);
}
