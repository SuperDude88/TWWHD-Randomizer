#include "Hints.hpp"

#include <logic/Search.hpp>
#include <logic/PoolFunctions.hpp>
#include <seedgen/random.hpp>
#include <command/Log.hpp>
#include <filetypes/util/msbtMacros.hpp>
#include <utility/string.hpp>

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
            if (dungeon.isRequiredDungeon)
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
                location.currentItem = {GameItem::INVALID, location.world};
            }
        }
    }

    // Determine path locations for each goal location by going through the playthrough
    // and seeing if taking away the item at each location can still access the goal locations
    for (auto& world : worlds)
    {
        auto& settings = world.getSettings();
        for (auto& sphere : world.playthroughSpheres)
        {
            for (auto location : sphere)
            {
                auto itemAtLocation = location->currentItem;
                // If this location has a small or big key and the key is known to be within the dungeon,
                // then ignore it because the player already knows where those items are. Also ignore race
                // mode locations at the end of dungeons because players know those locations are required.
                if (location->hasKnownVanillaItem ||
                   (itemAtLocation.isSmallKey()  && settings.dungeon_small_keys == PlacementOption::OwnDungeon) ||
                   (itemAtLocation.isBigKey()    && settings.dungeon_big_keys   == PlacementOption::OwnDungeon) ||
                   (location->isRaceModeLocation))
                {
                    continue;
                }

                // Take the item away from the location
                location->currentItem = Item(GameItem::INVALID, location->world);

                // Run a search without the item
                runGeneralSearch(worlds);

                for (auto& [goalLocation, pathLocations] : world.pathLocations)
                {
                    // If we never reached the goal location, then this location
                    // is "on the path to" the goal location. Since hints will refer
                    // to locations in an individual's world, only add locations
                    // which are in the same world as the goal location
                    if (!goalLocation->hasBeenFound && goalLocation->world == location->world)
                    {
                        pathLocations.push_back(location);
                    }
                }

                // Then give back the location's item
                location->currentItem = itemAtLocation;
            }
        }

        #ifdef ENABLE_DEBUG
            for (auto& [goalLocation, pathLocations] : world.pathLocations)
            {
                LOG_TO_DEBUG("Path locations for " + goalLocation->getName() + " [");
                for (auto location : pathLocations)
                {
                    LOG_TO_DEBUG("  " + location->getName());
                }
                LOG_TO_DEBUG("]");
            }
        #endif
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
                else if (world.barrenRegions.contains(hintRegion))
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
            if (world.barrenRegions.contains(dungeonName) &&
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
    std::u16string englishRegionText = u"";
    std::u16string spanishRegionText = u"";
    std::u16string frenchRegionText = u"";

    size_t totalRegions = 0;
    auto world = location->world;
    for (auto& hintRegion : location->hintRegions)
    {
        // Change the formatting depending on how many regions lead to the location
        if (totalRegions == 0)
        {
            englishRegionText += world->getUTF16HintRegion(hintRegion, "English", Text::Type::PRETTY, Text::Color::CYAN);
            spanishRegionText += world->getUTF16HintRegion(hintRegion, "Spanish", Text::Type::PRETTY, Text::Color::CYAN);
            frenchRegionText += world->getUTF16HintRegion(hintRegion, "French", Text::Type::PRETTY, Text::Color::CYAN);
        }
        else if (totalRegions == location->hintRegions.size() - 1 && location->hintRegions.size() == 2)
        {
            englishRegionText += u" and "s + world->getUTF16HintRegion(hintRegion, "English", Text::Type::PRETTY, Text::Color::CYAN);
            auto spanishRegion = world->getUTF16HintRegion(hintRegion, "Spanish", Text::Type::PRETTY, Text::Color::CYAN);
            spanishRegionText += ((spanishRegion[0] == u'I' || spanishRegion[0] == u'i') ? u" y " : u" e ") + spanishRegion;
            frenchRegionText += u" et "s + world->getUTF16HintRegion(hintRegion, "French", Text::Type::PRETTY, Text::Color::CYAN);
        }
        else if (totalRegions == location->hintRegions.size() - 1)
        {
            englishRegionText += u", and "s + world->getUTF16HintRegion(hintRegion, "English", Text::Type::PRETTY, Text::Color::CYAN);
            auto spanishRegion = world->getUTF16HintRegion(hintRegion, "Spanish", Text::Type::PRETTY, Text::Color::CYAN);
            spanishRegionText += ((spanishRegion[0] == u'I' || spanishRegion[0] == u'i') ? u", y " : u", e ") + spanishRegion;
            frenchRegionText += u", et "s + world->getUTF16HintRegion(hintRegion, "French", Text::Type::PRETTY, Text::Color::CYAN);
        }
        else
        {
            englishRegionText += u", "s + world->getUTF16HintRegion(hintRegion, "English", Text::Type::PRETTY, Text::Color::CYAN);
            spanishRegionText += u", "s + world->getUTF16HintRegion(hintRegion, "Spanish", Text::Type::PRETTY, Text::Color::CYAN);
            frenchRegionText += u", "s + world->getUTF16HintRegion(hintRegion, "French", Text::Type::PRETTY, Text::Color::CYAN);
        }

        totalRegions++;
    }

    std::u16string englishPlurality = totalRegions == 1 ? u" is"s : u" are"s;
    std::u16string spanishPlurality = (totalRegions == 1 && spanishRegionText.find(u"Ilas") == std::string::npos) ? u" dirige"s : u" dirigen"s;
    std::u16string frenchPlurality = (totalRegions == 1 && englishRegionText.find(u"Angular Isles") == std::string::npos && englishRegionText.find(u"Forbidden Woods") == std::string::npos) ? u" est"s : u" sont"s;

    location->hint.text["English"] = HINT_PREFIX_ENGLISH + englishRegionText + englishPlurality + u" on the path to "s + TEXT_COLOR_RED + Utility::Str::toUTF16(goalLocation->goalNames["English"]) + TEXT_COLOR_DEFAULT + u"."s;
    location->hint.text["Spanish"] = HINT_PREFIX_SPANISH + spanishRegionText + spanishPlurality + u" hacia el camino de "s + TEXT_COLOR_RED + Utility::Str::toUTF16(goalLocation->goalNames["Spanish"]) + TEXT_COLOR_DEFAULT + u"."s;
    location->hint.text["French"] = HINT_PREFIX_FRENCH + frenchRegionText + frenchPlurality + u" sur le chemin de "s + TEXT_COLOR_RED + Utility::Str::toUTF16(goalLocation->goalNames["French"]) + TEXT_COLOR_DEFAULT + u"."s;
    location->hint.type = HintType::PATH;
    return HintError::NONE;
}

static HintError generatePathHintLocations(World& world, std::vector<Location*>& pathHintLocations)
{
    // Shuffle each pool of path locations so that their orders are random
    std::vector<Location*> goalLocations = {};
    for (auto& [goalLocation, pathLocations] : world.pathLocations)
    {
        shufflePool(pathLocations);
        // Initially we want to pull path hints from race mode dungeons before pulling from Ganondorf
        if (goalLocation->getName() != "Ganon's Tower - Defeat Ganondorf")
        {
            goalLocations.push_back(goalLocation);
        }
    }

    bool addedGanonPathLocation = false;
    for (uint8_t i = 0; i < world.getSettings().path_hints; i++)
    {
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
            if (i == goalLocations.size() && !addedGanonPathLocation)
            {
                goalLocations.push_back(&world.locationEntries["Ganon's Tower - Defeat Ganondorf"]);
                addedGanonPathLocation = true;
            }

            if (goalLocations.empty())
            {
                LOG_TO_DEBUG("No more possible path hints");
                break;
            }

            goalLocation = RandomElement(goalLocations);
        }

        auto hintLocation = getHintableLocation(world.pathLocations[goalLocation]);
        if (hintLocation == nullptr)
        {
            LOG_TO_DEBUG("No more path locations for " + goalLocation->getName());
            filterAndEraseFromPool(goalLocations, [&goalLocation](Location* goal){return goal == goalLocation;});
            i--;
            continue;
        }
        hintLocation->hasBeenHinted = true;
        pathHintLocations.push_back(hintLocation);
        LOG_TO_DEBUG("Chose \"" + hintLocation->getName() + "\" as path hint for " + goalLocation->getName());
        LOG_AND_RETURN_IF_ERR(generatePathHintMessage(hintLocation, goalLocation));
    }

    return HintError::NONE;
}

static HintError generateBarrenHintMessage(Location* location, const std::string& barrenRegion)
{
    auto world = location->world;
    location->hint.text["English"] = HINT_PREFIX_ENGLISH u"visiting " + world->getUTF16HintRegion(barrenRegion, "English", Text::Type::PRETTY, Text::Color::BLUE) + u" is a foolish choice."s;
    location->hint.text["Spanish"] = HINT_PREFIX_SPANISH u"visitar " + world->getUTF16HintRegion(barrenRegion, "Spanish", Text::Type::PRETTY, Text::Color::BLUE) + u" es una idea imprudente."s;
    location->hint.text["French"] = HINT_PREFIX_FRENCH u"visiter " + world->getUTF16HintRegion(barrenRegion, "French", Text::Type::PRETTY, Text::Color::BLUE) + u" est un choix imprudent."s;
    location->hint.type = HintType::BARREN;
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
        if (barrenPool.empty())
        {
            LOG_TO_DEBUG("No more barren regions to hint at.");
            break;
        }
        auto regionIndex = barrenDistribution(GetGenerator());
        auto& barrenRegion = barrenPool[regionIndex];
        // Set all locations in the selected barren region as hinted at
        // so we don't hint at them again
        for (auto location : world.barrenRegions[barrenRegion])
        {
            location->hasBeenHinted = true;
            LOG_AND_RETURN_IF_ERR(generateBarrenHintMessage(location, barrenRegion));
        }
        barrenHintLocations.push_back(*world.barrenRegions[barrenRegion].begin());
        LOG_TO_DEBUG("Chose \"" + barrenRegion + "\" as a hinted barren region");

        // Reform the distribution without the region we just chose
        barrenDistributions.erase(barrenDistributions.begin() + regionIndex);
        barrenPool.erase(barrenPool.begin() + regionIndex);
        barrenDistribution = std::discrete_distribution<size_t>(barrenDistributions.begin(), barrenDistributions.end());
    }

    return HintError::NONE;
}

static HintError generateItemHintMessage(Location* location)
{
    std::u16string englishRegionText = u"";
    std::u16string spanishRegionText = u"";
    std::u16string frenchRegionText = u"";

    size_t totalRegions = 0;
    auto world = location->world;
    for (auto hintRegion : location->hintRegions)
    {
        // If this is an item in a dungeon, use the dungeon's island for the hint instead
        if (world->dungeons.contains(hintRegion))
        {
            hintRegion = world->dungeons[hintRegion].island;
        }
        // Change the formatting depending on how many regions lead to the location
        if (totalRegions == 0)
        {
            englishRegionText += world->getUTF16HintRegion(hintRegion, "English", Text::Type::PRETTY, Text::Color::RED);
            spanishRegionText += world->getUTF16HintRegion(hintRegion, "Spanish", Text::Type::PRETTY, Text::Color::RED);
            frenchRegionText += world->getUTF16HintRegion(hintRegion, "French", Text::Type::PRETTY, Text::Color::RED);
        }
        else if (totalRegions == location->hintRegions.size() - 1 && location->hintRegions.size() == 2)
        {
            englishRegionText += u" and "s + world->getUTF16HintRegion(hintRegion, "English", Text::Type::PRETTY, Text::Color::RED);
            auto spanishRegion = world->getUTF16HintRegion(hintRegion, "Spanish", Text::Type::PRETTY, Text::Color::RED);
            spanishRegionText += ((spanishRegion[0] == u'I' || spanishRegion[0] == u'i') ? u" y " : u" e ") + spanishRegion;
            frenchRegionText += u" et "s + world->getUTF16HintRegion(hintRegion, "French", Text::Type::PRETTY, Text::Color::RED);
        }
        else if (totalRegions == location->hintRegions.size() - 1)
        {
            englishRegionText += u", and "s + world->getUTF16HintRegion(hintRegion, "English", Text::Type::PRETTY, Text::Color::RED);
            auto spanishRegion = world->getUTF16HintRegion(hintRegion, "Spanish", Text::Type::PRETTY, Text::Color::RED);
            spanishRegionText += ((spanishRegion[0] == u'I' || spanishRegion[0] == u'i') ? u", y " : u", e ") + spanishRegion;
            frenchRegionText += u", et "s + world->getUTF16HintRegion(hintRegion, "French", Text::Type::PRETTY, Text::Color::RED);
        }
        else
        {
            englishRegionText += u", "s + world->getUTF16HintRegion(hintRegion, "English", Text::Type::PRETTY, Text::Color::RED);
            spanishRegionText += u", "s + world->getUTF16HintRegion(hintRegion, "Spanish", Text::Type::PRETTY, Text::Color::RED);
            frenchRegionText += u", "s + world->getUTF16HintRegion(hintRegion, "French", Text::Type::PRETTY, Text::Color::RED);
        }


        totalRegions++;
    }

    // Determine if the item's direct name or the cryptic text should be used
    auto textType = world->getSettings().clearer_hints ? Text::Type::PRETTY : Text::Type::CRYPTIC;
    auto& item = location->currentItem;

    std::u16string englishHintedItem = item.getUTF16Name("English", textType, Text::Color::CYAN);
    std::u16string spanishHintedItem = item.getUTF16Name("Spanish", textType, Text::Color::CYAN);
    std::u16string frenchHintedItem = item.getUTF16Name("French", textType, Text::Color::CYAN);

    // Angular Isles and Forbidden Woods should use the plural tense in French even if they're a single area being referred to
    std::u16string frenchPlurality = (totalRegions == 1 && englishRegionText.find(u"Angular Isles") == std::string::npos && englishRegionText.find(u"Forbidden Woods") == std::string::npos) ? u" détiendrait "s : u" détiendraient "s;

    location->hint.text["English"] = HINT_PREFIX_ENGLISH + englishHintedItem + u" can be found at "s + englishRegionText + u"."s;
    location->hint.text["Spanish"] = HINT_PREFIX_SPANISH + spanishHintedItem + u" se encuentra en "s + spanishRegionText + u"."s;
    location->hint.text["French"] = HINT_PREFIX_FRENCH + frenchRegionText + frenchPlurality + frenchHintedItem + u"."s;
    location->hint.type = HintType::ITEM;
    return HintError::NONE;
}

static HintError generateItemHintLocations(World& world, std::vector<Location*>& itemHintLocations)
{
    auto& settings = world.getSettings();
    // Since item hints must name a specific island, locations in the below regions can't be item hints
    std::unordered_set<std::string> invalidItemHintRegions = {"Mailbox", "Great Sea", "Hyrule"};
    // First, make a vector of possible item hint locations
    std::vector<Location*> possibleItemHintLocations = {};
    for (auto& [name, location] : world.locationEntries)
    {
        if (location.progression              &&  // if the location is a progression location...
           !location.currentItem.isJunkItem() &&  // and does not have a junk item...
           !location.hasKnownVanillaItem      &&  // and does not have a known vanilla item...
           !location.hasBeenHinted            &&  // and has not been hinted at yet...
          (!location.currentItem.isSmallKey() || settings.dungeon_small_keys != PlacementOption::OwnDungeon) && // and isn't a small key when small keys are in their own dungeon
          (!location.currentItem.isBigKey()   || settings.dungeon_big_keys   != PlacementOption::OwnDungeon) && // and isn't a big key when big keys are in their own dungeon
          (!location.isRaceModeLocation || settings.progression_dungeons == ProgressionDungeons::Standard)   && // and isn't a race mode location when race mode/require bosses is enabled...
          ( location.hintPriority != "Always" || !world.getSettings().use_always_hints)                      && // and the hint priority is not "Always" when we're using always hints...
           !std::any_of(location.hintRegions.begin(), location.hintRegions.end(), [&](const std::string& hintRegion){return invalidItemHintRegions.contains(hintRegion);})) // and isn't part of an invalid hint region...
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
            break;
        }
        auto hintLocation = popRandomElement(possibleItemHintLocations);
        itemHintLocations.push_back(hintLocation);
        LOG_AND_RETURN_IF_ERR(generateItemHintMessage(hintLocation));
        LOG_TO_DEBUG("Chose \"" + hintLocation->getName() + "\" as item hint location");
    }

    // Choose one more potential item hint to give to the big octo great fairy.
    // This hint will always be chosen regardless of settings
    // Don't let the great fairy hint at itself
    filterAndEraseFromPool(possibleItemHintLocations, [](Location* location){return location->hintRegions.size() > 1 || location->getName() == "Two Eye Reef - Big Octo Great Fairy";});
    if (!possibleItemHintLocations.empty())
    {
        world.bigOctoFairyHintLocation = popRandomElement(possibleItemHintLocations);
        LOG_AND_RETURN_IF_ERR(generateItemHintMessage(world.bigOctoFairyHintLocation));
        LOG_TO_DEBUG("Chose \"" + world.bigOctoFairyHintLocation->getName() + "\" as item hint location for big octo fairy")
    }
    else
    {
        LOG_TO_DEBUG("No possible item hints for big octo fairy. Skipping")
    }

    return HintError::NONE;
}

static HintError generateLocationHintMessage(Location* location)
{
    auto& item = location->currentItem;
    location->hint.text["English"] = HINT_PREFIX_ENGLISH + TEXT_COLOR_RED + Utility::Str::toUTF16(location->names["English"]) + TEXT_COLOR_DEFAULT + u" rewards " + item.getUTF16Name("English", Text::Type::PRETTY, Text::Color::RED) + u"."s;
    location->hint.text["Spanish"] = HINT_PREFIX_SPANISH + TEXT_COLOR_RED + Utility::Str::toUTF16(location->names["Spanish"]) + TEXT_COLOR_DEFAULT + u" otorgará " + item.getUTF16Name("Spanish", Text::Type::PRETTY, Text::Color::RED) + u"."s;
    location->hint.text["French"] = HINT_PREFIX_FRENCH + TEXT_COLOR_RED + Utility::Str::toUTF16(location->names["French"]) + TEXT_COLOR_DEFAULT + u" aurait pour récompense " + item.getUTF16Name("French", Text::Type::PRETTY, Text::Color::RED) + u"."s;
    location->hint.type = HintType::LOCATION;
    return HintError::NONE;
}

static HintError generateLocationHintLocations(World& world, std::vector<Location*>& locationHintLocations, uint8_t numLocationHints)
{

    std::vector<Location*> alwaysLocations = {};
    std::vector<Location*> sometimesLocations = {};

    // Put locations into the always or sometimes categories depending on what their hint priority is
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
        Location* hintLocation = nullptr;
        if (i < alwaysLocations.size())
        {
            hintLocation = alwaysLocations[i];
        }
        else
        {
            if (sometimesLocations.empty())
            {
                LOG_TO_DEBUG("No more possible location hints.");
                break;
            }
            hintLocation = popRandomElement(sometimesLocations);
        }
        locationHintLocations.push_back(hintLocation);
        LOG_AND_RETURN_IF_ERR(generateLocationHintMessage(hintLocation));
        LOG_TO_DEBUG("Chose \"" + hintLocation->getName() + "\" as location hint location");
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
    filterAndEraseFromPool(hohoLocations, [](Location* location){return !location->categories.contains(LocationCategory::HoHoHint);});

    // Keep retrying until Ho Ho hints are successfully placed, or until we run out
    // of retries
    bool successfullyPlacedHoHoHints = true;
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
            location->currentItem = Item(GameItem::INVALID, &world);

            ItemPool noItems = {};
            auto accessibleHoHoLocations = getAccessibleLocations(worlds, noItems, hohoLocations);
            // Erase Ho Ho locations which already have the desired number of hints
            filterAndEraseFromPool(accessibleHoHoLocations, [&](Location* hoho){return world.hohoHints[hoho].size() >= hintsPerHoHo || world.hohoHints[hoho].contains(location);});

            if (accessibleHoHoLocations.empty())
            {
                break;
            }

            auto hohoLocation = RandomElement(accessibleHoHoLocations);
            world.hohoHints[hohoLocation].insert(location);
            LOG_TO_DEBUG("\"" + hohoLocation->getName() + "\" now hints to \"" + location->getName() + "\"");

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
            LOG_TO_DEBUG("Hint for \"" + hintLocations[i]->getName() + "\" will be given to " + placementOption);
        }

        LOG_AND_RETURN_IF_ERR(assignHoHoHints(world, worlds, hintsForCategory["ho ho"]));

        for (auto location : hintsForCategory["korl"])
        {
            world.korlHints.push_back(location);
        }
    }

    return HintError::NONE;
}
