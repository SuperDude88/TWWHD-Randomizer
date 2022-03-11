
#pragma once

#include "Requirements.hpp"
#include "Area.hpp"

enum struct EntranceType
{
    NONE = 0,
    DUNGEON,
    CAVE,
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
    std::string getStageName() const;
    void setStageName(std::string newStageName);
    std::string getOriginalName() const;
    void setOriginalName();
    std::string getCurrentName() const;
    uint8_t getRoomNum() const;
    void setRoomNum(uint8_t& newRoomNum);
    uint8_t getSclsExitIndex() const;
    void setSclsExitIndex(uint8_t& newExitIndex);
    uint8_t getSpawnId() const;
    void setSpawnId(uint8_t& newSpawnId);
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
    std::string stageName = "";
    std::string originalName = "";
    bool alreadySetOriginalName = false;
    uint8_t roomNum = -1;
    uint8_t sclsExitIndex = -1;
    uint8_t spawnId = -1;
    int worldId = -1;
    Entrance* reverse = nullptr;
    Entrance* replaces = nullptr;
    Entrance* assumed = nullptr;
    bool shuffled = false;
    World* world = nullptr;
};

std::string entranceTypeToName(const EntranceType& type);
