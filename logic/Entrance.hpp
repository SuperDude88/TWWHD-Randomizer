
#pragma once

#include <unordered_set>

#include <logic/Requirements.hpp>

enum struct EntranceType
{
    NONE = 0,
    BOSS,
    BOSS_REVERSE,
    MINIBOSS,
    MINIBOSS_REVERSE,
    DUNGEON,
    DUNGEON_REVERSE,
    CAVE,
    CAVE_REVERSE,
    FAIRY,
    FAIRY_REVERSE,
    DOOR,
    DOOR_REVERSE,
    MISC,
    MISC_RESTRICTIVE,
    MIXED,
    ALL,
};

class Entrance;
class World;
class Area;

class Entrance
{
public:

    Entrance();
    Entrance(Area* parentArea_, Area* connectedArea_, World* world_);

    Area* getParentArea() const;
    void setParentArea(Area* newParentArea);
    Area* getConnectedArea() const;
    void setConnectedArea(Area* newConnectedArea);
    Area* getOriginalConnectedArea() const;
    Requirement& getRequirement();
    void setRequirement(const Requirement newRequirement);
    EntranceType getEntranceType() const;
    void setEntranceType(EntranceType newType);
    EntranceType getOriginalEntranceType() const;
    void setOriginalEntranceType(EntranceType newType);
    bool isPrimary() const;
    void setAsPrimary();
    std::string getOriginalName(const bool& arenaExitNameChange = false);
    void setOriginalName();
    std::string getCurrentName() const;
    std::string getFilepathStage() const;
    void setFilepathStage(std::string newFilepathStage);
    uint8_t getFilepathRoomNum() const;
    void setFilepathRoomNum(uint8_t& newFilepathRoomNum);
    uint8_t getSclsExitIndex() const;
    void setSclsExitIndex(uint8_t& newExitIndex);
    std::string getStageName() const;
    void setStageName(std::string newStageName);
    uint8_t getRoomNum() const;
    void setRoomNum(uint8_t& newRoomNum);
    uint8_t getSpawnId() const;
    void setSpawnId(uint8_t& newSpawnId);
    void setSavewarp(const bool& savewarp_);
    bool needsSavewarp() const;
    void setWindWarp(const bool& windWarp_);
    bool hasWindWarp() const;
    int getWorldId() const;
    void setWorldId(int& newWorldId);
    Entrance* getReverse();
    void setReverse(Entrance* reverseEntrance);
    Entrance* getReplaces();
    void setReplaces(Entrance* replacementEntrance);
    Entrance* getAssumed();
    bool isShuffled() const;
    void setAsShuffled();
    void setAsUnshuffled();
    bool isDecoupled() const;
    void setAsDecoupled();
    World* getWorld();
    void setWorld(World* newWorld);
    std::list<std::string> findIslands();

    void connect(Area* newConnectedArea);
    Area* disconnect();
    void bindTwoWay(Entrance* otherEntrance);
    Entrance* getNewTarget();
    Entrance* assumeReachable();

    bool operator<(const Entrance& rhs) const;

private:

    Area* parentArea = nullptr;
    Area* connectedArea = nullptr;
    Area* originalConnectedArea = nullptr;
    Requirement requirement;
    EntranceType type = EntranceType::NONE;
    EntranceType originalType = EntranceType::NONE;
    bool primary = false;
    std::string originalName = "";
    bool alreadySetOriginalName = false;
    bool alreadySetOriginalConnectedArea = false;
    std::string filepathStage = "";
    uint8_t filepathRoom = 0xFF;
    uint8_t sclsExitIndex = 0xFF;
    std::string stage = "";
    uint8_t room = 0xFF;
    uint8_t spawnId = 0xFF;
    bool savewarp = false;
    bool windWarp = false;
    int worldId = -1;
    Entrance* reverse = nullptr;
    Entrance* replaces = nullptr;
    Entrance* assumed = nullptr;
    bool shuffled = false;
    bool decoupled = false;
    World* world = nullptr;
};

std::string entranceTypeToName(const EntranceType& type);
EntranceType entranceNameToType(const std::string& name);
EntranceType entranceTypeToReverse(const EntranceType& type);
