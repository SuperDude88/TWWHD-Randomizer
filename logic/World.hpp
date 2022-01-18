
#pragma once

#include <string>
#include <sstream>
#include <unordered_set>
#include <list>
#include "Requirements.hpp"
#include "Area.hpp"
#include "Location.hpp"
#include "ItemPool.hpp"

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

    using LocationIndex = uint32_t;
    constexpr static uint32_t LOCATION_COUNT = static_cast<std::underlying_type_t<Location>>(Location::COUNT);
    constexpr static uint32_t AREA_COUNT = static_cast<std::underlying_type_t<Area>>(Area::COUNT);

    World();

    World copy();
    void setSettings(Settings& settings);
    void setItemPool();
    WorldLoadingError loadMacros(const std::vector<json>& macroObjectList);
    WorldLoadingError loadArea(const json& areaObject, Area& loadedArea);
    bool isAccessible(Location location, const ItemPool& ownedItems, const Settings& settings);
    void getAccessibleLocations(const ItemPool& ownedItems, const Settings& settings, std::vector<Location>& accessibleLocations, std::vector<Location>& allowedLocations);
    std::vector<Location> assumedSearch(ItemPool& ownedItems,
                                        const Settings& settings,
                                        std::vector<Location>& allowedLocations);
    void assumedFill(const ItemPool& items, std::vector<Location>& locationsToFill, const Settings& settings);
    void fastFill(const ItemPool& itemsToPlace, const std::vector<Location>& allowedLocations);
    static const char* errorToName(WorldLoadingError err);

    std::string getLastErrorDetails();

    void dumpWorldGraph(const std::string& filename);

    int worldId = 0;

private:

    struct Exit
    {
        Area parentArea = Area::INVALID;
        Area connectedArea = Area::INVALID;
        Requirement requirement;
    };

    struct AreaEntry
    {
        Area area = Area::INVALID;
        std::unordered_set<Location> locations;
        std::list<Exit> exits;
    };

    struct LocationEntry
    {
        Location location = Location::INVALID;
        std::unordered_set<LocationCategory> categories;
        GameItem originalItem = GameItem::INVALID;
        GameItem currentItem = GameItem::INVALID;
        Requirement requirement;
        LocationModificationMethod method;
    };

    constexpr static uint32_t areaAsIndex(Area area) {
        return static_cast<std::underlying_type_t<Area>>(area);
    }
    static Area indexAsArea(uint32_t index) {
        if (index >= AREA_COUNT) return Area::INVALID;
        return static_cast<Area>(index);
    }

    constexpr static uint32_t locationAsIndex(Location loc) {
        return static_cast<std::underlying_type_t<Location>>(loc);
    }
    static Location indexAsLocation(uint32_t index) {
        if (index >= LOCATION_COUNT) return Location::INVALID;
        return static_cast<Location>(index);
    }

    WorldLoadingError parseElement(RequirementType type, const std::vector<json>& args, std::vector<Requirement::Argument>& out);
    WorldLoadingError parseRequirement(const json& requirementsObject, Requirement& out);
    WorldLoadingError parseMacro(const json& macroObject, Requirement& reqOut);
    WorldLoadingError loadExit(const json& exitObject, Exit& loadedExit, Area& parentArea);
    WorldLoadingError loadLocation(const json& locationObject, Location& loadedLocation);
    bool evaluateRequirement(const Requirement& req, const ItemPool& ownedItems, const Settings& settings);

    std::stringstream lastError;
    // invalid game item indicates unitialized
    std::array<AreaEntry, AREA_COUNT> areaEntries;
    std::array<LocationEntry, LOCATION_COUNT> locationEntries;
    std::vector<Requirement> macros;
    std::unordered_map<std::string, MacroIndex> macroNameMap;
    Settings settings;
    ItemPool itemPool;
};
