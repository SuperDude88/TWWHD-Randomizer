#include "ItemPool.hpp"
#include "PoolFunctions.hpp"
#include "World.hpp"
#include "../seedgen/random.hpp"
#include "../server/command/Log.hpp"

static const std::unordered_map<std::string, uint8_t> alwaysItems = {
    // Potentially progress items
    {"Wind Waker", 1},
    {"Spoils Bag", 1},
    {"Grappling Hook", 1},
    {"Power Bracelets", 1},
    {"Iron Boots", 1},
    {"Bait Bag", 1},
    {"Boomerang", 1},
    {"Hookshot", 1},
    {"Delivery Bag", 1},
    {"Bombs", 1},
    {"Skull Hammer", 1},
    {"Deku Leaf", 1},

    {"Triforce Shard 1", 1},
    {"Triforce Shard 2", 1},
    {"Triforce Shard 3", 1},
    {"Triforce Shard 4", 1},
    {"Triforce Shard 5", 1},
    {"Triforce Shard 6", 1},
    {"Triforce Shard 7", 1},
    {"Triforce Shard 8", 1},

    {"Nayru's Pearl", 1},
    {"Din's Pearl", 1},
    {"Farore's Pearl", 1},

    {"Wind's Requiem", 1},
    {"Song of Passing", 1},
    {"Ballad of Gales", 1},
    {"Command Melody", 1},
    {"Earth God's Lyric", 1},
    {"Wind God's Aria", 1},

    {"Note to Mom", 1},
    {"Maggie's Letter", 1},
    {"Moblin's Letter", 1},
    {"Cabana Deed", 1},

    {"Dragon Tingle Statue", 1},
    {"Forbidden Tingle Statue", 1},
    {"Goddess Tingle Statue", 1},
    {"Earth Tingle Statue", 1},
    {"Wind Tingle Statue", 1},

    {"Progressive Sail", 2},

    {"Progressive Bomb Bag", 2},
    {"Progressive Quiver", 2},
    {"Progressive Magic Meter", 2},

    {"Progressive Shield", 2},
    {"Progressive Bow", 3},
    {"Progressive Wallet", 2},
    {"Progressive Picto Box", 2},
    {"Empty Bottle", 4},

    {"Ghost Ship Chart", 1},

    {"Treasure Chart 1", 1},
    {"Treasure Chart 2", 1},
    {"Treasure Chart 3", 1},
    {"Treasure Chart 4", 1},
    {"Treasure Chart 5", 1},
    {"Treasure Chart 6", 1},
    {"Treasure Chart 7", 1},
    {"Treasure Chart 8", 1},
    {"Treasure Chart 9", 1},
    {"Treasure Chart 10", 1},
    {"Treasure Chart 11", 1},
    {"Treasure Chart 12", 1},
    {"Treasure Chart 13", 1},
    {"Treasure Chart 14", 1},
    {"Treasure Chart 15", 1},
    {"Treasure Chart 16", 1},
    {"Treasure Chart 17", 1},
    {"Treasure Chart 18", 1},
    {"Treasure Chart 19", 1},
    {"Treasure Chart 20", 1},
    {"Treasure Chart 21", 1},
    {"Treasure Chart 22", 1},
    {"Treasure Chart 23", 1},
    {"Treasure Chart 24", 1},
    {"Treasure Chart 25", 1},
    {"Treasure Chart 26", 1},
    {"Treasure Chart 27", 1},
    {"Treasure Chart 28", 1},
    {"Treasure Chart 29", 1},
    {"Treasure Chart 30", 1},
    {"Treasure Chart 31", 1},
    {"Treasure Chart 32", 1},
    {"Treasure Chart 33", 1},
    {"Treasure Chart 34", 1},
    {"Treasure Chart 35", 1},
    {"Treasure Chart 36", 1},
    {"Treasure Chart 37", 1},
    {"Treasure Chart 38", 1},
    {"Treasure Chart 39", 1},
    {"Treasure Chart 40", 1},
    {"Treasure Chart 41", 1},
    {"Treasure Chart 42", 1},
    {"Treasure Chart 43", 1},
    {"Treasure Chart 44", 1},
    {"Treasure Chart 45", 1},
    {"Treasure Chart 46", 1},
    {"Triforce Chart 1", 1},
    {"Triforce Chart 2", 1},
    {"Triforce Chart 3", 1},

    // Non-consumable junk items
    {"Telescope", 1},
    {"Magic Armor", 1},
    {"Hero's Charm", 1},
    {"Fill Up Coupon", 1},

    {"Hurricane Spin", 1},

    {"Submarine Chart", 1},
    {"Beedle's Chart", 1},
    {"Platform Chart", 1},
    {"Light Ring Chart", 1},
    {"Secret Cave Chart", 1},
    {"Great Fairy Chart", 1},
    {"Octo Chart", 1},
    {"Tingle's Chart", 1},

    // Consumable junk items
    {"Green Rupee", 1},
    {"Blue Rupee", 2},
    {"Yellow Rupee", 3},
    {"Red Rupee", 5},
    {"Purple Rupee", 10},
    {"Orange Rupee", 15},
    {"Silver Rupee", 15},
    {"Rainbow Rupee", 1},
    {"Joy Pendant", 9},
    {"Skull Necklace", 9},
    {"Boko Baba Seed", 1},
    {"Golden Feather", 9},
    {"Knights Crest", 3},
    {"Red Chu Jelly", 1},
    {"Green Chu Jelly", 1},
    {"All Purpose Bait", 1},
    {"Hyoi Pear", 4},
};

// The distribution of elements in this pool is to give some items
// a higher chance of being randomly selected than others
static const GameItemPool junkPool = {
    GameItem::YellowRupee,  // 3 Yellow Rupees
    GameItem::YellowRupee,
    GameItem::YellowRupee,
    GameItem::RedRupee,     // 7 Red Rupees
    GameItem::RedRupee,
    GameItem::RedRupee,
    GameItem::RedRupee,
    GameItem::RedRupee,
    GameItem::RedRupee,
    GameItem::RedRupee,
    GameItem::PurpleRupee,  // 10 Purple Rupees
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::OrangeRupee,  // 15 Orange Rupees
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::JoyPendant,   // 3 Joy Pendants
    GameItem::JoyPendant,
    GameItem::JoyPendant,
};

std::string getRandomJunk()
{
    return gameItemToName(RandomElement(junkPool));
}

ItemNamePool generateGameItemPool(const Settings& settings, World* world)
{
    // Add items which will always be in the item pool
    ItemNamePool completeItemPool = {};
    for (const auto& [name, count] : alwaysItems)
    {
        addElementToPool(completeItemPool, name, count);
    }

    // Add dungeon items
    for (auto& [name, dungeon] : world->dungeons)
    {
        if (dungeon.smallKey != "")
        {
            addElementToPool(completeItemPool, dungeon.smallKey, dungeon.keyCount);
        }
        if (dungeon.bigKey != "")
        {
            addElementToPool(completeItemPool, dungeon.bigKey);
        }
        addElementToPool(completeItemPool, dungeon.map);
        addElementToPool(completeItemPool, dungeon.compass);
    }

    // Add swords to the pool if we aren't playing in swordless mode
    if (settings.sword_mode != SwordMode::NoSword)
    {
        addElementToPool(completeItemPool, std::string("Progressive Sword"), 4);
    }

    // Add appropriate numbers of heart containers and heart pieces
    int numContainers = 6 - settings.starting_hcs;
    int numPieces = 44 - settings.starting_pohs;
    addElementToPool(completeItemPool, std::string("Heart Container"), numContainers);
    addElementToPool(completeItemPool, std::string("Piece of Heart"), numPieces);

    return completeItemPool;
}

ItemNamePool generateStartingGameItemPool(const Settings& settings)
{
    //Should be able to randomize wind waker/sail but it would require some logic changes/fixes which aren't in yet
    ItemNamePool startingItems = {
        "Wind Waker",
        "Wind's Requiem",
        "Progressive Sail"
    };

    // Add in a sword, if the sword mode starts with one
    if (settings.sword_mode == SwordMode::StartWithSword)
    {
        startingItems.push_back("Progressive Sword");
    }

    for (auto& item : settings.starting_gear)
    {
        startingItems.push_back(gameItemToName(item));
    }

    return startingItems;
}

void logItemPool(const std::string& poolName, const ItemPool& itemPool)
{
    #ifdef ENABLE_DEBUG
        LOG_TO_DEBUG(poolName + ":");
        for (auto& item : itemPool) {
            LOG_TO_DEBUG("\t" + item.getName());
        }
        LOG_TO_DEBUG("]");
    #endif
}
