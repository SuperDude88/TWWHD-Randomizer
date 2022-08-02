
#include "Hints.hpp"
#include "Search.hpp"
#include "PoolFunctions.hpp"
#include "../seedgen/random.hpp"
#include "../server/command/Log.hpp"
#include "../server/filetypes/util/msbtMacros.hpp"
#include "../server/utility/stringUtil.hpp"
#include <cmath>

template<typename Container>
static Location* getHintableLocation(Container& locations)
{
    for (auto location : locations)
    {
        if (!location->hasBeenHinted)
        {
            return location;
        }
    }
    return nullptr;
}

static HintError calculatePossiblePathLocations(WorldPool& worlds)
{
    LOG_TO_DEBUG("Generating Path Locations");
    // First generate the goal location keys and also remove items from non-
    // progress locations since they shouldn't be considered when determining
    // path locations
    std::unordered_map<Location*, Item> nonRequiredLocations = {};
    for (auto& world : worlds)
    {
        // Defeat Ganondorf is always a goal location
        world.pathLocations[&world.locationEntries["Ganon's Tower - Defeat Ganondorf"]] = {};
        for (auto& [name, dungeon] : world.dungeons)
        {
            // Race mode locations are also goal locations
            if (dungeon.isRaceModeDungeon && world.getSettings().race_mode)
            {
                Location* goalLocation = &world.locationEntries[dungeon.raceModeLocation];
                world.pathLocations[goalLocation] = {};
            }
        }

        for (auto& [name, location] : world.locationEntries)
        {
            if (!location.progression)
            {
                nonRequiredLocations.insert({&location, location.currentItem});
                location.currentItem = {GameItem::INVALID, location.worldId};
            }
        }
    }

    // Determine path locations for each goal location by going through the playthrough
    // and seeing if taking away the item at each location can still access the goal locations
    for (auto& world : worlds)
    {
        for (auto& sphere : world.playthroughSpheres)
        {
            for (auto location : sphere)
            {
                auto itemAtLocation = location->currentItem;
                // If this location has a small or big key and keylunacy isn't on, then ignore it
                // because the player already knows where those items are. Also ignore race mode
                // locations at the end of dungeons because players know those locations are required
                if ((itemAtLocation.isDungeonItem() && !world.getSettings().keylunacy) || (location->isRaceModeLocation && world.getSettings().race_mode))
                {
                    continue;
                }

                // Take the item away from the location
                location->currentItem = Item(GameItem::INVALID, location->worldId);

                // Run a search without the item
                runGeneralSearch(worlds);

                for (auto& [goalLocation, pathLocations] : world.pathLocations)
                {
                    // If we never reached the goal location, then this location
                    // is "on the path to" the goal location. Since hints will refer
                    // to locations in an individual's world, only add locations
                    // which are in the same world as the goal location
                    if (!goalLocation->hasBeenFound && goalLocation->worldId == location->worldId)
                    {
                        pathLocations.push_back(location);
                    }
                }

                // Then give back the location's item
                location->currentItem = itemAtLocation;
            }
        }
    }

    // Give back nonprogress location items
    for (auto& [location, item] : nonRequiredLocations)
    {
        location->currentItem = item;
    }

    return HintError::NONE;
}

static HintError calculatePossibleBarrenRegions(WorldPool& worlds)
{
    LOG_TO_DEBUG("Calculating Barren Regions");
    for (auto& world : worlds)
    {
        std::unordered_set<Item*> potentiallyJunkItems = {};
        for (auto& [areaName, area] : world.areaEntries)
        {
            // We'll be performing several operations during this loop
            for (auto& locAccess : area.locations)
            {
                auto location = locAccess.location;
                // Assign the location its hint regions if it doesn't already
                // have them
                if (location->hintRegions.empty())
                {
                    location->hintRegions = world.getIslands(areaName);
                }
                // If this location is progression, then add its hint regions to
                // the set of potentially barren regions
                if (location->progression && !location->hintRegions.empty())
                {
                    world.barrenRegions[*location->hintRegions.begin()] = {};
                }

                // During this loop we'll also go through and mark certain progressive items as junk items. These
                // are "chain" items whose sole purpose is to unlock another check. Here they will
                // be marked as junk if the location they unlock contains junk. If the location they unlock
                // is not junk, then we'll put it into the potentially junk items set to check for later
                auto& chainLocations = location->currentItem.getChainLocations();
                if (location->progression && !chainLocations.empty())
                {
                    // If all of this item's chain locations are junk, then this item is also junk
                    if (std::all_of(chainLocations.begin(), chainLocations.end(), [](Location* loc){return loc->currentItem.isJunkItem();}))
                    {
                        location->currentItem.setAsJunkItem();
                        LOG_TO_DEBUG(location->currentItem.getName() + " is now junk.");
                    }
                    else
                    {
                        potentiallyJunkItems.insert(&location->currentItem);
                    }
                }
            }
        }
        #ifdef ENABLE_DEBUG
            LOG_TO_DEBUG("Potentially Barren Regions: [");
            for (auto& [hintRegion, locations] : world.barrenRegions)
            {
                LOG_TO_DEBUG("\t" + hintRegion);
            }
            LOG_TO_DEBUG("]");
        #endif

        // Loop through the potentially junk items until no new junk items have been
        // found incase the locations of chain items are themselves chained together
        bool newJunkItems = false;
        do
        {
            newJunkItems = false;
            for (auto item : potentiallyJunkItems)
            {
                if (!item->isJunkItem() && std::all_of(item->getChainLocations().begin(), item->getChainLocations().end(), [](Location* loc){return loc->currentItem.isJunkItem();}))
                {
                    newJunkItems = true;
                    item->setAsJunkItem();
                    LOG_TO_DEBUG(item->getName() + " is now junk.");
                }
            }
        }
        while (newJunkItems);

        // Now loop through all the progression locations again and remove any
        // regions from the barren regions map which have non-junk items at any
        // of their locations. Otherwise add the location to the list of locations
        // in the barren region
        for (auto& [name, location] : world.locationEntries)
        {
            for (auto& hintRegion : location.hintRegions)
            {
                if (location.progression && !location.currentItem.isJunkItem())
                {
                    world.barrenRegions.erase(hintRegion);
                }
                else if (world.barrenRegions.count(hintRegion) > 0)
                {
                    world.barrenRegions[hintRegion].insert(&location);
                }
            }
        }

        // If any dungeon is barren, check to make sure all its dungeon dependency locations
        // are also barren. Otherwise, remove it from the list
        for (auto& [dungeonName, dungeon] : world.dungeons)
        {
            auto& outsideLocations = dungeon.outsideDependentLocations;
            if (world.barrenRegions.count(dungeonName) > 0 &&
                std::any_of(outsideLocations.begin(), outsideLocations.end(), [&world](std::string& locationName){return !world.locationEntries[locationName].currentItem.isJunkItem();}))
            {
                world.barrenRegions.erase(dungeonName);
            }
        }

        #ifdef ENABLE_DEBUG
            LOG_TO_DEBUG("Final Barren Regions: [");
            for (auto& [hintRegion, locations] : world.barrenRegions)
            {
                LOG_TO_DEBUG("\t" + hintRegion);
            }
            LOG_TO_DEBUG("]");
        #endif
    }

    return HintError::NONE;
}

static HintError generatePathHintMessage(Location* location, Location* goalLocation)
{
    std::u16string regionText = u"";
    size_t totalRegions = 0;
    for (auto& hintRegion : location->hintRegions)
    {
        // Change the formatting depending on how many regions lead to the location
        if (totalRegions == 0)
        {
            regionText += TEXT_COLOR_CYAN + Utility::Str::toUTF16(hintRegion) + TEXT_COLOR_DEFAULT;
        }
        else if (totalRegions == location->hintRegions.size() - 1 && location->hintRegions.size() == 2)
        {
            regionText += u" and "s + TEXT_COLOR_CYAN + Utility::Str::toUTF16(hintRegion) + TEXT_COLOR_DEFAULT;
        }
        else if (totalRegions == location->hintRegions.size() - 1)
        {
            regionText += u", and "s + TEXT_COLOR_CYAN + Utility::Str::toUTF16(hintRegion) + TEXT_COLOR_DEFAULT;
        }
        else
        {
            regionText += u", "s + TEXT_COLOR_CYAN + Utility::Str::toUTF16(hintRegion) + TEXT_COLOR_DEFAULT;
        }

        totalRegions++;
    }

    std::u16string plurality = totalRegions == 1 ? u" is"s : u" are"s;

    location->hintText = HINT_PREFIX + regionText + plurality + u" on the path to "s + TEXT_COLOR_RED + Utility::Str::toUTF16(goalLocation->goalName) + TEXT_COLOR_DEFAULT + u"."s;
    return HintError::NONE;
}

static HintError generatePathHintLocations(World& world, std::vector<Location*>& pathHintLocations)
{
    // Shuffle each pool of path locations so that its order is random
    std::vector<Location*> goalLocations = {};
    for (auto& [goalLocation, pathLocations] : world.pathLocations)
    {
        shufflePool(pathLocations);
        // Initially we want to pull path hints from race mode dungeons before pulling from Ganondorf
        if (goalLocation->name != "Ganon's Tower - Defeat Ganondorf")
        {
            goalLocations.push_back(goalLocation);
        }
    }

    for (uint8_t i = 0; i < world.getSettings().path_hints; i++)
    {
        if (goalLocations.empty())
        {
            LOG_TO_DEBUG("No more possible path hints");
            break;
        }

        // Try to get at least one hint for each race mode dungeon first
        Location* goalLocation = nullptr;
        if (i < goalLocations.size())
        {
            goalLocation = goalLocations[i];
        }
        else
        {
            // Once we've pulled from all race mode dungeons, then add Ganondorf to the list
            // and choose randomly
            if (i == goalLocations.size())
            {
                goalLocations.push_back(&world.locationEntries["Ganon's Tower - Defeat Ganondorf"]);
            }
            goalLocation = RandomElement(goalLocations);
        }
        auto hintLocation = getHintableLocation(world.pathLocations[goalLocation]);
        if (hintLocation == nullptr)
        {
            LOG_TO_DEBUG("No more path locations for " + goalLocation->name);
            filterAndEraseFromPool(goalLocations, [&goalLocation](Location* goal){return goal == goalLocation;});
            i--;
            continue;
        }
        hintLocation->hasBeenHinted = true;
        pathHintLocations.push_back(hintLocation);
        LOG_TO_DEBUG("Chose " + hintLocation->name + " as path hint for " + goalLocation->name);
        LOG_AND_RETURN_IF_ERR(generatePathHintMessage(hintLocation, goalLocation));
    }

    return HintError::NONE;
}

static HintError generateBarrenHintLocations(World& world, std::vector<Location*>& barrenHintLocations)
{
    std::vector<std::string> barrenPool = {};
    std::vector<double> barrenDistributions = {};
    for (auto& [barrenRegion, barrenLocations] : world.barrenRegions)
    {
        barrenPool.push_back(barrenRegion);
        // The probability of a region being chosen for a barren hint is the square root
        // of how many locations are in that region
        barrenDistributions.push_back(sqrt(barrenLocations.size()));
    }

    std::discrete_distribution<size_t> barrenDistribution(barrenDistributions.begin(), barrenDistributions.end());

    for (uint8_t i = 0; i < world.getSettings().barren_hints; i++)
    {
        auto regionIndex = barrenDistribution(GetGenerator());
        auto& barrenRegion = barrenPool[regionIndex];
        // Set all locations in the selected barren region as hinted at
        for (auto location : world.barrenRegions[barrenRegion])
        {
            location->hasBeenHinted = true;
            location->hintText = HINT_PREFIX u"visiting " + TEXT_COLOR_BLUE + Utility::Str::toUTF16(barrenRegion) + TEXT_COLOR_DEFAULT + u" is a foolish choice.";
        }
        barrenHintLocations.push_back(*world.barrenRegions[barrenRegion].begin());
        LOG_TO_DEBUG("Chose \"" + barrenRegion + "\" as a hinted barren region");

        // Reform the distribution without the region we just chose
        barrenDistributions.erase(barrenDistributions.begin() + regionIndex);
        barrenPool.erase(barrenPool.begin() + regionIndex);
        if (barrenPool.empty())
        {
            LOG_TO_DEBUG("No more barren regions to hint at.");
            break;
        }
        barrenDistribution = std::discrete_distribution<size_t>(barrenDistributions.begin(), barrenDistributions.end());
    }

    return HintError::NONE;
}

static HintError generateItemHintMessage(World& world, Location* location)
{
    std::u16string regionText = u"";
    size_t totalRegions = 0;
    for (auto hintRegion : location->hintRegions)
    {
        // If this is an item in a dungeon, use the dungeon's island for the hint instead
        if (world.dungeons.count(hintRegion) > 0)
        {
            hintRegion = world.dungeons[hintRegion].island;
        }
        // Change the formatting depending on how many regions lead to the location
        if (totalRegions == 0)
        {
            regionText += TEXT_COLOR_RED + Utility::Str::toUTF16(hintRegion) + TEXT_COLOR_DEFAULT;
        }
        else if (totalRegions == location->hintRegions.size() - 1 && location->hintRegions.size() == 2)
        {
            regionText += u" and "s + TEXT_COLOR_RED + Utility::Str::toUTF16(hintRegion) + TEXT_COLOR_DEFAULT;
        }
        else if (totalRegions == location->hintRegions.size() - 1)
        {
            regionText += u", and "s + TEXT_COLOR_RED + Utility::Str::toUTF16(hintRegion) + TEXT_COLOR_DEFAULT;
        }
        else
        {
            regionText += u", "s + TEXT_COLOR_CYAN + Utility::Str::toUTF16(hintRegion) + TEXT_COLOR_DEFAULT;
        }

        totalRegions++;
    }

    location->hintText = HINT_PREFIX + TEXT_COLOR_CYAN + Utility::Str::toUTF16(gameItemToName(location->currentItem.getGameItemId())) + TEXT_COLOR_DEFAULT + u" is located in "s + regionText + u"."s;
    return HintError::NONE;
}

static HintError generateItemHintLocations(World& world, std::vector<Location*>& itemHintLocations)
{
    // Since item hints must name a specific island, locations in the below regions can't be item hints
    std::unordered_set<std::string> invalidItemHintRegions = {"Mailbox", "The Great Sea", "Hyrule"};
    // First, make a vector of possible item hint locations
    std::vector<Location*> possibleItemHintLocations = {};
    for (auto& [name, location] : world.locationEntries)
    {
        if (location.progression              &&  // if the location is a progression location...
           !location.hasBeenHinted            &&  // and has not been hinted at yet...
                                                  // and in't part of an invalid hint region
           !std::any_of(location.hintRegions.begin(), location.hintRegions.end(), [&](const std::string& hintRegion){return invalidItemHintRegions.count(hintRegion) > 0;}) &&
           !location.currentItem.isJunkItem() &&  // and it does not have a junk item...
          (!location.currentItem.isDungeonItem() || world.getSettings().keylunacy) && // and this isn't a dungeon item when keylunacy is off...
           !location.isRaceModeLocation &&        // and this isn't a race mode location
            location.hintPriority != "Always")    // and the hint priority is not always (this will be handled in the location hints)
           {
              // Then the item is a possible item hint location
              possibleItemHintLocations.push_back(&location);
           }
    }

    // Then, choose randomly from the vector until we've selected an appropriate number of hints
    for (uint8_t i = 0; i < world.getSettings().item_hints; i++)
    {
        if (possibleItemHintLocations.empty())
        {
            LOG_TO_DEBUG("No more possible item hint locations.")
        }
        auto hintLocation = popRandomElement(possibleItemHintLocations);
        itemHintLocations.push_back(hintLocation);
        LOG_AND_RETURN_IF_ERR(generateItemHintMessage(world, hintLocation));
        LOG_TO_DEBUG("Chose \"" + hintLocation->name + "\" as item hint location");
    }

    return HintError::NONE;
}

static HintError generateLocationHintLocations(World& world, std::vector<Location*>& locationHintLocations, uint8_t numLocationHints)
{

    std::vector<Location*> alwaysLocations = {};
    std::vector<Location*> sometimesLocations = {};

    // Put locations into the always or never categories depending on what their hint priority is
    for (auto& [name, location] : world.locationEntries)
    {
        if (location.progression && !location.hasBeenHinted && !location.isRaceModeLocation)
        {
            if (location.hintPriority == "Always")
            {
                alwaysLocations.push_back(&location);
            }
            else if (location.hintPriority == "Sometimes")
            {
                sometimesLocations.push_back(&location);
            }
        }
    }

    // If use_always_hints is off, then add the always locations to the sometimes locations
    if (!world.getSettings().use_always_hints)
    {
        addElementsToPool(sometimesLocations, alwaysLocations);
        alwaysLocations.clear();
    }

    for (uint8_t i = 0; i < numLocationHints; i++)
    {
        if (i < alwaysLocations.size())
        {
            locationHintLocations.push_back(alwaysLocations[i]);
        }
        else
        {
            if (sometimesLocations.empty())
            {
                LOG_TO_DEBUG("No more possible location hints.");
                break;
            }
            auto hintLocation = popRandomElement(sometimesLocations);
            locationHintLocations.push_back(hintLocation);
            hintLocation->hintText = HINT_PREFIX + TEXT_COLOR_RED + Utility::Str::toUTF16(hintLocation->name) + TEXT_COLOR_DEFAULT + u" rewards " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(hintLocation->currentItem.getGameItemId())) + TEXT_COLOR_DEFAULT + u".";
            LOG_TO_DEBUG("Chose \"" + hintLocation->name + "\" as location hint location");
        }
    }

    return HintError::NONE;
}

static HintError assignHoHoHints(World& world, WorldPool& worlds, std::list<Location*>& locations)
{
    // Shuffle the hints
    std::vector<Location*> locationsVector (locations.begin(), locations.end());
    shufflePool(locationsVector);
    // Duplicate hints until we have a multiple of 10 so that each Ho Ho gets the same
    // number of hints
    size_t counter = 0;
    while (locationsVector.size() % 10 != 0)
    {
        locationsVector.push_back(locationsVector[counter++]);
    }

    locations.assign(locationsVector.begin(), locationsVector.end());
    size_t hintsPerHoHo = locations.size() / 10;

    auto hohoLocations = world.getLocations();
    filterAndEraseFromPool(hohoLocations, [](Location* location){return location->categories.count(LocationCategory::HoHoHint) == 0;});

    // Keep retrying until Hoh Ho hints are successfully placed, or until we run out
    // of retries
    bool successfullyPlacedHoHoHints = true;
    int retries = 5;
    do
    {
        successfullyPlacedHoHoHints = true;
        LOG_TO_DEBUG("Clearing Ho Ho Hints");
        world.hohoHints.clear();
        for (auto location : locations)
        {
            // Remove this item from the world and see which Ho Ho are available
            // to be hinted at
            auto itemAtLocation = location->currentItem;
            location->currentItem = Item(GameItem::INVALID, world.getWorldId());

            ItemPool noItems = {};
            auto accessibleHoHoLocations = getAccessibleLocations(worlds, noItems, hohoLocations);
            // Erase Ho Ho locations which already have the desired number of hints
            filterAndEraseFromPool(accessibleHoHoLocations, [&](Location* hoho){return world.hohoHints[hoho].size() >= hintsPerHoHo || world.hohoHints[hoho].count(location) > 0;});

            if (accessibleHoHoLocations.empty())
            {
                break;
            }

            auto hohoLocation = RandomElement(accessibleHoHoLocations);
            world.hohoHints[hohoLocation].insert(location);
            LOG_TO_DEBUG("\"" + hohoLocation->name + "\" now hints to \"" + location->name + "\"");

            // Give back the item
            location->currentItem = itemAtLocation;
        }

        // If we went through and couldn't place all the hints, flag a failure and
        // try again
        for (auto& [hohoLocation, hintedLocations] : world.hohoHints)
        {
            if (hintedLocations.size() != hintsPerHoHo)
            {
                successfullyPlacedHoHoHints = false;
                break;
            }
        }
    }
    while(!successfullyPlacedHoHoHints);
    return HintError::NONE;
}

HintError generateHints(WorldPool& worlds)
{
    LOG_AND_RETURN_IF_ERR(calculatePossiblePathLocations(worlds));
    LOG_AND_RETURN_IF_ERR(calculatePossibleBarrenRegions(worlds));

    for (auto& world : worlds)
    {
        std::vector<Location*> hintLocations = {};

        // First, select path hint locations so that we don't hint at them during item and location hints
        LOG_AND_RETURN_IF_ERR(generatePathHintLocations(world, hintLocations));

        // Determine barren hint locations next so that we similarly don't hint at locations
        // in barren regions when generating item and location hints
        LOG_AND_RETURN_IF_ERR(generateBarrenHintLocations(world, hintLocations));

        // Next, select item hint locations
        LOG_AND_RETURN_IF_ERR(generateItemHintLocations(world, hintLocations));

        // Finally, select locations for location hints
        // If we weren't able to select as many hints as the player wanted for previous
        // types, then select more location hints to fill in the total number the player wanted
        auto& settings = world.getSettings();
        uint8_t totalNumHints = settings.path_hints + settings.barren_hints + settings.item_hints + settings.location_hints;
        uint8_t totalMadeHints = hintLocations.size();
        LOG_AND_RETURN_IF_ERR(generateLocationHintLocations(world, hintLocations, totalNumHints - totalMadeHints));

        // Distribute hints evenly among the possible hint placement options
        std::vector<std::string> hintPlacementOptions = {};
        std::unordered_map<std::string, std::list<Location*>> hintsForCategory = {};
        if (settings.ho_ho_hints)
        {
            hintPlacementOptions.push_back("ho ho");
        }
        if (settings.korl_hints)
        {
            hintPlacementOptions.push_back("korl");
        }

        // No placement options selected, don't use hints
        if (hintPlacementOptions.empty())
        {
            return HintError::NONE;
        }

        for (size_t i = 0; i < hintLocations.size(); i++)
        {
            // iterate to the next placement option on each index of the hint locations
            std::string placementOption = hintPlacementOptions[i % hintPlacementOptions.size()];
            // add the hint location to that placement option
            hintsForCategory[placementOption].push_back(hintLocations[i]);
            LOG_TO_DEBUG("Hint for \"" + hintLocations[i]->name + "\" will be given to " + placementOption);
        }

        LOG_AND_RETURN_IF_ERR(assignHoHoHints(world, worlds, hintsForCategory["ho ho"]));

        for (auto location : hintsForCategory["korl"])
        {
            world.korlHints.push_back(location);
        }
    }

    return HintError::NONE;
}
