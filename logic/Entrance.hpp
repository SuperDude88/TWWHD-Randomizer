
#pragma once

#include "Requirements.hpp"
#include "HintRegion.hpp"
#include "Area.hpp"
#include <unordered_set>

enum struct EntranceType
{
    NONE = 0,
    DUNGEON,
    DUNGEON_REVERSE,
    CAVE,
    CAVE_REVERSE,
    DOOR,
    DOOR_REVERSE,
    MISC,
    MISC_REVERSE,
    MISC_RESTRICTIVE,
    MISC_RESTRICTIVE_REVERSE,
    MISC_CRAWLSPACE,
    MISC_CRAWLSPACE_REVERSE,
    MIXED,
    ALL,
};

class Entrance;
class World;

class Entrance
{
public:

    Entrance();
    Entrance(const Area& parentArea_, const Area& connectedArea_, World* world_);

    Area getParentArea() const;
    void setParentArea(Area newParentArea);
    Area getConnectedArea() const;
    void setConnectedArea(Area newConnectedArea);
    Requirement& getRequirement();
    void setRequirement(const Requirement newRequirement);
    EntranceType getEntranceType() const;
    void setEntranceType(EntranceType& newType);
    bool isPrimary() const;
    void setAsPrimary();
    std::string getOriginalName() const;
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
    std::string getBossFilepathStageName() const;
    void setBossFilepathStageName(std::string newBossOutStageName);
    std::string getBossOutStageName() const;
    void setBossOutStageName(std::string newBossOutStageName);
    uint8_t getBossOutRoomNum() const;
    void setBossOutRoomNum(uint8_t& newBossOutRoomNum);
    uint8_t getBossOutSpawnId() const;
    void setBossOutSpawnId(uint8_t& newBossOutSpawnId);
    int getWorldId() const;
    void setWorldId(int& newWorldId);
    Entrance* getReverse();
    void setReverse(Entrance* reverseEntrance);
    Entrance* getReplaces();
    void setReplaces(Entrance* replacementEntrance);
    Entrance* getAssumed();
    bool isShuffled() const;
    void setAsShuffled();
    World* getWorld();
    void setWorld(World* newWorld);
    std::unordered_set<HintRegion> getIslands();

    void connect(const Area newConnectedArea);
    Area disconnect();
    void bindTwoWay(Entrance* otherEntrance);
    Entrance* getNewTarget();
    Entrance* assumeReachable();

    bool operator<(const Entrance& rhs) const;

private:

    Area parentArea = Area::INVALID;
    Area connectedArea = Area::INVALID;
    Requirement requirement;
    EntranceType type = EntranceType::NONE;
    bool primary = false;
    std::string originalName = "";
    bool alreadySetOriginalName = false;
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
    int worldId = -1;
    Entrance* reverse = nullptr;
    Entrance* replaces = nullptr;
    Entrance* assumed = nullptr;
    bool shuffled = false;
    World* world = nullptr;
};

std::string entranceTypeToName(const EntranceType& type);
