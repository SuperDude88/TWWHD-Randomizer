
#pragma once

#include <string>
#include <sstream>
#include <unordered_set>
#include <list>
#include "Requirements.hpp"
#include "Area.hpp"
#include "Location.hpp"
#include "ItemPool.hpp"

static std::stringstream lastError;

struct LocationEntry
{
    Location locationId = Location::INVALID;
    std::unordered_set<LocationCategory> categories;
    Item originalItem = {GameItem::INVALID, -1};
    Item currentItem = {GameItem::INVALID, -1};
    Requirement requirement;
    LocationModificationMethod method;
    int worldId = -1;

    // variables used for the searching algorithm
    bool hasBeenTried = false;
};

struct Exit
{
    Area parentArea = Area::INVALID;
    Area connectedArea = Area::INVALID;
    Requirement requirement;
    int worldId = -1;

    // variables used for the searching algorithm
    bool hasBeenTried = false;
};

struct AreaEntry
{
    Area area = Area::INVALID;
    std::list<LocationEntry*> locations;
    std::list<Exit> exits;
    int worldId = -1;

    // variables used for the searching algorithm
    bool isAccessible = false;
};

using LocationPool = std::vector<LocationEntry*>;
using LocationIndex = uint32_t;

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
        SETTING_DOES_NOT_EXIST,
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

    void setSettings(Settings& settings);
    const Settings& getSettings() const;
    void setWorldId(int newWorldId);
    int getWorldId() const;
    void setItemPools();
    ItemPool getItemPool() const;
    ItemPool getStartingItems() const;
    LocationPool getLocations() const;
    WorldLoadingError loadMacros(const std::vector<json>& macroObjectList);
    WorldLoadingError loadArea(const json& areaObject, Area& loadedArea);
    bool isAccessible(Location location, const GameItemPool& ownedItems, const Settings& settings);
    void getAccessibleLocations(const GameItemPool& ownedItems, const Settings& settings, std::vector<Location>& accessibleLocations, std::vector<Location>& allowedLocations);
    std::vector<Location> assumedSearch(GameItemPool& ownedItems,
                                        const Settings& settings,
                                        std::vector<Location>& allowedLocations);
    void assumedFill(const GameItemPool& items, std::vector<Location>& locationsToFill, const Settings& settings);
    void fastFill(const GameItemPool& itemsToPlace, const std::vector<Location>& allowedLocations);
    static const char* errorToName(WorldLoadingError err);
    std::string getLastErrorDetails();
    void dumpWorldGraph(const std::string& filename);

    // invalid game item indicates unitialized
    std::vector<AreaEntry> areaEntries = {};
    std::vector<LocationEntry> locationEntries = {};
    std::vector<Requirement> macros;

private:

    WorldLoadingError parseElement(RequirementType type, const std::vector<json>& args, std::vector<Requirement::Argument>& out);
    WorldLoadingError parseRequirement(const json& requirementsObject, Requirement& out);
    WorldLoadingError parseMacro(const json& macroObject, Requirement& reqOut);
    WorldLoadingError loadExit(const json& exitObject, Exit& loadedExit, Area& parentArea);
    WorldLoadingError loadLocation(const json& locationObject, Location& loadedLocation);


    std::unordered_map<std::string, MacroIndex> macroNameMap;
    Settings settings;
    ItemPool itemPool;
    ItemPool startingItems;
    int worldId = -1;
};
