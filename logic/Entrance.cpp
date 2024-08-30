
#include "Entrance.hpp"

#include <logic/World.hpp>
#include <logic/Area.hpp>
#include <command/Log.hpp>

Entrance::Entrance(Area* parentArea_, Area* connectedArea_, World* world_) :
    parentArea(parentArea_),
    connectedArea(connectedArea_),
    originalConnectedArea(connectedArea_),
    alreadySetOriginalConnectedArea(true),
    world(world_)
{
    worldId = world->getWorldId();
    setOriginalName();
    requirement = Requirement();
    requirement.type = RequirementType::NOTHING;
    requirement.args = {};
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
    requirement = newRequirement;
}

EntranceType Entrance::getEntranceType() const
{
    return type;
}

void Entrance::setEntranceType(EntranceType newType)
{
    type = newType;
}

EntranceType Entrance::getOriginalEntranceType() const
{
    return originalType;
}

void Entrance::setOriginalEntranceType(EntranceType newType)
{
    originalType = newType;
}

bool Entrance::isPrimary() const
{
    return primary;
}

void Entrance::setAsPrimary()
{
    primary = true;
}

std::string Entrance::getOriginalName(const bool& arenaExitNameChange /*= false*/) const
{
    // If this is a reverse boss entrance, change it's name to be more
    // intuitive as to what entrance it actually is (the wind warp)
    if (arenaExitNameChange && originalName.substr(0, originalName.find(" -> ")).ends_with("Battle Arena"))
    {
        // Change to "Boss Battle Arena -> Boss Battle Arena Exit"
        return originalName.substr(0, originalName.find(" -> ")) + " -> " + 
               originalName.substr(0, originalName.find(" -> ")) + " Exit";
        
    }
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

void Entrance::setSavewarp(const bool& savewarp_)
{
    savewarp = savewarp_;
}

bool Entrance::needsSavewarp() const
{
    return savewarp;
}

void Entrance::setWindWarp(const bool& windWarp_)
{
    windWarp = windWarp_;
}

bool Entrance::hasWindWarp() const
{
    return windWarp;
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

Requirement Entrance::getComputedRequirement() const
{
    return computedRequirement;
}

void Entrance::setComputedRequirement(const Requirement& req)
{
    computedRequirement = req;
    computedRequirement.simplifyParenthesis();
    computedRequirement.sortArgs();
}

bool Entrance::hasBeenFound() const
{
    return found;
}

void Entrance::setFound(const bool& found_)
{
    found = found_;
}

bool Entrance::isHidden() const
{
    return hidden;
}

void Entrance::setHidden(const bool& hidden_)
{
    hidden = hidden_;
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

    // TODO: should do something like PointerLess<Area>{}(this->parentArea, rhs.parentArea);
    // not sure how best to sort areas though
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
        {EntranceType::FAIRY, "FAIRY"},
        {EntranceType::FAIRY_REVERSE, "FAIRY_REVERSE"},
        {EntranceType::DOOR, "DOOR"},
        {EntranceType::DOOR_REVERSE, "DOOR_REVERSE"},
        {EntranceType::MISC, "MISC"},
        {EntranceType::MISC_RESTRICTIVE, "MISC_RESTRICTIVE"},
        {EntranceType::MISC_REVERSE, "MISC_REVERSE"},
        {EntranceType::MIXED, "MIXED"},
        {EntranceType::MIXED_REVERSE, "MIXED_REVERSE"},
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
        {"FAIRY", EntranceType::FAIRY},
        {"FAIRY_REVERSE", EntranceType::FAIRY_REVERSE},
        {"DOOR", EntranceType::DOOR},
        {"DOOR_REVERSE", EntranceType::DOOR_REVERSE},
        {"MISC", EntranceType::MISC},
        {"MISC_RESTRICTIVE", EntranceType::MISC_RESTRICTIVE},
        {"MISC_REVERSE", EntranceType::MISC_REVERSE},
        {"MIXED", EntranceType::MIXED},
        {"MIXED_REVERSE", EntranceType::MIXED_REVERSE},
        {"ALL", EntranceType::ALL},
    };

    if (!nameToType.contains(name))
    {
        return EntranceType::NONE;
    }
    return nameToType.at(name);
}

EntranceType entranceTypeToReverse(const EntranceType& type, bool miscReverse /*= true*/)
{
    if (!miscReverse && type == EntranceType::MISC)
    {
        return EntranceType::MISC;
    }

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
        {EntranceType::FAIRY, EntranceType::FAIRY_REVERSE},
        {EntranceType::FAIRY_REVERSE, EntranceType::FAIRY},
        {EntranceType::DOOR, EntranceType::DOOR_REVERSE},
        {EntranceType::DOOR_REVERSE, EntranceType::DOOR},
        {EntranceType::MISC, EntranceType::MISC_REVERSE},
        {EntranceType::MISC_REVERSE, EntranceType::MISC},
        {EntranceType::MISC_RESTRICTIVE, EntranceType::MISC_RESTRICTIVE},
        {EntranceType::MIXED, EntranceType::MIXED_REVERSE},
        {EntranceType::MIXED_REVERSE, EntranceType::MIXED},
        {EntranceType::ALL, EntranceType::ALL},
    };

    if (!typeReverseMap.contains(type))
    {
        return EntranceType::NONE;
    }
    return typeReverseMap.at(type);
}

// Decides if the current entrance path is better than the one passed in based on
// certain criteria. This is used to determine what entrance path should be shown
// to a user when they hover over locations/entrances in the tracker. Generally, the
// shortest, most logical path should be shown. However, if there is a longer, equally
// logical path through the currently selected area, then that one will be shown instead
bool EntrancePath::isBetterThan(const EntrancePath& other, const std::string& curArea)
{
    // If this path has better logicality then it's always better
    if (logicality > other.logicality)
    {
        return true;
    }

    // If the other list is empty and has no logicality, this path is always better
    if (other.list.empty() && other.logicality == EntrancePath::Logicality::None)
    {
        return true;
    }

    // If the other list is empty (and it doesn't have no logicality), then it's always better.
    // At this point, the other path either has equal or better logicality than this path
    if (other.list.empty())
    {
        return false;
    }

    // If this path has the same logicality, then evaluate other criteria
    if (logicality == other.logicality)
    {
        // If this list is empty, it's always better
        if (list.empty())
        {
            return true;
        }

        auto thisStartArea = list.front()->getParentArea()->getRegion();
        auto otherStartArea = other.list.front()->getParentArea()->getRegion();

        // If this path's start area matches the current area, and the other path's start area does not,
        // then this path is better than the other one. This takes priority over list size
        if (thisStartArea == curArea && otherStartArea != curArea)
        {
            return true;
        }

        // If this list is smaller than the other one, and the other path's start area does not equal
        // the current area, or this path's start area is the current area, then this list is better
        if (list.size() < other.list.size() && (otherStartArea != curArea || thisStartArea == curArea))
        {
            return true;
        }
    }

    return false;
}

std::string EntrancePath::to_string() const
{
    std::string returnStr = "";
    for (const auto& entrance : list)
    {
        returnStr += entrance->getOriginalName() + "\n";
    }
    return returnStr;
}