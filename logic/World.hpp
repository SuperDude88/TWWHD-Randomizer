
#pragma once

#include <string>
#include <sstream>
#include <unordered_set>
#include <list>
#include "Requirements.hpp"
#include "Area.hpp"
#include "Location.hpp"
#include "ItemPool.hpp"
#include "Dungeon.hpp"
#include "Entrance.hpp"
#include "HintRegion.hpp"
#include "../libs/ryml.hpp"

static std::stringstream lastError;

class World;
using WorldPool = std::vector<World>;
using LocationPool = std::vector<Location*>;
using EntrancePool = std::vector<Entrance*>;

struct LocationAccess
{
    Location* location = nullptr;
    Requirement requirement;
};

struct EventAccess
{
    std::string event;
    Requirement requirement;
    int worldId = -1;
};

struct AreaEntry
{
    Area area = Area::INVALID;
    HintRegion island = HintRegion::NONE;
    HintRegion dungeon = HintRegion::NONE;
    std::list<EventAccess> events;
    std::list<LocationAccess> locations;
    std::list<Entrance> exits;
    std::list<Entrance*> entrances;
    int worldId = -1;

    // variables used for the searching algorithm
    bool isAccessible = false;
};

class World
{
public:

    enum struct WorldLoadingError
    {
        NONE = 0,
        DUPLICATE_MACRO_NAME,
        MACRO_DOES_NOT_EXIST,
        REQUIREMENT_TYPE_DOES_NOT_EXIST,
        MAPPING_MISMATCH,
        GAME_ITEM_DOES_NOT_EXIST,
        AREA_DOES_NOT_EXIST,
        LOCATION_DOES_NOT_EXIST,
        EXIT_MISSING_KEY,
        OPTION_DOES_NOT_EXIST,
        INCORRECT_ARG_COUNT,
        EXPECTED_JSON_OBJECT,
        AREA_MISSING_KEY,
        LOCATION_MISSING_KEY,
        MACRO_MISSING_KEY,
        MACRO_MISSING_VAL,
        REQUIREMENT_MISISNG_KEY,
        INVALID_LOCATION_CATEGORY,
        INVALID_MODIFICATION_TYPE,
        INVALID_OFFSET_VALUE,
        INVALID_GAME_ITEM,
        LOGIC_SYMBOL_DOES_NOT_EXIST,
        COULD_NOT_DETERMINE_TYPE,
        SAME_NESTING_LEVEL,
        EXTRA_OR_MISSING_PARENTHESIS,
        PLANDOMIZER_ERROR,
    };

    World();
    World(size_t numWorlds_);

    void setSettings(const Settings& settings);
    const Settings& getSettings() const;
    void setWorldId(int newWorldId);
    int getWorldId() const;
    void setItemPools();
    ItemPool getItemPool() const;
    ItemPool getStartingItems() const;
    LocationPool getLocations(bool onlyProgression = false);
    AreaEntry& getArea(const Area& area);

    void determineChartMappings();
    void determineProgressionLocations();
    WorldLoadingError determineRaceModeDungeons();
    int loadWorld(const std::string& worldFilePath, const std::string& macrosFilePath, const std::string& locationDataPath);
    Entrance& getEntrance(const Area& parentArea, const Area& connectedArea);
    void removeEntrance(Entrance* entranceToRemove);
    EntrancePool getShuffleableEntrances(const EntranceType& type, const bool& onlyPrimary = false);
    EntrancePool getShuffledEntrances(const EntranceType& type, const bool& onlyPrimary = false);
    std::unordered_set<HintRegion> getIslands(const Area& area);
    static const char* errorToName(WorldLoadingError err);
    std::string getLastErrorDetails();
    void dumpWorldGraph(const std::string& filename, bool onlyRandomizedExits = false);

    std::vector<AreaEntry> areaEntries = {};
    std::vector<Location> locationEntries = {};
    std::unordered_map<Location*, Item> plandomizerLocations = {};
    std::unordered_map<std::string, MacroIndex> macroNameMap;
    std::vector<Requirement> macros;
    std::list<std::list<Location*>> playthroughSpheres = {};
    std::list<std::list<Entrance*>> entranceSpheres = {};
    std::array<GameItem, 49> chartMappings;
    std::unordered_map<DungeonId, HintRegion> raceModeDungeons; // map of dungeonId to the island it's in
    uint8_t startingIslandRoomIndex = 44;

private:

    bool chartLeadsToSunkenTreasure(const Location& location, const std::string& itemPrefix);


    WorldLoadingError parseRequirementString( const std::string& str, Requirement& req);
    WorldLoadingError parseMacro(const std::string& macroLogicExpression, Requirement& reqOut);
    WorldLoadingError loadExit(const std::string& connectedAreaName, const std::string& logicExpression, Entrance& loadedExit, Area& parentArea);
    WorldLoadingError loadLocation(const ryml::NodeRef& locationObject, LocationId& loadedLocation);
    WorldLoadingError loadEventRequirement(const std::string& eventName, const std::string& logicExpression, EventAccess& eventAccess);
    WorldLoadingError loadLocationRequirement(const std::string& locationName, const std::string& logicExpression, LocationAccess& loadedLocation);
    WorldLoadingError loadMacros(const ryml::Tree& macroListTree);
    WorldLoadingError loadArea(const ryml::NodeRef& areaObject, Area& loadedArea);
    WorldLoadingError loadPlandomizerLocations();
    int getFileContents(const std::string& filename, std::string& fileContents);

    Settings settings;
    ItemPool itemPool;
    ItemPool startingItems;
    int worldId = -1;
    size_t numWorlds = 1;
};
