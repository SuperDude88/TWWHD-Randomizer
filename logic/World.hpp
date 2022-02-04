
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

static std::stringstream lastError;

class World;
using WorldPool = std::vector<World>;
using LocationPool = std::vector<Location*>;


struct Exit
{
    Area parentArea = Area::INVALID;
    Area connectedArea = Area::INVALID;
    Requirement requirement;
    int worldId = -1;
};

struct AreaEntry
{
    Area area = Area::INVALID;
    std::list<Location*> locations;
    std::list<Exit> exits;
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
        REQUIREMENT_MISISNG_KEY,
        INVALID_LOCATION_CATEGORY,
        INVALID_MODIFICATION_TYPE,
        INVALID_OFFSET_VALUE,
        INVALID_GAME_ITEM
    };

    World();

    void setSettings(const Settings& settings);
    const Settings& getSettings() const;
    void setWorldId(int newWorldId);
    int getWorldId() const;
    void setItemPools();
    ItemPool getItemPool() const;
    void addToItemPool(const GameItem gameItem);
    ItemPool getStartingItems() const;
    LocationPool getLocations();
    void determineChartMappings();
    void determineProgressionLocations();
    void determineRaceModeDungeons();
    int loadWorld(const std::string& worldFilePath, const std::string& macrosFilePath);
    static const char* errorToName(WorldLoadingError err);
    std::string getLastErrorDetails();
    void dumpWorldGraph(const std::string& filename);

    std::vector<AreaEntry> areaEntries = {};
    std::vector<Location> locationEntries = {};
    std::unordered_map<std::string, MacroIndex> macroNameMap;
    std::vector<Requirement> macros;
    std::vector<std::list<Location*>> playthroughSpheres = {};
    std::array<GameItem, 49> chartMappings;
    std::vector<DungeonId> raceModeDungeons;

private:

    bool chartLeadsToSunkenTreasure(const Location& location, const std::string& itemPrefix);

    WorldLoadingError parseElement(RequirementType type, const std::vector<json>& args, std::vector<Requirement::Argument>& out);
    WorldLoadingError parseRequirement(const json& requirementsObject, Requirement& out);
    WorldLoadingError parseMacro(const json& macroObject, Requirement& reqOut);
    WorldLoadingError loadExit(const json& exitObject, Exit& loadedExit, Area& parentArea);
    WorldLoadingError loadLocation(const json& locationObject, LocationId& loadedLocation);
    WorldLoadingError loadMacros(const std::vector<json>& macroObjectList);
    WorldLoadingError loadArea(const json& areaObject, Area& loadedArea);

    Settings settings;
    ItemPool itemPool;
    ItemPool startingItems;
    int worldId = -1;
};
