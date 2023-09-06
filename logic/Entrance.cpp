
#include "Entrance.hpp"

#include <logic/World.hpp>
#include <logic/Area.hpp>
#include <command/Log.hpp>

Entrance::Entrance() {}

Entrance::Entrance(Area* parentArea_, Area* connectedArea_, World* world_)
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

Area* Entrance::getParentArea() const
{
    return parentArea;
}

void Entrance::setParentArea(Area* newParentArea)
{
    parentArea = newParentArea;
}

Area* Entrance::getConnectedArea() const
{
    return connectedArea;
}

void Entrance::setConnectedArea(Area* newConnectedArea)
{
    connectedArea = newConnectedArea;
    if (!alreadySetOriginalConnectedArea)
    {
        originalConnectedArea = newConnectedArea;
    }
}

Area* Entrance::getOriginalConnectedArea() const
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
        originalName = parentArea->name + " -> " + connectedArea->name;
        alreadySetOriginalName = true;
    }
}

std::string Entrance::getCurrentName() const
{
    return parentArea->name + " -> " + connectedArea->name;
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

void Entrance::setSavewarp(const bool& savewarp_)
{
    savewarp = savewarp_;
}

bool Entrance::needsSavewarp() const
{
    return savewarp;
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

void Entrance::setAsUnshuffled()
{
    shuffled = false;
}

bool Entrance::isDecoupled() const
{
    return decoupled;
}

void Entrance::setAsDecoupled()
{
    decoupled = true;
}

World* Entrance::getWorld()
{
    return world;
}

void Entrance::setWorld(World* newWorld)
{
    world = newWorld;
}

std::list<std::string> Entrance::findIslands()
{
    return parentArea->findIslands();
}

void Entrance::connect(Area* newConnectedArea)
{
    connectedArea = newConnectedArea;
    connectedArea->entrances.push_back(this);
}

Area* Entrance::disconnect()
{
    connectedArea->entrances.remove(this);
    auto previouslyConnected = connectedArea;
    connectedArea = nullptr;
    return previouslyConnected;
}

void Entrance::bindTwoWay(Entrance* otherEntrance)
{
    this->setReverse(otherEntrance);
    otherEntrance->setReverse(this);
}

Entrance* Entrance::getNewTarget()
{
    auto root = world->getArea("Root");
    root->exits.emplace_back(root, connectedArea, world);
    Entrance& targetEntrance = root->exits.back();
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
        {EntranceType::MINIBOSS, "MINIBOSS"},
        {EntranceType::MINIBOSS_REVERSE, "MINIBOSS_REVERSE"},
        {EntranceType::BOSS, "BOSS"},
        {EntranceType::BOSS_REVERSE, "BOSS_REVERSE"},
        {EntranceType::CAVE, "CAVE"},
        {EntranceType::CAVE_REVERSE, "CAVE_REVERSE"},
        {EntranceType::DOOR, "DOOR"},
        {EntranceType::DOOR_REVERSE, "DOOR_REVERSE"},
        {EntranceType::MISC, "MISC"},
        {EntranceType::MISC_RESTRICTIVE, "MISC_RESTRICTIVE"},
        {EntranceType::MIXED, "MIXED"},
        {EntranceType::ALL, "ALL"},
    };

    if (!typeNameMap.contains(type))
    {
        return "INVALID ENTRANCE TYPE";
    }
    return typeNameMap.at(type);
}

EntranceType entranceNameToType(const std::string& name)
{
    std::unordered_map<std::string, EntranceType> nameToType = {
        {"NONE", EntranceType::NONE},
        {"BOSS", EntranceType::BOSS},
        {"BOSS_REVERSE", EntranceType::BOSS_REVERSE},
        {"MINIBOSS", EntranceType::MINIBOSS},
        {"MINIBOSS_REVERSE", EntranceType::MINIBOSS_REVERSE},
        {"DUNGEON", EntranceType::DUNGEON},
        {"DUNGEON_REVERSE", EntranceType::DUNGEON_REVERSE},
        {"CAVE", EntranceType::CAVE},
        {"CAVE_REVERSE", EntranceType::CAVE_REVERSE},
        {"DOOR", EntranceType::DOOR},
        {"DOOR_REVERSE", EntranceType::DOOR_REVERSE},
        {"MISC", EntranceType::MISC},
        {"MISC_RESTRICTIVE", EntranceType::MISC_RESTRICTIVE},
        {"MIXED", EntranceType::MIXED},
        {"ALL", EntranceType::ALL},
    };

    if (!nameToType.contains(name))
    {
        return EntranceType::NONE;
    }
    return nameToType.at(name);
}

EntranceType entranceTypeToReverse(const EntranceType& type)
{
    std::unordered_map<EntranceType, EntranceType> typeReverseMap = {
        {EntranceType::NONE, EntranceType::NONE},
        {EntranceType::DUNGEON, EntranceType::DUNGEON_REVERSE},
        {EntranceType::DUNGEON_REVERSE, EntranceType::DUNGEON},
        {EntranceType::BOSS, EntranceType::BOSS_REVERSE},
        {EntranceType::BOSS_REVERSE, EntranceType::BOSS},
        {EntranceType::MINIBOSS, EntranceType::MINIBOSS_REVERSE},
        {EntranceType::MINIBOSS_REVERSE, EntranceType::MINIBOSS},
        {EntranceType::CAVE, EntranceType::CAVE_REVERSE},
        {EntranceType::CAVE_REVERSE, EntranceType::CAVE},
        {EntranceType::DOOR, EntranceType::DOOR_REVERSE},
        {EntranceType::DOOR_REVERSE, EntranceType::DOOR},
        {EntranceType::MISC, EntranceType::MISC},
        {EntranceType::MISC_RESTRICTIVE, EntranceType::MISC_RESTRICTIVE},
        {EntranceType::MIXED, EntranceType::MIXED},
        {EntranceType::ALL, EntranceType::ALL},
    };

    if (!typeReverseMap.contains(type))
    {
        return EntranceType::NONE;
    }
    return typeReverseMap.at(type);
}
