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
        world.goalLocations.push_back(world.locationTable["Ganon's Tower - Defeat Ganondorf"].get());
        for (auto& [name, dungeon] : world.dungeons)
        {
            // Race mode locations are also goal locations
            if (dungeon.isRequiredDungeon)
            {
                world.goalLocations.push_back(dungeon.raceModeLocation);
            }
        }

        for (auto& [name, location] : world.locationTable)
        {
            if (!location->progression && !location->categories.contains(LocationCategory::BlueChuChu))
            {
                nonRequiredLocations.insert({location.get(), location->currentItem});
                location->currentItem = {GameItem::INVALID, location->world};
            }
        }
    }

    // Determine path locations for each goal location by going through the playthrough
    // and seeing if taking away the item at each location can still access the goal locations
    for (auto& world : worlds)
    {
        for (auto& potentialPathLocation : world.getLocations(true))
        {
            auto itemAtLocation = potentialPathLocation->currentItem;
            if (itemAtLocation.isJunkItem())
            {
                continue;
            }

            // Take the item away from the location
            potentialPathLocation->currentItem = Item(GameItem::INVALID, potentialPathLocation->world);

            // Run a search without the item
            runGeneralSearch(worlds);

            for (auto& location : world.getLocations(true))
            {
                // If we never reached the goal location, then this location
                // is "on the path to" the goal location. Since hints will refer
                // to locations in an individual's world, only add locations
                // which are in the same world as the goal location
                if (!location->hasBeenFound)
                {
                    location->pathLocations.push_back(potentialPathLocation);
                }
            }

            // Then give back the location's item
            potentialPathLocation->currentItem = itemAtLocation;
        }

        #ifdef ENABLE_DEBUG
            for (auto& location : world.getLocations(true))
            {
                LOG_TO_DEBUG("Path locations for " + location->getName() + " [");
                for (auto& pathLocation : location->pathLocations)
                {
                    LOG_TO_DEBUG("\t" + pathLocation->getName() + " (" + pathLocation->currentItem.getName() + ")");
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
        for (auto& [areaName, area] : world.areaTable)
        {
            // We'll be performing several operations during this loop
            for (auto& locAccess : area->locations)
            {
                auto location = locAccess.location;
                // Assign the location its hint regions if it doesn't already
                // have them
                if (location->hintRegions.empty())
                {
                    location->hintRegions = area->findHintRegions();
                }
                // If this location is progression, then add its hint regions to
                // the set of potentially barren regions
                if (location->progression && !location->hintRegions.empty())
                {
                    for (auto& region : location->hintRegions)
                    {
                        // Don't add ganon's tower to the list
                        if (region == "Ganon's Tower")
                        {
                            continue;
                        }
                        world.barrenRegions[region] = {};
                    }
                }

                // During this loop we'll also go through and mark certain items as junk items.
                // For the purposes of barren hints, if a major item cannot possibly lead to any
                // required items, then it will not block the region it's in from being considered
                // barren.
                auto chainLocations = location->currentItem.getChainLocations();
                if (!chainLocations.empty())
                {
                    // If all of this item's chain locations' are barren as chain locations, then this item is barren too.
                    if (std::ranges::all_of(chainLocations, [](const Location* loc){ return loc->isBarrenAsChainLocation(); }))
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
        bool newJunkItems;
        do
        {
            newJunkItems = false;
            for (Item* item : potentiallyJunkItems)
            {
                if (!item->isJunkItem() && std::ranges::all_of(item->getChainLocations(), [](const Location* loc){ return loc->isBarrenAsChainLocation(); }))
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
        // in the barren region. For barren hints, dungeons within islands also
        // count as being part of the island.
        for (auto& [name, location] : world.locationTable)
        {
            // Locations which have known vanilla/expected items should not block a region from being barren
            if (location->progression && !location->hasKnownVanillaItem && !location->hasExpectedItem)
            {
                for (auto& locAccess : location->accessPoints)
                {
                    auto area = locAccess->area;
                    auto generalHintRegions = area->findHintRegions(/*onlyNonIslands = */true);
                    auto islands = area->findIslands();
                    for (auto hintRegions : {generalHintRegions, islands})
                    {
                        for (auto& hintRegion : hintRegions)
                        {
                            if (world.barrenRegions.contains(hintRegion))
                            {
                                if (location->currentItemCanBeBarren())
                                {
                                    world.barrenRegions[hintRegion].insert(location.get());
                                }
                                else
                                {
                                    LOG_TO_DEBUG("Removed " + hintRegion + " from barren pool due to item " + location->currentItem.getName() + " at location " + location->getName());
                                    world.barrenRegions.erase(hintRegion);
                                    continue;
                                }

                                // Also make sure the outside dependent locations are barren a well
                                for (auto outsideLoc : location->outsideDependentLocations)
                                {
                                    // If an outside dependent location is not barren, remove the region from being barren
                                    if (!outsideLoc->isBarrenAsChainLocation())
                                    {
                                        LOG_TO_DEBUG("Removed " + hintRegion + " from barren pool due to item " + outsideLoc->currentItem.getName() + " at location " + outsideLoc->getName() + " which is dependent on " + location->getName());
                                        world.barrenRegions.erase(hintRegion);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
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

static HintError generatePathHintMessage(Location* location, Location* goalLocation, std::list<Hint>& hints)
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

    Hint hint{};
    hint.text["English"] = HINT_PREFIX_ENGLISH + englishRegionText + englishPlurality + u" on the path to "s + TEXT_COLOR_RED + Utility::Str::toUTF16(goalLocation->goalNames["English"]) + TEXT_COLOR_DEFAULT + u"."s;
    hint.text["Spanish"] = HINT_PREFIX_SPANISH + spanishRegionText + spanishPlurality + u" hacia el camino de "s + TEXT_COLOR_RED + Utility::Str::toUTF16(goalLocation->goalNames["Spanish"]) + TEXT_COLOR_DEFAULT + u"."s;
    hint.text["French"] = HINT_PREFIX_FRENCH + frenchRegionText + frenchPlurality + u" sur le chemin de "s + TEXT_COLOR_RED + Utility::Str::toUTF16(goalLocation->goalNames["French"]) + TEXT_COLOR_DEFAULT + u"."s;
    hint.type = HintType::PATH;
    hint.location = location;
    hints.emplace_back(std::move(hint));
    return HintError::NONE;
}

static HintError generatePathHintLocations(World& world, std::list<Hint>& hints)
{
    // Shuffle each pool of path locations so that their orders are random
    std::vector<Location*> goalLocations = {};
    for (auto& goalLocation : world.goalLocations)
    {
        shufflePool(goalLocation->pathLocations);
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
                goalLocations.push_back(world.locationTable["Ganon's Tower - Defeat Ganondorf"].get());
                addedGanonPathLocation = true;
            }

            if (goalLocations.empty())
            {
                LOG_TO_DEBUG("No more possible path hints");
                break;
            }

            goalLocation = RandomElement(goalLocations);
        }

        auto possiblePathLocations = goalLocation->pathLocations;

        // Filter out known vanilla items, and expected items, and triforce shards if Ho Ho is hinting them
        filterAndEraseFromPool(possiblePathLocations, [settings = world.getSettings()](auto location){
            auto& item = location->currentItem;
            return (location->hasKnownVanillaItem || location->hasExpectedItem || (settings.ho_ho_triforce_hints && item.isTriforceShard()));
        });

        auto hintLocation = getHintableLocation(possiblePathLocations);
        if (hintLocation == nullptr)
        {
            LOG_TO_DEBUG("No more path locations for " + goalLocation->getName());
            filterAndEraseFromPool(goalLocations, [&goalLocation](Location* goal){return goal == goalLocation;});
            i--;
            continue;
        }
        LOG_AND_RETURN_IF_ERR(generatePathHintMessage(hintLocation, goalLocation, hints));
        hintLocation->hasBeenHinted = true;
        LOG_TO_DEBUG("Chose \"" + hintLocation->getName() + "\" as path hint for " + goalLocation->getName());
    }

    return HintError::NONE;
}

static HintError generateBarrenHintMessage(const std::string& barrenRegion, std::list<Hint>& hints, World& world)
{
    Hint hint{};
    hint.text["English"] = HINT_PREFIX_ENGLISH u"visiting " + world.getUTF16HintRegion(barrenRegion, "English", Text::Type::PRETTY, Text::Color::BLUE) + u" is a foolish choice."s;
    hint.text["Spanish"] = HINT_PREFIX_SPANISH u"visitar " + world.getUTF16HintRegion(barrenRegion, "Spanish", Text::Type::PRETTY, Text::Color::BLUE) + u" es una idea imprudente."s;
    hint.text["French"] = HINT_PREFIX_FRENCH u"visiter " + world.getUTF16HintRegion(barrenRegion, "French", Text::Type::PRETTY, Text::Color::BLUE) + u" est un choix imprudent."s;
    hint.type = HintType::BARREN;
    hints.emplace_back(std::move(hint));
    return HintError::NONE;
}

static HintError generateBarrenHintLocations(World& world, std::list<Hint>& hints)
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
        }
        LOG_AND_RETURN_IF_ERR(generateBarrenHintMessage(barrenRegion, hints, world));
        hints.back().location = *world.barrenRegions[barrenRegion].begin();
        LOG_TO_DEBUG("Chose \"" + barrenRegion + "\" as a hinted barren region");

        // Erase any potential barren regions if any of their locations have already
        // been hinted at. This means that if we've hinted a dungeon barren, then
        // we can't also hint the island the dungeon is on as barren (and vice versa)
        // Basically, there should be no overlapping of the regions that are hinted
        // barren.
        for (size_t j = 0; j < barrenPool.size(); j++)
        {
            auto& pool = world.barrenRegions[barrenPool[j]];
            if (std::ranges::any_of(pool, [](auto loc){return loc->hasBeenHinted;}))
            {
                // Reform the distribution without this region
                LOG_TO_DEBUG("Removed " + barrenPool[j] + " from barren pool.");
                barrenDistributions.erase(barrenDistributions.begin() + j);
                barrenPool.erase(barrenPool.begin() + j);
                j--;
            }
        }

        barrenDistribution = std::discrete_distribution<size_t>(barrenDistributions.begin(), barrenDistributions.end());
    }

    return HintError::NONE;
}

static HintError generateItemHintMessage(Location* location, std::list<Hint>& hints)
{
    std::u16string englishRegionText = u"";
    std::u16string spanishRegionText = u"";
    std::u16string frenchRegionText = u"";

    size_t totalRegions = 0;
    const World* const world = location->world;
    std::list<std::string> hintRegions = location->hintRegions;

    // If this is an item in a dungeon, use the dungeon's island(s) for the hint instead
    if (world->dungeons.contains(hintRegions.front()))
    {
        hintRegions = world->dungeons.at(hintRegions.front()).islands;
    }

    for (const std::string& hintRegion : hintRegions)
    {
        // Change the formatting depending on how many regions lead to the location
        if (totalRegions == 0)
        {
            englishRegionText += world->getUTF16HintRegion(hintRegion, "English", Text::Type::PRETTY, Text::Color::RED);
            spanishRegionText += world->getUTF16HintRegion(hintRegion, "Spanish", Text::Type::PRETTY, Text::Color::RED);
            frenchRegionText += world->getUTF16HintRegion(hintRegion, "French", Text::Type::PRETTY, Text::Color::RED);
        }
        else if (totalRegions == hintRegions.size() - 1 && hintRegions.size() == 2)
        {
            englishRegionText += u" and "s + world->getUTF16HintRegion(hintRegion, "English", Text::Type::PRETTY, Text::Color::RED);
            auto spanishRegion = world->getUTF16HintRegion(hintRegion, "Spanish", Text::Type::PRETTY, Text::Color::RED);
            spanishRegionText += ((spanishRegion[0] == u'I' || spanishRegion[0] == u'i') ? u" y " : u" e ") + spanishRegion;
            frenchRegionText += u" et "s + world->getUTF16HintRegion(hintRegion, "French", Text::Type::PRETTY, Text::Color::RED);
        }
        else if (totalRegions == hintRegions.size() - 1)
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

    auto englishHintedItem = item.getUTF16Name("English", textType, Text::Color::CYAN);
    auto spanishHintedItem = item.getUTF16Name("Spanish", textType, Text::Color::CYAN);
    auto frenchHintedItem = item.getUTF16Name("French", textType, Text::Color::CYAN);

    auto englishItemImportance = location->generateImportanceText("English");
    auto spanishItemImportance = location->generateImportanceText("Spanish");
    auto frenchItemImportance = location->generateImportanceText("French");

    // Angular Isles and Forbidden Woods should use the plural tense in French even if they're a single area being referred to
    std::u16string frenchPlurality = (totalRegions == 1 && englishRegionText.find(u"Angular Isles") == std::string::npos && englishRegionText.find(u"Forbidden Woods") == std::string::npos) ? u" détiendrait "s : u" détiendraient "s;

    Hint hint{};
    hint.text["English"] = HINT_PREFIX_ENGLISH + englishHintedItem + englishItemImportance + u" can be found at "s + englishRegionText + u"."s;
    hint.text["Spanish"] = HINT_PREFIX_SPANISH + spanishHintedItem + spanishItemImportance + u" se encuentra en "s + spanishRegionText + u"."s;
    hint.text["French"] = HINT_PREFIX_FRENCH + frenchRegionText + frenchPlurality + frenchHintedItem + frenchItemImportance + u"."s;
    hint.type = HintType::ITEM;
    hint.location = location;
    hints.emplace_back(std::move(hint));
    return HintError::NONE;
}

static HintError generateItemHintLocations(World& world, std::list<Hint>& hints)
{
    auto& settings = world.getSettings();
    // First, make a vector of possible item hint locations
    std::vector<Location*> possibleItemHintLocations = {};
    for (auto& [name, location] : world.locationTable)
    {
        if (location->progression              &&  // if the location is a progression location...
           !location->currentItem.isJunkItem() &&  // and does not have a junk item...
           !location->hasKnownVanillaItem      &&  // and does not have a known vanilla item...
           !location->hasExpectedItem          &&  // and does not have an expected item...
           !location->hasBeenHinted            &&  // and has not been hinted at yet...
           !(settings.ho_ho_triforce_hints && location->currentItem.isTriforceShard())) // and isn't a shard when ho ho will hint shards...
           
           {
              // Then the item is a possible item hint location
              possibleItemHintLocations.push_back(location.get());
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
        LOG_AND_RETURN_IF_ERR(generateItemHintMessage(hintLocation, hints));
        hintLocation->hasBeenHinted = true;
        LOG_TO_DEBUG("Chose \"" + hintLocation->getName() + "\" as item hint location");
    }

    // Choose one more potential item hint to give to the big octo great fairy.
    // This hint will always be chosen regardless of settings
    // Don't let the great fairy hint at itself
    Location* octoFairyHintLocation = nullptr;
    filterAndEraseFromPool(possibleItemHintLocations, [](const Location* location){ return location->hintRegions.size() != 1 || location->getName() == "Two Eye Reef - Big Octo Great Fairy"; });
    if (!possibleItemHintLocations.empty())
    {
        octoFairyHintLocation = popRandomElement(possibleItemHintLocations);
        LOG_TO_DEBUG("Chose \"" + octoFairyHintLocation->getName() + "\" as item hint location for big octo fairy")
    }
    else
    {
        // This item may not be helpful but we need something to fill the text
        // And it mirrors her vanilla hint at the Fairy Queen in M&C
        octoFairyHintLocation = world.locationTable["Mother & Child Isles - Inside Mother Isle"].get();
        LOG_TO_DEBUG("No possible item hints for big octo fairy. Falling back to Mother & Child Isles - Inside Mother Isle")
    }

    std::list<Hint> bigOctoFairyHints{};
    LOG_AND_RETURN_IF_ERR(generateItemHintMessage(octoFairyHintLocation, bigOctoFairyHints));
    world.bigOctoFairyHint = bigOctoFairyHints.back();

    return HintError::NONE;
}

static HintError generateLocationHintMessage(Location* location, std::list<Hint>& hints)
{
    auto& item = location->currentItem;

    auto englishLocation = Utility::Str::toUTF16(location->names["English"]);
    auto spanishLocation = Utility::Str::toUTF16(location->names["Spanish"]);
    auto frenchLocation = Utility::Str::toUTF16(location->names["French"]);

    // Determine if the item's direct name or the cryptic text should be used
    auto textType = location->world->getSettings().clearer_hints ? Text::Type::PRETTY : Text::Type::CRYPTIC;
    auto englishHintedItem = item.getUTF16Name("English", textType, Text::Color::RED);
    auto spanishHintedItem = item.getUTF16Name("Spanish", textType, Text::Color::RED);
    auto frenchHintedItem = item.getUTF16Name("French", textType, Text::Color::RED);

    auto englishItemImportance = location->generateImportanceText("English");
    auto spanishItemImportance = location->generateImportanceText("Spanish");
    auto frenchItemImportance = location->generateImportanceText("French");

    Hint hint{};
    hint.text["English"] = HINT_PREFIX_ENGLISH + TEXT_COLOR_RED + englishLocation + TEXT_COLOR_DEFAULT + u" rewards " + englishHintedItem + englishItemImportance + u"."s;
    hint.text["Spanish"] = HINT_PREFIX_SPANISH + TEXT_COLOR_RED + spanishLocation + TEXT_COLOR_DEFAULT + u" otorgará " + spanishHintedItem + spanishItemImportance + u"."s;
    hint.text["French"] = HINT_PREFIX_FRENCH + TEXT_COLOR_RED + frenchLocation + TEXT_COLOR_DEFAULT + u" aurait pour récompense " + frenchHintedItem + frenchItemImportance + u"."s;
    hint.type = HintType::LOCATION;
    hint.location = location;
    hints.emplace_back(std::move(hint));
    return HintError::NONE;
}

static HintError generateAlwaysHints(World& world, std::list<Hint>& hints)
{
    // Return early if we're not specifically generating always hints, or if we don't have any location hints
    if (!world.getSettings().use_always_hints || world.getSettings().location_hints == 0)
    {
        return HintError::NONE;
    }

    // Gather all the always locations
    std::vector<Location*> alwaysLocations = {};
    for (auto& [name, location] : world.locationTable)
    {
        if (location->progression && location->hintPriority == "Always")
        {
            alwaysLocations.push_back(location.get());
        }
    }

    // Shuffle the always locations so we don't hint them in the same order
    shufflePool(alwaysLocations);

    for (uint8_t i = 0; i < world.getSettings().location_hints; ++i)
    {
        Location* hintLocation = nullptr;
        if (i < alwaysLocations.size())
        {
            hintLocation = alwaysLocations[i];
        }
        // Break if we've hinted all the always locations
        else
        {
            break;
        }
        LOG_AND_RETURN_IF_ERR(generateLocationHintMessage(hintLocation, hints));
        hintLocation->hasBeenHinted = true;
        LOG_TO_DEBUG("Chose \"" + hintLocation->getName() + "\" as always hint location");
    }

    return HintError::NONE;
}

static HintError generateLocationHintLocations(World& world, std::list<Hint>& hints, uint8_t numLocationHints)
{
    std::vector<Location*> sometimesLocations = {};

    // Gather all the sometimes locations
    for (auto& [name, location] : world.locationTable)
    {
        if (location->progression && 
           !location->hasBeenHinted && 
           !location->isRaceModeLocation && 
            location->hintPriority == "Sometimes" && 
           !(world.getSettings().ho_ho_triforce_hints && location->currentItem.isTriforceShard()))
            {
                sometimesLocations.push_back(location.get());
            }
    }

    // Choose them randomly
    for (uint8_t i = 0; i < numLocationHints; ++i)
    {
        Location* hintLocation = nullptr;
        if (sometimesLocations.empty())
        {
            LOG_TO_DEBUG("No more possible location hints.");
            break;
        }
        hintLocation = popRandomElement(sometimesLocations);
        LOG_AND_RETURN_IF_ERR(generateLocationHintMessage(hintLocation, hints));
        hintLocation->hasBeenHinted = true;
        LOG_TO_DEBUG("Chose \"" + hintLocation->getName() + "\" as location hint location");
    }

    return HintError::NONE;
}

static HintError assignHoHoHints(World& world, WorldPool& worlds, std::list<Hint>& hints)
{
    // If ho ho is hinting triforces, make those hints now
    if (world.getSettings().ho_ho_triforce_hints)
    {
        for (auto location : world.getLocations(/*onlyProgression =*/ true))
        {
            if (location->currentItem.isTriforceShard() && !(world.getSettings().required_boss_items && location->isRaceModeLocation))
            {
                LOG_AND_RETURN_IF_ERR(generateItemHintMessage(location, hints));
            }
        }
    }

    // Shuffle the hints
    std::vector<Hint> hintsVector (hints.begin(), hints.end());
    shufflePool(hintsVector);
    // Duplicate hints until we have a multiple of 10 so that each Ho Ho gets the same
    // number of hints
    size_t counter = 0;
    while (hintsVector.size() % 10 != 0)
    {
        hintsVector.push_back(hintsVector[counter++]);
    }

    hints.assign(hintsVector.begin(), hintsVector.end());
    size_t hintsPerHoHo = hints.size() / 10;

    auto hohoLocations = world.getLocations();
    filterAndEraseFromPool(hohoLocations, [](const Location* location){ return !location->categories.contains(LocationCategory::HoHoHint); });

    // Keep retrying until Ho Ho hints are successfully placed, or until we run out
    // of retries
    bool successfullyPlacedHoHoHints = true;
    do
    {
        successfullyPlacedHoHoHints = true;
        LOG_TO_DEBUG("Clearing Ho Ho Hints");
        world.hohoHints.clear();
        for (auto& hint : hints)
        {
            auto location = hint.location;
            // Remove this item from the world and see which Ho Ho are available
            // to be hinted at
            auto itemAtLocation = location->currentItem;
            location->currentItem = Item(GameItem::INVALID, &world);

            // Find all accessible locations without the item
            ItemPool noItems = {};
            auto accessibleHoHoLocations = getAccessibleLocations(worlds, noItems, hohoLocations);

            // Give back the item
            location->currentItem = itemAtLocation;

            // Erase Ho Ho locations which already have the desired number of hints
            filterAndEraseFromPool(accessibleHoHoLocations, [&](Location* hoho){
                return world.hohoHints[hoho].size() >= hintsPerHoHo || std::ranges::any_of(world.hohoHints[hoho], [&](const Hint& h){return h.location == location;});
            });

            if (accessibleHoHoLocations.empty())
            {
                break;
            }

            auto hohoLocation = RandomElement(accessibleHoHoLocations);
            world.hohoHints[hohoLocation].push_back(hint);
            LOG_TO_DEBUG("\"" + hohoLocation->getName() + "\" now hints to \"" + location->getName() + "\"");
        }

        // If we went through and couldn't place all the hints, flag a failure and
        // try again
        for (auto& [hohoLocation, locationHints] : world.hohoHints)
        {
            if (locationHints.size() != hintsPerHoHo)
            {
                successfullyPlacedHoHoHints = false;
                break;
            }
        }
    }
    while(!successfullyPlacedHoHoHints);
    return HintError::NONE;
}

static HintError assignKreebHints(World& world, WorldPool& worlds)
{
    // Get all bow locations
    // Shuffle locations to prevent any possible meta-gaming where the last bow might be
    // since otherwise they'll appear in order of location id
    auto allLocations = world.getLocations(/*onlyProgression = */ true);
    shufflePool(allLocations);
    for (auto& location : allLocations)
    {
        if (location->currentItem.getGameItemId() == GameItem::ProgressiveBow)
        {
            LOG_AND_RETURN_IF_ERR(generateItemHintMessage(location, world.kreebHints));
        }
    }
    return HintError::NONE;
}

static HintError assignKorlSwordHints(World& world, WorldPool& worlds)
{
    // Get all sword locations
    // Shuffle locations to prevent any possible meta-gaming where the swords bow might be
    // since otherwise they'll appear in order of location id
    auto allLocations = world.getLocations(/*onlyProgression = */ true);
    shufflePool(allLocations);
    for (auto& location : allLocations)
    {
        if (location->currentItem.getGameItemId() == GameItem::ProgressiveSword)
        {
            LOG_AND_RETURN_IF_ERR(generateItemHintMessage(location, world.korlHyruleHints));
        }
    }
    return HintError::NONE;
}

HintError generateHints(WorldPool& worlds)
{
    LOG_AND_RETURN_IF_ERR(calculatePossiblePathLocations(worlds));
    LOG_AND_RETURN_IF_ERR(calculatePossibleBarrenRegions(worlds));

    for (auto& world : worlds)
    {
        std::list<Hint> hints = {};

        // First, select always hints
        LOG_AND_RETURN_IF_ERR(generateAlwaysHints(world, hints));

        // Then select path hint locations so that we don't hint at them during item and location hints
        LOG_AND_RETURN_IF_ERR(generatePathHintLocations(world, hints));

        // Determine barren hint locations next so that we similarly don't hint at locations
        // in barren regions when generating item and location hints
        LOG_AND_RETURN_IF_ERR(generateBarrenHintLocations(world, hints));

        // Next, select item hint locations
        LOG_AND_RETURN_IF_ERR(generateItemHintLocations(world, hints));

        // Finally, select locations for location hints
        // If we weren't able to select as many hints as the player wanted for previous
        // types, then select more location hints to fill in the total number the player wanted
        auto& settings = world.getSettings();
        uint8_t totalNumHints = settings.path_hints + settings.barren_hints + settings.item_hints + settings.location_hints;
        uint8_t totalMadeHints = hints.size();
        LOG_AND_RETURN_IF_ERR(generateLocationHintLocations(world, hints, totalNumHints - totalMadeHints));

        // Sort hints by type
        // Need a vector to sort
        std::vector<Hint> hintsVector (hints.begin(), hints.end());
        std::ranges::sort(hintsVector, [](const Hint& h1, const Hint& h2){
            return h1.type < h2.type;
        });
        hints.assign(hintsVector.begin(), hintsVector.end());

        // Assign Kreeb Bow Hints if the setting is enabled
        if (settings.kreeb_bow_hints)
        {
            assignKreebHints(world, worlds);
        }

        // Assign Korl Sword Hints if the setting is enabled
        if (settings.korl_sword_hints)
        {
            assignKorlSwordHints(world, worlds);
        }

        // Distribute hints evenly among the possible hint placement options
        std::vector<std::string> hintPlacementOptions = {};
        std::unordered_map<std::string, std::list<Hint>> hintsForCategory = {};
        // Only include ho ho if he's not hinting triforces
        if (settings.ho_ho_hints && !settings.ho_ho_triforce_hints)
        {
            hintPlacementOptions.emplace_back("ho ho");
        }
        if (settings.korl_hints)
        {
            hintPlacementOptions.emplace_back("korl");
        }

        // No placement options selected, don't use hints
        if (hintPlacementOptions.empty() && !settings.ho_ho_triforce_hints)
        {
            return HintError::NONE;
        }

        size_t i = 0;
        for (auto& hint : hints)
        {
            // iterate to the next placement option on each index of the hint locations
            std::string placementOption = hintPlacementOptions[i % hintPlacementOptions.size()];
            // add the hint location to that placement option
            hintsForCategory[placementOption].push_back(hint);
            LOG_TO_DEBUG("Hint \"" + Utility::Str::toUTF8(hint.text["English"]) + "\" will be given to " + placementOption);
            ++i;
        }

        LOG_AND_RETURN_IF_ERR(assignHoHoHints(world, worlds, hintsForCategory["ho ho"]));

        for (auto& hint : hintsForCategory["korl"])
        {
            world.korlHints.push_back(hint);
        }
    }

    return HintError::NONE;
}
