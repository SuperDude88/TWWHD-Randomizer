
#include "EntranceShuffle.hpp"

#include <map>
#include <utility>
#include <iostream>

#include <logic/PoolFunctions.hpp>
#include <logic/Search.hpp>
#include <logic/Fill.hpp>
#include <utility/file.hpp>
#include <utility/string.hpp>
#include <seedgen/random.hpp>
#include <command/Log.hpp>
#include <libs/yaml.hpp>

#define ENTRANCE_SHUFFLE_ERROR_CHECK(err) if (err != EntranceShuffleError::NONE) {LOG_TO_DEBUG("Error: " + errorToName(err)); return err;}
#define CHECK_MIXED_POOL(name, type) if (name) { poolsToMix.insert(type); if (settings.decouple_entrances) { poolsToMix.insert(type##_REVERSE); } }

// The entrance randomization algorithm used here is heavily inspired by the entrance
// randomization algorithm from the Ocarina of Time Randomizer. While the algorithm
// has been adapted to work for The Wind Waker, credit also goes to those who developed
// it for Ocarina of Time.
//
// https://github.com/TestRunnerSRL/OoT-Randomizer/blob/Dev/EntranceShuffle.py

#ifdef ENABLE_DEBUG
static void logEntrancePool(EntrancePool& entrancePool, const std::string& poolName)
{
    LOG_TO_DEBUG(poolName + ": [");
    for (auto entrance : entrancePool)
    {
        LOG_TO_DEBUG("\t" + entrance->getOriginalName());
    }
    LOG_TO_DEBUG("]");
}

static void logMissingLocations(WorldPool& worlds)
{
    static int identifier = 0;
    LOG_TO_DEBUG("Missing Locations: [");
    for (auto& world : worlds)
    {
        for (auto& [name, location] : world.locationTable)
        {
            if (!location->hasBeenFound)
            {
                LOG_TO_DEBUG("\t" + location->getName());
                if (identifier++ < 10)
                {
                    // world.dumpWorldGraph(std::to_string(identifier));
                    // LOG_TO_DEBUG("Now Dumping " + std::to_string(identifier));
                }
                break;
            }
        }
    }
    LOG_TO_DEBUG("]");
}
#endif

static std::list<EntranceInfoPair> loadEntranceShuffleTable()
{
    std::string entrancePairsStr = "";
    Utility::getFileContents(Utility::get_data_path() / "logic/entrance_shuffle_table.yaml", entrancePairsStr, true);
    YAML::Node entrancePairs = YAML::Load(entrancePairsStr);
    std::list<EntranceInfoPair> table = {};

    for (const auto& entrancePair : entrancePairs)
    {
        auto type = entranceNameToType(entrancePair["Type"].as<std::string>());
        auto forward = entrancePair["Forward"].as<std::string>();
        auto reverse = entrancePair["Return"].as<std::string>();

        auto forwardArgs = Utility::Str::split(forward, ',');
        auto returnArgs = Utility::Str::split(reverse, ',');

        // Remove leading/trailing spaces
        for (auto args : {&forwardArgs, &returnArgs})
        {
            for (auto& arg : *args)
            {
                size_t pos = arg.find(' ');
                while (pos == 0)
                {
                    arg.erase(pos, 1);
                    pos = arg.find(' ');
                }
                pos = arg.rfind(' ');
                while (pos == arg.length() - 1)
                {
                    arg.erase(pos, 1);
                    pos = arg.rfind(' ');
                }
            }
        }

        // Get data
        auto parentArea = forwardArgs[0];
        auto connectedArea = forwardArgs[1];
        auto filepathStage = forwardArgs[2];
        uint8_t filepathRoom = atoi(forwardArgs[3].data());
        uint8_t sclsExitIndex = atoi(forwardArgs[4].data());
        auto stage = forwardArgs[5];
        uint8_t room = atoi(forwardArgs[6].data());
        uint8_t spawn = atoi(forwardArgs[7].data());
        auto forwardEntrance = EntranceInfo{parentArea, connectedArea, filepathStage, filepathRoom, sclsExitIndex, stage, room, spawn};

        parentArea = returnArgs[0];
        connectedArea = returnArgs[1];
        filepathStage = returnArgs[2];
        filepathRoom = atoi(returnArgs[3].data());
        sclsExitIndex = atoi(returnArgs[4].data());
        stage = returnArgs[5];
        room = atoi(returnArgs[6].data());
        spawn = atoi(returnArgs[7].data());

        auto returnEntrance = EntranceInfo{parentArea, connectedArea, filepathStage, filepathRoom, sclsExitIndex, stage, room, spawn};    

        bool savewarp = entrancePair["Savewarp"] ? true : false;
        bool windWarp = entrancePair["Wind Warp"] ? true : false;
        table.push_back(EntranceInfoPair{type, forwardEntrance, returnEntrance, savewarp, windWarp});
    }

    return table;
}

EntranceShuffleError setAllEntrancesData(World& world)
{
    for (auto& [type, forwardEntry, returnEntry, savewarp, windWarp] : loadEntranceShuffleTable())
    {
        auto forwardEntrance = world.getEntrance(forwardEntry.parentArea, forwardEntry.connectedArea);
        if (forwardEntrance == nullptr)
        {
            return EntranceShuffleError::BAD_ENTRANCE_SHUFFLE_TABLE_ENTRY;
        }
        forwardEntrance->setFilepathStage(forwardEntry.filepathStage);
        forwardEntrance->setFilepathRoomNum(forwardEntry.filepathRoom);
        forwardEntrance->setSclsExitIndex(forwardEntry.sclsExitIndex);
        forwardEntrance->setStageName(forwardEntry.stage);
        forwardEntrance->setRoomNum(forwardEntry.room);
        forwardEntrance->setSpawnId(forwardEntry.spawnId);
        forwardEntrance->setEntranceType(type);
        forwardEntrance->setOriginalEntranceType(type);
        forwardEntrance->setAsPrimary();
        if (returnEntry.parentArea != "")
        {
            auto returnEntrance = world.getEntrance(returnEntry.parentArea, returnEntry.connectedArea);
            if (returnEntrance == nullptr)
            {
                return EntranceShuffleError::BAD_ENTRANCE_SHUFFLE_TABLE_ENTRY;
            }
            returnEntrance->setFilepathStage(returnEntry.filepathStage);
            returnEntrance->setFilepathRoomNum(returnEntry.filepathRoom);
            returnEntrance->setSclsExitIndex(returnEntry.sclsExitIndex);
            returnEntrance->setStageName(returnEntry.stage);
            returnEntrance->setRoomNum(returnEntry.room);
            returnEntrance->setSpawnId(returnEntry.spawnId);
            returnEntrance->setEntranceType(entranceTypeToReverse(type, false));
            returnEntrance->setOriginalEntranceType(entranceTypeToReverse(type, false));
            forwardEntrance->bindTwoWay(returnEntrance);
            if (savewarp)
            {
                returnEntrance->setSavewarp(true);
            }
            if (windWarp)
            {
                returnEntrance->setWindWarp(true);
            }
        }
    }
    return EntranceShuffleError::NONE;
}

// Disconnect each entrance in the pool and create a corresponding target entrance
// which is connected to the root of the world graph. This creates the assumption
// that we have access to all the connected areas of each entrance
static EntrancePool assumeEntrancePool(EntrancePool& entrancePool)
{
    EntrancePool assumedPool = {};
    for (auto entrance : entrancePool)
    {
        auto assumedForward = entrance->assumeReachable();
        if (entrance->getReverse() && !entrance->isDecoupled())
        {
            auto assumedReturn = entrance->getReverse()->assumeReachable();
            assumedForward->bindTwoWay(assumedReturn);
        }
        assumedPool.push_back(assumedForward);
    }
    return assumedPool;
}

static EntranceShuffleError checkEntrancesCompatibility(Entrance* entrance, Entrance* target, std::vector<EntrancePair>& rollbacks)
{
    // While self-connections are funny, they get old pretty quickly and are more
    // prone to failure for the entrance placement algorithm. So we'll forbid them
    if (entrance->getReverse())
    {
        if (target->getReplaces() == entrance->getReverse())
        {
            return EntranceShuffleError::ATTEMPTED_SELF_CONNECTION;
        }
    }
    return EntranceShuffleError::NONE;
}

void changeConnections(Entrance* entrance, Entrance* targetEntrance)
{
    entrance->connect(targetEntrance->disconnect());
    entrance->setReplaces(targetEntrance->getReplaces());
    if (entrance->getReverse() && !entrance->isDecoupled())
    {
        targetEntrance->getReplaces()->getReverse()->connect(entrance->getReverse()->getAssumed()->disconnect());
        targetEntrance->getReplaces()->getReverse()->setReplaces(entrance->getReverse());
    }
}

void restoreConnections(Entrance* entrance, Entrance* targetEntrance)
{
    LOG_TO_DEBUG("Restoring Connection for " + entrance->getOriginalName());
    targetEntrance->connect(entrance->disconnect());
    entrance->setReplaces(nullptr);
    if (entrance->getReverse() && !entrance->isDecoupled())
    {
        entrance->getReverse()->getAssumed()->connect(targetEntrance->getReplaces()->getReverse()->disconnect());
        targetEntrance->getReplaces()->getReverse()->setReplaces(nullptr);
    }
}

static void deleteTargetEntrance(Entrance* targetEntrance)
{
    if (targetEntrance->getConnectedArea())
    {
        targetEntrance->disconnect();
    }
    if (targetEntrance->getParentArea())
    {
        targetEntrance->getWorld()->removeEntrance(targetEntrance);
    }
}

static void confirmReplacement(Entrance* entrance, Entrance* targetEntrance)
{
    deleteTargetEntrance(targetEntrance);
    if (entrance->getReverse() && !entrance->isDecoupled())
    {
        deleteTargetEntrance(entrance->getReverse()->getAssumed());
    }
}

static EntranceShuffleError validateWorld(WorldPool& worlds, const Entrance* entrancePlaced, ItemPool& itemPool)
{
    LOG_TO_DEBUG("Validating World");
    // Ensure that all item locations are still reachable within the given world
    if (!allLocationsReachable(worlds, itemPool))
    {
        #ifdef ENABLE_DEBUG
            LOG_TO_DEBUG("Error: All locations not reachable");
            logMissingLocations(worlds);
        #endif
        return EntranceShuffleError::ALL_LOCATIONS_NOT_REACHABLE;
    }

    // Ensure that there's at least one sphere zero location available to place an item
    // for the beginning of the seed
    ItemPool noItems = {};
    LocationPool progLocations = {};
    GET_COMPLETE_PROGRESSION_LOCATION_POOL(progLocations, worlds);
    auto locs = getAccessibleLocations(worlds, noItems, progLocations);
    #ifdef ENABLE_DEBUG
        LOG_TO_DEBUG(std::to_string(locs.size()) + " Sphere 0 locations: [");
        // uncomment to see all sphere zero locations
        // for (auto location : locs)
        // {
        //     LOG_TO_DEBUG(location->getName());
        // }
        LOG_TO_DEBUG("]");
    #endif
    if (locs.empty())
    {
        LOG_TO_DEBUG("Error: Not enough sphere zero locations to place items");
        return EntranceShuffleError::NOT_ENOUGH_SPHERE_ZERO_LOCATIONS;
    }

    if (entrancePlaced == nullptr)
    {
        // Check to make sure there's enough sphere 0 locations for the items that
        // are necessary to place at the beginning
        ItemPool itemPool2;
        LocationPool progressionLocations;
        GET_COMPLETE_ITEM_POOL(itemPool2, worlds);
        GET_COMPLETE_PROGRESSION_LOCATION_POOL(progressionLocations, worlds);
        determineMajorItems(worlds, itemPool2, progressionLocations);
        filterAndEraseFromPool(itemPool2, [&](const Item& item){return !item.isMajorItem();});
        // Run a dummy forward fill attempt until enough spots are open, then clear the worlds
        auto err = forwardFillUntilMoreFreeSpace(worlds, itemPool2, progressionLocations);
        clearWorlds(worlds);
        if (err != FillError::NONE)
        {
            return EntranceShuffleError::NOT_ENOUGH_SPHERE_ZERO_LOCATIONS;
        }
    }

    for (auto& world : worlds)
    {
        // Ensure that all race mode bosses are assigned to a single island
        auto& settings = world.getSettings();
        if (settings.progression_dungeons != ProgressionDungeons::Disabled && settings.num_required_dungeons > 0)
        {
            std::unordered_set<std::string> raceModeIslands = {};
            for (auto loc : world.raceModeLocations)
            {
                auto bossIslands = loc->accessPoints.front()->area->findIslands();

                if (bossIslands.size() > 1)
                {
                    #ifdef ENABLE_DEBUG
                        LOG_TO_DEBUG("Error: More than 1 island leading to race mode boss room " + loc->accessPoints.front()->area->name);
                        for (auto& island : bossIslands)
                        {
                            LOG_TO_DEBUG("\t" + island);
                        }
                    #endif
                    return EntranceShuffleError::AMBIGUOUS_RACE_MODE_ISLAND;
                }
            }
        }

        // Ensure that each dungeon's exit doesn't lead to a place that the player
        // might need to savewarp out of to escape. For now this is other dungeon entrances,
        // minibosses, and bosses.
        // Otherwise players could get sandwiched between two savewarp areas without 
        // the proper items to leave and not have the ability to savewarp back to the sea.
        for (auto& [dungeonName, dungeon] : world.dungeons)
        {
            if (dungeon.startingEntrance->getReverse())
            {
                if (const Entrance* dungeonExit = dungeon.startingEntrance->getReverse()->getReplaces(); dungeonExit != nullptr)
                {
                    // Dungeon reverse entrances are fine since those leave dungeons
                    if (isAnyOf(dungeonExit->getOriginalEntranceType(), 
                        EntranceType::DUNGEON, EntranceType::BOSS, EntranceType::BOSS_REVERSE, EntranceType::MINIBOSS, EntranceType::MINIBOSS_REVERSE))
                    {
                        return EntranceShuffleError::SAVEWARP_SANDWICH;
                    }
                }
            }
        }
    }

    return EntranceShuffleError::NONE;
}

// Attempt to connect the given entrance to the given target and verify that the
// new world graph is valid for this world's settings
static EntranceShuffleError replaceEntrance(WorldPool& worlds, Entrance* entrance, Entrance* target, std::vector<EntrancePair>& rollbacks, ItemPool& itemPool)
{
    LOG_TO_DEBUG("Attempting to Connect " + entrance->getOriginalName() + " To " + target->getReplaces()->getOriginalName());
    EntranceShuffleError err = EntranceShuffleError::NONE;
    err = checkEntrancesCompatibility(entrance, target, rollbacks);
    ENTRANCE_SHUFFLE_ERROR_CHECK(err);
    changeConnections(entrance, target);
    err = validateWorld(worlds, entrance, itemPool);
    // If the attempted replacement produces an invalid world graph, then undo
    // the attempted connection and try again with a different target.
    if (err != EntranceShuffleError::NONE)
    {
        if (entrance->getConnectedArea())
        {
            restoreConnections(entrance, target);
        }
        return err;
    }
    rollbacks.emplace_back(entrance, target);
    return EntranceShuffleError::NONE;
}

static EntranceShuffleError shuffleEntrances(WorldPool& worlds, EntrancePool& entrances, EntrancePool& targetEntrances, std::vector<EntrancePair>& rollbacks)
{

    ItemPool completeItemPool = {};
    GET_COMPLETE_ITEM_POOL(completeItemPool, worlds)

    shufflePool(entrances);

    // Place all entrances in the pool, validating worlds after each placement.
    // We first choose a random entrance from the list of entrances and attempt
    // to connect it with random target entrances from the target pool until
    // one of the connections produces a valid world graph.
    for (auto entrance : entrances)
    {
        EntranceShuffleError err = EntranceShuffleError::NONE;
        if (entrance->getConnectedArea())
        {
            continue;
        }
        shufflePool(targetEntrances);

        for (auto target : targetEntrances)
        {
            // If the target has already been disconnected, then don't use it again
            if (target->getConnectedArea() == nullptr)
            {
                continue;
            }
            err = replaceEntrance(worlds, entrance, target, rollbacks, completeItemPool);
            if (err == EntranceShuffleError::NONE)
            {
                break;
            }
        }

        if (entrance->getConnectedArea() == nullptr)
        {
            LOG_TO_DEBUG("Could not connect " + entrance->getOriginalName() + ". Error: " + errorToName(err));
            return EntranceShuffleError::NO_MORE_VALID_ENTRANCES;
        }
    }

    // Verify that all targets were disconnected and that we didn't create any closed root loops
    for (auto target : targetEntrances)
    {
        if (target->getConnectedArea())
        {
            LOG_TO_DEBUG("Error: Target entrance " + target->getCurrentName() + " was never disconnected");
            return EntranceShuffleError::FAILED_TO_DISCONNECT_TARGET;
        }
    }

    return EntranceShuffleError::NONE;
}

// Attempt to shuffle the given entrance pool with the given pool of target entrances
// When choosing connections randomly, it's possible that the algorithm can lock itself
// out of producing a valid world which is why we give it some number of retries
// in hopes that a complete failure with valid settings is exceedingly rare.
static EntranceShuffleError shuffleEntrancePool(World& world, WorldPool& worlds, EntrancePool& entrancePool, EntrancePool& targetEntrances, int retryCount = 20)
{
    EntranceShuffleError err = EntranceShuffleError::NONE;

    while (retryCount > 0)
    {
        retryCount--;
        std::vector<EntrancePair> rollbacks = {};

        err = shuffleEntrances(worlds, entrancePool, targetEntrances, rollbacks);
        if (err != EntranceShuffleError::NONE)
        {
            LOG_TO_DEBUG("Failed to place all entrances in a pool for world " + std::to_string(world.getWorldId() + 1) + ". Will retry " + std::to_string(retryCount) + " more times.");
            LOG_TO_DEBUG("Last Error: " + errorToName(err));
            for (auto& [entrance, target] : rollbacks)
            {
                restoreConnections(entrance, target);
            }
            continue;
        }
        for (auto& [entrance, target] : rollbacks)
        {
            confirmReplacement(entrance, target);
        }
        return EntranceShuffleError::NONE;
    }

    LOG_TO_DEBUG("Entrance placement attempt count exceeded for world " + std::to_string(world.getWorldId() + 1));
    return EntranceShuffleError::RAN_OUT_OF_RETRIES;
}

static EntranceShuffleError processPlandomizerEntrances(World& world)
{
    // Sanity check that the entrances put in for plandomizer are good
    for (auto& [originalEntranceStr, replacementEntranceStr] : world.plandomizer.entrancesStr)
    {
        const std::string originalTok = " -> ";
        const std::string replacementTok = " from ";

        // Verify that the format of each one is correct
        auto arrowPos = originalEntranceStr.find(originalTok);
        if (arrowPos == std::string::npos)
        {
            ErrorLog::getInstance().log("Plandomizer Error: Entrance plandomizer string \"" + originalEntranceStr + "\" is not properly formatted.");
            return EntranceShuffleError::PLANDOMIZER_ERROR;
        }
        auto fromPos = replacementEntranceStr.find(replacementTok);
        if (fromPos == std::string::npos)
        {
            ErrorLog::getInstance().log("Plandomizer Error: Entrance plandomizer string \"" + replacementEntranceStr + "\" is not properly formatted.");
            return EntranceShuffleError::PLANDOMIZER_ERROR;
        }

        // Separate out all the area names
        std::string originalEntranceParent = originalEntranceStr.substr(0, arrowPos);
        std::string originalEntranceConnection = originalEntranceStr.substr(arrowPos + originalTok.size());
        std::string replacementEntranceParent = replacementEntranceStr.substr(fromPos + replacementTok.size());
        std::string replacementEntranceConnection = replacementEntranceStr.substr(0, fromPos);

        Entrance* originalEntrance = world.getEntrance(originalEntranceParent, originalEntranceConnection);
        Entrance* replacementEntrance = world.getEntrance(replacementEntranceParent, replacementEntranceConnection);
        // Sanity check the entrance pointers
        if (originalEntrance == nullptr)
        {
            ErrorLog::getInstance().log("Plandomizer Error: Entrance \"" + originalEntranceStr + "\" is not a known connection.");
            return EntranceShuffleError::PLANDOMIZER_ERROR;
        }
        if (replacementEntrance == nullptr)
        {
            ErrorLog::getInstance().log("Plandomizer Error: Entrance \"" + replacementEntranceStr + "\" is not a known connection.");
            return EntranceShuffleError::PLANDOMIZER_ERROR;
        }

        world.plandomizer.entrances.insert({originalEntrance, replacementEntrance});
        LOG_TO_DEBUG("Plandomizer Entrance for world " + std::to_string(world.getWorldId() + 1) + " - " + originalEntranceStr + ": " + replacementEntranceStr);
    }

    return EntranceShuffleError::NONE;
}

static EntranceShuffleError setPlandomizerEntrances(World& world, WorldPool& worlds, EntrancePools& entrancePools, EntrancePools& targetEntrancePools, const std::set<EntranceType>& poolsToMix)
{
    LOG_TO_DEBUG("Now placing plandomized entrances");
    ItemPool completeItemPool = {};
    GET_COMPLETE_ITEM_POOL(completeItemPool, worlds)

    // Now attempt to connect plandomized entrances
    for (auto& [entrance, target] : world.plandomizer.entrances)
    {
        std::string fullConnectionName = "\"" + entrance->getOriginalName() + "\" to \"" + target->getOriginalConnectedArea()->name + " from " + target->getParentArea()->name + "\"";
        LOG_TO_DEBUG("Attempting to set plandomized entrance " + fullConnectionName);
        Entrance* entranceToConnect = entrance;
        Entrance* targetToConnect = target;

        auto type = entrance->getEntranceType();
        // If the entrance doesn't have a type, it's not shuffable
        if (type == EntranceType::NONE)
        {
            ErrorLog::getInstance().log("\"" + entrance->getOriginalName() + "\" is not an in-game entrance that can be shuffled.");
            return EntranceShuffleError::PLANDOMIZER_ERROR;
        }
        // Change misc restrictive to misc since restrictive entrances are still in the misc pool
        else if (type == EntranceType::MISC_RESTRICTIVE)
        {
            type = EntranceType::MISC;
        }

        // Check to make sure this type of entrance is being shuffled
        if (!entrancePools.contains(type))
        {
            // Check if its reverse is being shuffled if decoupled entrances are off
            if (!entrance->isDecoupled() && entrance->getReverse() != nullptr && entrancePools.contains(entrance->getReverse()->getEntranceType()))
            {
                // If this entrance has already been connected, throw an error
                if (entrance->getConnectedArea())
                {
                    ErrorLog::getInstance().log("Entrance \"" + entranceToConnect->getOriginalName() + "\" has already been connected. If you previously set the reverse of this entrance, you'll need to enabled the Decouple Entrances setting to plandomize this one also.");
                    return EntranceShuffleError::PLANDOMIZER_ERROR;
                }
                // If so, take the reverse of the entrance and target and attempt to connect them instead
                entranceToConnect = entrance->getReverse();
                targetToConnect = target->getReverse();
                type = entranceToConnect->getEntranceType();
                LOG_TO_DEBUG("Trying Reverse");
            }
            else
            {
                ErrorLog::getInstance().log("Entrance \"" + entrance->getOriginalName() + "\"'s type is not being shuffled and thus can't be plandomized.");
                return EntranceShuffleError::PLANDOMIZER_ERROR;
            }
        }

        // Get the appropriate pools (depending on if the pool is being mixed)
        auto& entrancePool = entrancePools[poolsToMix.contains(type) ? EntranceType::MIXED : type];
        auto& targetPool = targetEntrancePools[poolsToMix.contains(type) ? EntranceType::MIXED : type];

        if (elementInPool(entranceToConnect, entrancePool))
        {
            bool validTargetFound = false;
            for (auto targetEntrance : targetPool)
            {
                // Loop through until we find the *actual* target entrance with the valid
                // replacement for the entrance the user wants to connect
                if (targetToConnect == targetEntrance->getReplaces())
                {
                    std::vector<EntrancePair> dummyRollbacks = {};
                    EntranceShuffleError err = replaceEntrance(worlds, entranceToConnect, targetEntrance, dummyRollbacks, completeItemPool);
                    if (err != EntranceShuffleError::NONE)
                    {
                        ErrorLog::getInstance().log("Plandomizer Error when attempting to connect " + fullConnectionName + ": " + errorToName(err));
                        return EntranceShuffleError::PLANDOMIZER_ERROR;
                    }
                    validTargetFound = true;
                    // Remove the entrance and target from their pools when it's done
                    removeElementFromPool(entrancePool, entranceToConnect);
                    removeElementFromPool(targetPool, targetEntrance);
                    LOG_TO_DEBUG("Success");
                    break;
                }
            }
            if (!validTargetFound)
            {
                ErrorLog::getInstance().log("Entrance \"" + target->getOriginalConnectedArea()->name + " from " + target->getParentArea()->name + "\" is not a valid target for \"" + entrance->getOriginalName() + "\".");
                return EntranceShuffleError::PLANDOMIZER_ERROR;
            }
        }
        else
        {
            ErrorLog::getInstance().log("Entrance \"" + entrance->getOriginalName() + "\" is not being shuffled and thus can't be plandomized.");
            return EntranceShuffleError::PLANDOMIZER_ERROR;
        }

    }
    LOG_TO_DEBUG("All plandomizer entrances have been placed.");
    return EntranceShuffleError::NONE;
}

// Helper function for getting the reverse entrances from a given entrance pool
static EntrancePool getReverseEntrances(EntrancePools& entrancePools, EntranceType type)
{
    EntrancePool reversePool = {};
    for (auto entrance : entrancePools[type])
    {
        reversePool.push_back(entrance->getReverse());
    }
    return reversePool;
}

// Set all entrances in the passed in pools as shuffled
static void SetShuffledEntrances(EntrancePools& entrancePools) {
    for (auto& [type, entrancePool] : entrancePools)
    {
        for (auto entrance : entrancePool)
        {
            entrance->setAsShuffled();
            if (entrance->getReverse() != nullptr)
            {
                entrance->getReverse()->setAsShuffled();
            }
        }
    }
}

// static EntranceShuffleError resetToVanillaEntrances(World& world)
// {
//     LOG_TO_DEBUG("Resetting entrance connections");
//     // Reset random starting island
//     auto startingIsland = roomNumToIslandName(world.startingIslandRoomNum);
//
//     // Set original starting island in the world graph
//     auto linksSpawn = world.getEntrance("Link's Spawn", startingIsland);
//     if (linksSpawn == nullptr)
//     {
//         return EntranceShuffleError::BAD_LINKS_SPAWN;
//     }
//     linksSpawn->setConnectedArea("Outset Island");
//     world.getArea(startingIsland).entrances.remove(linksSpawn);
//     world.getArea("Outset Island").entrances.push_back(linksSpawn);
//     world.startingIslandRoomNum = 44;
//
//     // Reset all entrances that were shuffled
//     auto shuffledEntrances = world.getShuffledEntrances(EntranceType::ALL);
//     for (auto entrance : shuffledEntrances)
//     {
//         auto entranceOriginalName = entrance->getOriginalName();
//         auto originalConnection = entranceOriginalName.substr(entranceOriginalName.rfind("-> ") + 3, std::string::npos);
//         entrance->disconnect();
//         entrance->connect(originalConnection);
//         entrance->setReplaces(nullptr);
//         entrance->setAsUnshuffled();
//     }
//
//     return EntranceShuffleError::NONE;
// }

EntrancePools createEntrancePools(World& world, std::set<EntranceType>& poolsToMix)
{

    auto& settings = world.getSettings();

    // Only consider mixed pools as active if the entrance type and mixed pool setting is on
    bool mix_dungeons = settings.randomize_dungeon_entrances && settings.mix_dungeons;
    bool mix_bosses = settings.randomize_boss_entrances && settings.mix_bosses;
    bool mix_minibosses = settings.randomize_miniboss_entrances && settings.mix_minibosses;
    bool mix_caves = settings.randomize_cave_entrances != ShuffleCaveEntrances::Disabled && settings.mix_caves;
    bool mix_doors = settings.randomize_door_entrances && settings.mix_doors;
    bool mix_misc = settings.randomize_misc_entrances && settings.mix_misc;

    // Determine how many mixed pools there will be before determining which entrances will be randomized
    int totalMixedPools = (mix_dungeons ? 1 : 0) +
                          (mix_bosses ? 1 : 0) +
                          (mix_minibosses ? 1 : 0) +
                          (mix_caves ? 1 : 0) +
                          (mix_doors ? 1 : 0) +
                          (mix_misc ? 1 : 0);

    // Determine entrance pools based on settings, to be shuffled in the order we set them by
    EntrancePools entrancePools = {};

    // Save pools that we decouple to make code a little cleaner
    std::list<EntranceType> typesToDecouple = {};

    // Keep track of certain vanilla entrances that we want to manually connect
    // since we sometimes rely later on assuming that these entrances were set
    // in the entrance shuffling algorithm
    std::list<EntranceType> vanillaConnectionTypes = {};

    if (settings.randomize_dungeon_entrances)
    {
        entrancePools[EntranceType::DUNGEON] = world.getShuffleableEntrances(EntranceType::DUNGEON, true);
        if (settings.decouple_entrances)
        {
            entrancePools[EntranceType::DUNGEON_REVERSE] = getReverseEntrances(entrancePools, EntranceType::DUNGEON);
            typesToDecouple.push_back(EntranceType::DUNGEON);
            typesToDecouple.push_back(EntranceType::DUNGEON_REVERSE);
        }
    }
    else
    {
        vanillaConnectionTypes.push_back(EntranceType::DUNGEON);
    }

    if (settings.randomize_boss_entrances)
    {
        entrancePools[EntranceType::BOSS] = world.getShuffleableEntrances(EntranceType::BOSS, true);
        // Only decouple boss entrances when required bosses are off, or if caves/doors/misc entrances are mixed
        // as well
        if (settings.decouple_entrances && (settings.num_required_dungeons == 0 || (mix_bosses && (mix_doors || mix_caves || mix_misc))))
        {
            entrancePools[EntranceType::BOSS_REVERSE] = getReverseEntrances(entrancePools, EntranceType::BOSS);
            typesToDecouple.push_back(EntranceType::BOSS);
            typesToDecouple.push_back(EntranceType::BOSS_REVERSE);
        }
    }
    else
    {
        vanillaConnectionTypes.push_back(EntranceType::BOSS);
    }

    if (settings.randomize_miniboss_entrances)
    {
        entrancePools[EntranceType::MINIBOSS] = world.getShuffleableEntrances(EntranceType::MINIBOSS, true);
        if (settings.decouple_entrances)
        {
            entrancePools[EntranceType::MINIBOSS_REVERSE] = getReverseEntrances(entrancePools, EntranceType::MINIBOSS);
            typesToDecouple.push_back(EntranceType::MINIBOSS);
            typesToDecouple.push_back(EntranceType::MINIBOSS_REVERSE);
        }
    }
    else
    {
        vanillaConnectionTypes.push_back(EntranceType::MINIBOSS);
    }

    if (settings.randomize_cave_entrances != ShuffleCaveEntrances::Disabled)
    {
        entrancePools[EntranceType::CAVE] = world.getShuffleableEntrances(EntranceType::CAVE, true);
        
        // include fairy fountains with caves
        if(settings.randomize_cave_entrances == ShuffleCaveEntrances::CavesFairies) {
            auto fairyEntrances = world.getShuffleableEntrances(EntranceType::FAIRY, true);
            addElementsToPool(entrancePools[EntranceType::CAVE], fairyEntrances);
        }

        if (settings.decouple_entrances)
        {
            entrancePools[EntranceType::CAVE_REVERSE] = getReverseEntrances(entrancePools, EntranceType::CAVE);
            typesToDecouple.push_back(EntranceType::CAVE);
            typesToDecouple.push_back(EntranceType::CAVE_REVERSE);
        }
        // Don't randomize the cliff plateau upper isles grotto unless entrances are decoupled
        else
        {
            filterAndEraseFromPool(entrancePools[EntranceType::CAVE], [](const Entrance* e){return e->getParentArea()->name == "Cliff Plateau Highest Isle";});
        }
    }

    if (settings.randomize_door_entrances)
    {
        entrancePools[EntranceType::DOOR] = world.getShuffleableEntrances(EntranceType::DOOR, true);
        if (settings.decouple_entrances)
        {
            entrancePools[EntranceType::DOOR_REVERSE] = getReverseEntrances(entrancePools, EntranceType::DOOR);
            typesToDecouple.push_back(EntranceType::DOOR);
            typesToDecouple.push_back(EntranceType::DOOR_REVERSE);
        }
    }

    if (settings.randomize_misc_entrances)
    {
        // Allow both primary and non-primary entrances in the MISC pool unless
        // we're mixing the entrance pools and aren't decoupling entrances
        entrancePools[EntranceType::MISC] = world.getShuffleableEntrances(EntranceType::MISC, settings.mix_misc && totalMixedPools > 1 && !settings.decouple_entrances);
        auto miscRestrictiveEntrances = world.getShuffleableEntrances(EntranceType::MISC_RESTRICTIVE, !settings.decouple_entrances);
        addElementsToPool(entrancePools[EntranceType::MISC], miscRestrictiveEntrances);

        if (settings.decouple_entrances)
        {
            typesToDecouple.push_back(EntranceType::MISC);
        }
    }

    // Set collected entrance types as decoupled
    for (const auto& type : typesToDecouple)
    {
        for (auto entrance : entrancePools[type])
        {
            entrance->setAsDecoupled();
        }
    }

    // Assign collected vanilla entrance types
    for (const auto& type : vanillaConnectionTypes)
    {
        auto vanillaEntrances = world.getShuffleableEntrances(type, true);
        for (auto entrance : vanillaEntrances)
        {
            auto assumedForward = entrance->assumeReachable();
            if (entrance->getReverse() != nullptr && !entrance->isDecoupled())
            {
                auto assumedReturn = entrance->getReverse()->assumeReachable();
                assumedForward->bindTwoWay(assumedReturn);
            }
            changeConnections(entrance, assumedForward);
            confirmReplacement(entrance, assumedForward);
        }
    }

    SetShuffledEntrances(entrancePools);

    // Combine entrance pools if mixing pools. Only continue if more than one pool is selected
    if (totalMixedPools > 1)
    {
        CHECK_MIXED_POOL(mix_dungeons, EntranceType::DUNGEON);
        CHECK_MIXED_POOL(mix_bosses, EntranceType::BOSS);
        CHECK_MIXED_POOL(mix_minibosses, EntranceType::MINIBOSS);
        CHECK_MIXED_POOL(mix_doors, EntranceType::DOOR);
        CHECK_MIXED_POOL(mix_caves, EntranceType::CAVE);
        if (mix_misc)
        {
            poolsToMix.insert(EntranceType::MISC);
        }
        entrancePools[EntranceType::MIXED] = {};
        // For each entrance type, add the entrance to the mixed pool instead
        for (auto& [type, entrancePool] : entrancePools)
        {
            // Don't re-add the mixed pool to itself
            if (poolsToMix.contains(type) && type != EntranceType::MIXED)
            {
                addElementsToPool(entrancePools[EntranceType::MIXED], entrancePool);
                for (auto entrance : entrancePool)
                {
                    entrance->setEntranceType(EntranceType::MIXED);
                }
                entrancePools[type].clear();
            }
        }
    }

    return entrancePools;
}

EntrancePools createTargetEntrances(EntrancePools& entrancePools)
{
    // Create the pool of target entrances for each entrance type
    EntrancePools targetEntrancePools = {};
    for (auto& [type, entrancePool] : entrancePools)
    {
        targetEntrancePools[type] = assumeEntrancePool(entrancePool);
        #ifdef ENABLE_DEBUG
            logEntrancePool(targetEntrancePools[type], "Targets for entrance type " + entranceTypeToName(type));
        #endif
    }

    return targetEntrancePools;
}

EntranceShuffleError randomizeEntrances(WorldPool& worlds)
{
    EntranceShuffleError err = EntranceShuffleError::NONE;

    ItemPool completeItemPool;
    GET_COMPLETE_ITEM_POOL(completeItemPool, worlds);

    // Shuffle all entrances within their own world
    for (auto& world : worlds)
    {
        auto& settings = world.getSettings();
        // Set random starting island (either if the settings is set, or one is plandomized)
        if (settings.randomize_starting_island || world.plandomizer.startingIslandRoomNum != 0)
        {

            // Set plandomizer island if there is one
            if (world.plandomizer.startingIslandRoomNum != 0 && world.plandomizer.startingIslandRoomNum < 50)
            {
                world.startingIslandRoomNum = world.plandomizer.startingIslandRoomNum;
            }
            else
            {
                // Rooms 2 - 49 include every island except Forsaken Fortress
                world.startingIslandRoomNum = Random(2, 50);
            }

            auto startingIsland = roomNumToIslandName(world.startingIslandRoomNum);
            LOG_TO_DEBUG("starting island: \"" + startingIsland + "\" index: " + std::to_string(world.startingIslandRoomNum));

            // Set the new starting island in the world graph
            auto linksSpawn = world.getEntrance("Link's Spawn", "Outset Island");
            if (linksSpawn == nullptr)
            {
                return EntranceShuffleError::BAD_LINKS_SPAWN;
            }
            linksSpawn->setConnectedArea(world.getArea(startingIsland));
            world.getArea("Outset Island")->entrances.remove(linksSpawn);
            world.getArea(startingIsland)->entrances.push_back(linksSpawn);
        }

        // Set entrance data for all entrances, even those we aren't shuffling
        err = setAllEntrancesData(world);
        ENTRANCE_SHUFFLE_ERROR_CHECK(err);

        // Process plandomizer entrance data (but don't place plandomzier entrances yet)
        err = processPlandomizerEntrances(world);
        ENTRANCE_SHUFFLE_ERROR_CHECK(err);

        // This algorithm works similarly to the assumed fill algorithm used to
        // place items within the world. First, we disconnect all the entrances
        // that we're going to shuffle and "assume" that we have access to them
        // by creating a *target entrance* that corresponds to each disconnected
        // entrance. These target entrances are connected directly to the Root
        // of the world graph so that they have no requirements for access.

        std::set<EntranceType> poolsToMix;
        auto entrancePools = createEntrancePools(world, poolsToMix);
        auto targetEntrancePools = createTargetEntrances(entrancePools);

        // Set Plandomized entrances at this point
        err = setPlandomizerEntrances(world, worlds, entrancePools, targetEntrancePools, poolsToMix);
        if (err != EntranceShuffleError::NONE)
        {
            LOG_TO_DEBUG("| Encountered when setting plandomizer entrances");
            return err;
        }

        // Shuffle the entrances
        for (auto& [type, entrancePool] : entrancePools)
        {
            err = shuffleEntrancePool(world, worlds, entrancePool, targetEntrancePools[type]);
            if (err != EntranceShuffleError::NONE)
            {
                LOG_TO_DEBUG("| Encountered when shuffling pool of type " + entranceTypeToName(type));
                return err;
            }
        }

        // Now set the islands the race mode dungeons are in
        for (auto& [name, dungeon] : world.dungeons)
        {
            dungeon.islands = dungeon.startingArea->findIslands();
        }
    }

    // Validate the worlds one last time to ensure everything went okay
    err = validateWorld(worlds, nullptr, completeItemPool);
    ENTRANCE_SHUFFLE_ERROR_CHECK(err);

    return EntranceShuffleError::NONE;
}

const std::string errorToName(EntranceShuffleError err)
{
    switch (err)
    {
        case EntranceShuffleError::NONE:
            return "NONE";
        case EntranceShuffleError::BAD_LINKS_SPAWN:
            return "BAD_LINKS_SPAWN";
        case EntranceShuffleError::BAD_ENTRANCE_SHUFFLE_TABLE_ENTRY:
            return "BAD_ENTRANCE_SHUFFLE_TABLE_ENTRY";
        case EntranceShuffleError::RAN_OUT_OF_RETRIES:
            return "RAN_OUT_OF_RETRIES";
        case EntranceShuffleError::NO_MORE_VALID_ENTRANCES:
            return "NO_MORE_VALID_ENTRANCES";
        case EntranceShuffleError::ALL_LOCATIONS_NOT_REACHABLE:
            return "ALL_LOCATIONS_NOT_REACHABLE";
        case EntranceShuffleError::AMBIGUOUS_RACE_MODE_ISLAND:
            return "AMBIGUOUS_RACE_MODE_ISLAND";
        case EntranceShuffleError::AMBIGUOUS_RACE_MODE_DUNGEON:
            return "AMBIGUOUS_RACE_MODE_DUNGEON";
        case EntranceShuffleError::NO_RACE_MODE_ISLAND:
            return "NO_RACE_MODE_ISLAND";
        case EntranceShuffleError::NOT_ENOUGH_SPHERE_ZERO_LOCATIONS:
            return "NOT_ENOUGH_SPHERE_ZERO_LOCATIONS";
        case EntranceShuffleError::ATTEMPTED_SELF_CONNECTION:
            return "ATTEMPTED_SELF_CONNECTION";
        case EntranceShuffleError::FAILED_TO_DISCONNECT_TARGET:
            return "FAILED_TO_DISCONNECT_TARGET";
        case EntranceShuffleError::SAVEWARP_SANDWICH:
            return "SAVEWARP_SANDWICH";
        case EntranceShuffleError::PLANDOMIZER_ERROR:
            return "PLANDOMIZER_ERROR";
        default:
            return "UNKNOWN";
    }

}
