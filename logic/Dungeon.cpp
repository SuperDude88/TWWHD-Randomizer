
#include "Dungeon.hpp"
#include <unordered_map>

// The last location name in each list of dungeon locations is the initial race mode location
static const Dungeon DragonRoostCavern = {4, GameItem::DRCSmallKey, GameItem::DRCBigKey, GameItem::DRCDungeonMap, GameItem::DRCCompass, {
            "Dragon Roost Cavern - First Room Chest",
            "Dragon Roost Cavern - Water Jug Alcove Chest",
            "Dragon Roost Cavern - Boarded Up Chest",
            "Dragon Roost Cavern - Swing Across Lava Chest",
            "Dragon Roost Cavern - Rat Room Chest",
            "Dragon Roost Cavern - Rat Room Boarded Up Chest",
            "Dragon Roost Cavern - Bird's Nest",
            "Dragon Roost Cavern - Dark Room Chest",
            "Dragon Roost Cavern - Hub Room Tingle Chest",
            "Dragon Roost Cavern - Pot Room Chest",
            "Dragon Roost Cavern - Mini Boss",
            "Dragon Roost Cavern - Under Rope Bridge Chest",
            "Dragon Roost Cavern - Tingle Statue Chest",
            "Dragon Roost Cavern - Big Key Chest",
            "Dragon Roost Cavern - Boss Stairs Right Chest",
            "Dragon Roost Cavern - Boss Stairs Left Chest",
            "Dragon Roost Cavern - Gohma Heart Container",
        }
};
static const Dungeon ForbiddenWoods = {1, GameItem::FWSmallKey, GameItem::FWBigKey, GameItem::FWDungeonMap, GameItem::FWCompass, {
            "Forbidden Woods - First Room Chest",
            "Forbidden Woods - Inside Hollow Tree Chest",
            "Forbidden Woods - Boko Baba Climb Chest",
            "Forbidden Woods - Hole In Tree Chest",
            "Forbidden Woods - Morth Pit Chest",
            "Forbidden Woods - Vine Maze Left Chest",
            "Forbidden Woods - Vine Maze Right Chest",
            "Forbidden Woods - Tall Room Chest",
            "Forbidden Woods - Mothula Mini Boss Chest",
            "Forbidden Woods - Past Seeds Hanging by Vines Chest",
            "Forbidden Woods - Chest Across Hanging Flower",
            "Forbidden Woods - Tingle Statue Chest",
            "Forbidden Woods - Locked Tree Trunk Chest",
            "Forbidden Woods - Big Key Chest",
            "Forbidden Woods - Double Mothula Room Chest",
            "Forbidden Woods - Kalle Demos Heart Container",
        }, {     // Outside dungeon dependencies
            "Mailbox - Letter from Orca",
        }
};
static const Dungeon TowerOfTheGods = {2, GameItem::TotGSmallKey, GameItem::TotGBigKey, GameItem::TotGDungeonMap, GameItem::TotGCompass, {
            "Tower of the Gods - Chest Behind Bombable Wall",
            "Tower of the Gods - Hop Across Floating Boxes Chest",
            "Tower of the Gods - Light Two Torches Chest",
            "Tower of the Gods - Skull Room Chest",
            "Tower of the Gods - Shoot Eye Above Skulls Chest",
            "Tower of the Gods - Tingle Statue Chest",
            "Tower of the Gods - First Armos Knights Chest",
            "Tower of the Gods - Stone Tablet",
            "Tower of the Gods - Darknut Mini Boss",
            "Tower of the Gods - Second Armos Knights Chest",
            "Tower of the Gods - Floating Platforms Room Lower Chest",
            "Tower of the Gods - Floating Platforms Room Upper Chest",
            "Tower of the Gods - Big Key Chest",
            "Tower of the Gods - Gohdan Heart Container",
        }
};
static const Dungeon ForsakenFortress = {0, GameItem::INVALID, GameItem::INVALID, GameItem::FFDungeonMap, GameItem::FFCompass, {
            "Forsaken Fortress - Phantom Ganon",
            "Forsaken Fortress - Chest Outside Upper Jail Cell",
            "Forsaken Fortress - Chest Inside Lower Jail Cell",
            "Forsaken Fortress - Chest Guarded by Bokoblin",
            "Forsaken Fortress - Chest on Bed",
            "Forsaken Fortress - Helmaroc King Heart Container",
        }, {     // Outside dungeon dependencies
            "Mailbox - Letter from Aryll",
            "Mailbox - Letter from Tingle",
        }
};
static const Dungeon EarthTemple = {3, GameItem::ETSmallKey,   GameItem::ETBigKey,   GameItem::ETDungeonMap,   GameItem::ETCompass, {
            "Earth Temple - Warp Pot Room Chest",
            "Earth Temple - Warp Pot Room Behind Curtain",
            "Earth Temple - First Crypt Chest",
            "Earth Temple - Chest Behind Destructable Wall",
            "Earth Temple - Three Blocks Room Chest",
            "Earth Temple - Behind Statues Chest",
            "Earth Temple - Second Crypt Casket",
            "Earth Temple - Stalfos Mini Boss",
            "Earth Temple - Tingle Statue Chest",
            "Earth Temple - Foggy Floormaster Room End Chest",
            "Earth Temple - Kill All Floormasters Chest",
            "Earth Temple - Near Hammer Button Behind Curtain",
            "Earth Temple - Third Crypt Chest",
            "Earth Temple - Many Mirrors Room Right Chest",
            "Earth Temple - Many Mirrors Room Left Chest",
            "Earth Temple - Stalfos Crypt Room Chest",
            "Earth Temple - Big Key Chest",
            "Earth Temple - Jalhalla Heart Container",
        }, {     // Outside dungeon dependencies
            "Mailbox - Letter from Baito",
        }
};
static const Dungeon WindTemple = {2, GameItem::WTSmallKey, GameItem::WTBigKey, GameItem::WTDungeonMap, GameItem::WTCompass, {
            "Wind Temple - Between Dirt Patches Chest",
            "Wind Temple - Tingle Statue Chest",
            "Wind Temple - Behind Stone Head Chest",
            "Wind Temple - Left Alcove Chest",
            "Wind Temple - Big Key Chest",
            "Wind Temple - Cyclones Room Chest",
            "Wind Temple - Hub Room Center Chest",
            "Wind Temple - Spike Wall Room First Chest",
            "Wind Temple - Spike Wall Room Destroy Floors",
            "Wind Temple - Wizzrobe Mini Boss",
            "Wind Temple - Hub Room Top Chest",
            "Wind Temple - Behind Armos Chest",
            "Wind Temple - Kill All Basement Room Enemies",
            "Wind Temple - Molgera Heart Container",
        }
};
static const Dungeon InvalidDungeon = {0, GameItem::INVALID, GameItem::INVALID, GameItem::INVALID, GameItem::INVALID, {}};

const std::array<DungeonId, 6> getDungeonList()
{
    return {
        DungeonId::DragonRoostCavern,
        DungeonId::ForbiddenWoods,
        DungeonId::TowerOfTheGods,
        DungeonId::ForsakenFortress,
        DungeonId::EarthTemple,
        DungeonId::WindTemple,
    };
}

const Dungeon nameToDungeon(const std::string& name)
{
    static std::unordered_map<std::string, const Dungeon> nameDungeonMap = {
        {"DragonRoostCavern", DragonRoostCavern},
        {"ForbiddenWoods", ForbiddenWoods},
        {"TowerOfTheGods", TowerOfTheGods},
        {"ForsakenFortress", ForsakenFortress},
        {"EarthTemple", EarthTemple},
        {"WindTemple", WindTemple},
    };

    if (nameDungeonMap.count(name) == 0)
    {
        return InvalidDungeon;
    }

    return nameDungeonMap.at(name);
}

std::string dungeonIdToName(const DungeonId& dungeonId)
{
    static std::unordered_map<DungeonId, std::string> dungeonIdNameMap = {
        {DungeonId::DragonRoostCavern, "DragonRoostCavern"},
        {DungeonId::ForbiddenWoods, "ForbiddenWoods"},
        {DungeonId::TowerOfTheGods, "TowerOfTheGods"},
        {DungeonId::ForsakenFortress, "ForsakenFortress"},
        {DungeonId::EarthTemple, "EarthTemple"},
        {DungeonId::WindTemple, "WindTemple"},
    };

    if (dungeonIdNameMap.count(dungeonId) == 0)
    {
        return "INVALID DUNGEON";
    }

    return dungeonIdNameMap.at(dungeonId);
}

DungeonId nameToDungeonId(const std::string& name)
{
    static std::unordered_map<std::string, DungeonId> nameDungeonIdMap = {
        {"DragonRoostCavern", DungeonId::DragonRoostCavern},
        {"ForbiddenWoods", DungeonId::ForbiddenWoods},
        {"TowerOfTheGods", DungeonId::TowerOfTheGods},
        {"ForsakenFortress", DungeonId::ForsakenFortress},
        {"EarthTemple", DungeonId::EarthTemple},
        {"WindTemple", DungeonId::WindTemple},
    };

    if (nameDungeonIdMap.count(name) == 0)
    {
        return DungeonId::INVALID;
    }

    return nameDungeonIdMap.at(name);
}

std::string dungeonIdToFirstRoom(const DungeonId& dungeonId)
{
    static std::unordered_map<DungeonId, std::string> dungeonIdAreaMap = {
        {DungeonId::DragonRoostCavern, "DRC First Room"},
        {DungeonId::ForbiddenWoods, "FW First Room"},
        {DungeonId::TowerOfTheGods, "TOTG Entrance Room"},
        {DungeonId::ForsakenFortress, "Forsaken Fortress Inner Courtyard"},
        {DungeonId::EarthTemple, "ET First Room"},
        {DungeonId::WindTemple, "WT First Room"},
    };

    if (dungeonIdAreaMap.count(dungeonId) == 0)
    {
        return "INVALID";
    }

    return dungeonIdAreaMap.at(dungeonId);
}

const Dungeon dungeonIdToDungeon(const DungeonId& dungeonId)
{
    return nameToDungeon(dungeonIdToName(dungeonId));
}

DungeonId dungeonItemToDungeon(const GameItem& item) {
    static std::unordered_map<GameItem, DungeonId> itemDungeonMap = {
        {GameItem::DRCDungeonMap, DungeonId::DragonRoostCavern},
        {GameItem::DRCCompass, DungeonId::DragonRoostCavern},
        {GameItem::DRCSmallKey, DungeonId::DragonRoostCavern},
        {GameItem::DRCBigKey, DungeonId::DragonRoostCavern},
        {GameItem::FWDungeonMap, DungeonId::ForbiddenWoods},
        {GameItem::FWCompass, DungeonId::ForbiddenWoods},
        {GameItem::FWSmallKey, DungeonId::ForbiddenWoods},
        {GameItem::FWBigKey, DungeonId::ForbiddenWoods},
        {GameItem::TotGDungeonMap, DungeonId::TowerOfTheGods},
        {GameItem::TotGCompass, DungeonId::TowerOfTheGods},
        {GameItem::TotGSmallKey, DungeonId::TowerOfTheGods},
        {GameItem::TotGBigKey, DungeonId::TowerOfTheGods},
        {GameItem::FFDungeonMap, DungeonId::ForsakenFortress},
        {GameItem::FFCompass, DungeonId::ForsakenFortress},
        {GameItem::ETDungeonMap, DungeonId::EarthTemple},
        {GameItem::ETCompass, DungeonId::EarthTemple},
        {GameItem::ETSmallKey, DungeonId::EarthTemple},
        {GameItem::ETBigKey, DungeonId::EarthTemple},
        {GameItem::WTDungeonMap, DungeonId::WindTemple},
        {GameItem::WTCompass, DungeonId::WindTemple},
        {GameItem::WTSmallKey, DungeonId::WindTemple},
        {GameItem::WTBigKey, DungeonId::WindTemple},
    };

    if (itemDungeonMap.count(item) == 0)
    {
        return DungeonId::INVALID;
    }

    return itemDungeonMap.at(item);
}
