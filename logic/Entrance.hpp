
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
    std::string getStageName() const;
    void setStageName(std::string newStageName);
    uint8_t getRoomNum() const;
    void setRoomNum(uint8_t& newRoomNum);
    uint8_t getSclsExitIndex() const;
    void setSclsExitIndex(uint8_t& newExitIndex);
    uint8_t getSpawnId() const;
    void setSpawnId(uint8_t& newSpawnId);
    std::string getWarpOutStageName() const;
    void setWarpOutStageName(std::string newWarpOutStageName);
    uint8_t getWarpOutRoomNum() const;
    void setWarpOutRoomNum(uint8_t& newWarpOutRoomNum);
    uint8_t getWarpOutSpawnId() const;
    void setWarpOutSpawnId(uint8_t& newWarpOutSpawnId);
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


private:

    Area parentArea = Area::INVALID;
    Area connectedArea = Area::INVALID;
    Requirement requirement;
    EntranceType type = EntranceType::NONE;
    bool primary = false;
    std::string originalName = "";
    bool alreadySetOriginalName = false;
    std::string stageName = "";
    uint8_t roomNum = -1;
    uint8_t sclsExitIndex = -1;
    uint8_t spawnId = -1;
    std::string warpOutStageName = "";
    uint8_t warpOutRoomNum = -1;
    uint8_t warpOutSpawnId = -1;
    int worldId = -1;
    Entrance* reverse = nullptr;
    Entrance* replaces = nullptr;
    Entrance* assumed = nullptr;
    bool shuffled = false;
    World* world = nullptr;
};

std::string entranceTypeToName(const EntranceType& type);
