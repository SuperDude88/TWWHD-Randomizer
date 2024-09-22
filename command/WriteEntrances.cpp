#include "WriteEntrances.hpp"

#include <logic/Entrance.hpp>
#include <logic/World.hpp>
#include <utility/platform.hpp>
#include <filetypes/dzx.hpp>
#include <filetypes/events.hpp>
#include <command/Log.hpp>
#include <command/GamePath.hpp>
#include <command/RandoSession.hpp>

#include <gui/desktop/update_dialog_header.hpp>

bool restoreEntrances(WorldPool& worlds) {
    // Go through all the entrances and restore their data so they don't persist across seeds
    // restoreGameFile() does not need to check if the file is cached because it only tries to get the cache entry
    // Getting a cache entry doesn't overwrite anything if the file already had modifications
    const EntrancePool entrances = worlds[0].getShuffleableEntrances(EntranceType::ALL);
    for (const auto entrance : entrances)
    {
        const std::string fileStage = entrance->getFilepathStage();
        const uint8_t roomNum = entrance->getFilepathRoomNum();
        if(roomNum != 0xFF && entrance->getSclsExitIndex() != 0xFF) {
            if(const fspath path = getRoomFilePath(fileStage, roomNum); !g_session.restoreGameFile(path)) {
                ErrorLog::getInstance().log("Failed to restore " + path.string() + '\n');
                return false;
            }
        }

        // Savewarp update -> need to restore SCLS changes to stage.dzs
        // Warp update -> need to restore changes to event_list.dat (both are in stage archive)
        if (entrance->needsSavewarp() || entrance->hasWindWarp())
        {
            if(const fspath path = getStageFilePath(fileStage); !g_session.restoreGameFile(path)) {
                ErrorLog::getInstance().log("Failed to restore " + path.string() + '\n');
                return false;
            }
        }
    }

    return true;
}

bool writeEntrances(WorldPool& worlds) {
    Utility::platformLog("Saving entrances...");
    UPDATE_DIALOG_VALUE(45);
    UPDATE_DIALOG_LABEL("Saving entrances...");

    const EntrancePool entrances = worlds[0].getShuffledEntrances(EntranceType::ALL);
    for (const auto entrance : entrances)
    {
        const std::string fileStage = entrance->getFilepathStage();
        const uint8_t sclsExitIndex = entrance->getSclsExitIndex();
        std::string replacementStage = entrance->getReplaces()->getStageName();
        uint8_t replacementRoom = entrance->getReplaces()->getRoomNum();
        uint8_t replacementSpawn = entrance->getReplaces()->getSpawnId();

        // If this is the entrance that spawns the player at Ice Ring Inner Cave -> Ice Ring Isle, then
        // we want to change it so that it takes from the exit of the first ice ring interior
        if (entrance->getReplaces()->getOriginalName() == "Ice Ring Inner Cave -> Ice Ring Isle") {
            auto actualEntrance = worlds[0].getEntrance("Ice Ring Interior", "Ice Ring Isle");
            replacementStage = actualEntrance->getReplaces()->getStageName();
            replacementRoom = actualEntrance->getReplaces()->getRoomNum();
            replacementSpawn = actualEntrance->getReplaces()->getSpawnId();
        }

        fspath filepath = getRoomDzrPath(fileStage, entrance->getFilepathRoomNum());
        // Modify the kill triggers inside Fire Mountain and Ice Ring to act appropriately
        // "MiniKaz" is the Fire Mountain stage name
        // "MiniHyo" is the Ice Ring stage name
        if (replacementStage == "MiniKaz" || replacementStage == "MiniHyo") {
            const fspath exitFilepath = getRoomDzrPath(replacementStage, replacementRoom);
            RandoSession::CacheEntry& exitDzrEntry = g_session.openGameFile(exitFilepath);
            exitDzrEntry.addAction([entrance, replacementStage](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

                // Get the "VolTag" actor (otherwise known as the kill trigger)
                const std::vector<ChunkEntry*> actors = dzr.entries_by_type("ACTR");
                for (auto actor : actors) {
                    if (actor->data.substr(0, 6) == "VolTag") {
                        // If Fire Mountain/Ice Ring entrances lead to themselves, then don't change anything
                        if (entrance->getReplaces() == entrance) {
                        // If Fire Mountain leads to Ice Ring then change the kill trigger type to act like the one
                        // inside Fire Mountain
                        } else if (entrance->getStageName() == "MiniKaz" && replacementStage == "MiniHyo") {
                            actor->data[11] &= 0x3F;
                            actor->data[11] |= 1 << 6;
                        // If Ice Ring leads to Fire Mountain then change the kill trigger type to act like the one
                        // inside Ice Ring
                        } else if (entrance->getStageName() == "MiniHyo" && replacementStage == "MiniKaz") {
                            actor->data[11] &= 0x3F;
                            actor->data[11] |= 2 << 6;
                        // Otherwise, destroy the kill trigger so that players don't get thrown out immediately upon entering
                        } else {
                            dzr.remove_entity(actor);
                        }

                        break;
                    }
                }

                return true;
            });
        }

        // an SCLS Exit index of 255 indicates that there isn't a room file that we want to modify
        if (sclsExitIndex != 0xFF)
        {
            RandoSession::CacheEntry& dzrEntry = g_session.openGameFile(filepath);
            dzrEntry.addAction([fileStage, sclsExitIndex, replacementStage, replacementRoom, replacementSpawn](RandoSession* session, FileType* data) mutable -> int
            {
                CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

                const std::vector<ChunkEntry*> scls_entries = dzr.entries_by_type("SCLS");
                if(sclsExitIndex > (scls_entries.size() - 1)) {
                    ErrorLog::getInstance().log("SCLS entry index " + std::to_string(sclsExitIndex) + " outside of list in " + fileStage);
                    return false;
                }

                // Update the SCLS entry so that the player gets taken to the new entrance
                ChunkEntry* exit = scls_entries[sclsExitIndex];
                replacementStage.resize(8, '\0');
                exit->data.replace(0, 8, replacementStage.c_str(), 8);
                exit->data[8] = replacementSpawn;
                exit->data[9] = replacementRoom;

                return true;
            });
        }

        // If this entrance needs a savewarp update, then update the scls entry in the stage.dzs file
        if (entrance->needsSavewarp())
        {
            filepath = getStageFilePath(fileStage).concat("@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs@DZX");
            RandoSession::CacheEntry& dzsEntry = g_session.openGameFile(filepath);
            //sclsExitIndex = 0;
            dzsEntry.addAction([fileStage, entrance, replacementStage, replacementRoom, replacementSpawn](RandoSession* session, FileType* data) mutable -> int
            {
                CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

                // If this boss/miniboss room is accessed via a dungeon then set the savewarp
                // as the dungeon entrance
                auto& areaEntrances = entrance->getParentArea()->entrances;
                Entrance* replacementForThis = nullptr;
                for (auto e : areaEntrances)
                {
                    if (e->getReplaces() == entrance->getReverse())
                    {
                        replacementForThis = e;
                        break;
                    }
                }

                std::string dungeonName = replacementForThis->getParentArea()->dungeon;
                if (dungeonName != "") {
                    auto dungeon = entrance->getWorld()->getDungeon(dungeonName);
                    replacementStage = dungeon.savewarpStage;
                    replacementRoom = dungeon.savewarpRoom;
                    replacementSpawn = dungeon.savewarpSpawn;
                } else if (entrance->isDecoupled()) {
                    // If the entrance is decoupled, then set the savewarp as the reverse
                    // of the entrance used to enter the boss/miniboss room
                    auto reverse = replacementForThis->getReverse();
                    replacementStage = reverse->getStageName();
                    replacementRoom = reverse->getRoomNum();
                    replacementSpawn = reverse->getSpawnId();
                }

                const std::vector<ChunkEntry*> scls_entries = dzr.entries_by_type("SCLS");
                if(scls_entries.size() == 0) {
                    ErrorLog::getInstance().log(fileStage + " stage dzs missing savewarp exit!");
                    return false;
                }

                // Update the SCLS entry so that the player savewarps to the right place
                ChunkEntry* exit = scls_entries[0];
                replacementStage.resize(8, '\0');
                exit->data.replace(0, 8, replacementStage.c_str(), 8);
                exit->data[8] = replacementSpawn;
                exit->data[9] = replacementRoom;

                return true;
            });
        }
    }

    // Update wind warp exits appropriately
    std::list<Entrance*> bossReverseEntrances = {};
    for (auto& [areaName, area] : worlds[0].areaTable) {
        for (auto& exit : area->exits) {
            if (exit.hasWindWarp()) {
                bossReverseEntrances.push_back(&exit);
            }
        }
    }

    for (auto entrance : bossReverseEntrances) {
        std::string replacementStage = entrance->getReplaces()->getStageName();
        uint8_t replacementRoom = entrance->getReplaces()->getRoomNum();
        uint8_t replacementSpawn = entrance->getReplaces()->getSpawnId();
        if (!entrance->isDecoupled()) {
            // If this boss room is connected to a dungeon, then send the player
            // back out the exit of the dungeon
            auto dungeonName = entrance->getReplaces()->getReverse()->getConnectedArea()->dungeon;
            if (dungeonName != "") {
                auto dungeonExit = entrance->getWorld()->getDungeon(dungeonName).startingEntrance->getReverse();
                // If dungeon entrances are not mixed, and misc entrances aren't shuffled
                // then send players back to the appropriate natural warp wind exit
                auto& settings = entrance->getWorld()->getSettings();
                if (!settings.mix_dungeons && !settings.randomize_misc_entrances) {
                    // Get the warp wind exit of the dungeon entrance that was randomized to this dungeon
                    // If dungeons aren't randomized it'll just return the same one
                    Dungeon dungeon;
                    if (dungeonName == "Forsaken Fortress") {
                        dungeon = entrance->getWorld()->getDungeon("Forsaken Fortress");
                    } else {
                        dungeon = entrance->getWorld()->getDungeon(dungeonExit->getReplaces()->getReverse()->getOriginalConnectedArea()->dungeon);
                    }
                        
                    replacementStage = dungeon.windWarpExitStage;
                    replacementRoom = dungeon.windWarpExitRoom;
                    replacementSpawn = dungeon.windWarpExitSpawn;
                } else {
                    if (dungeonName == "Forsaken Fortress") {
                        replacementStage = "sea";
                        replacementRoom = 1;
                        replacementSpawn = 0;
                    }
                    else {
                        replacementStage = dungeonExit->getReplaces()->getStageName();
                        replacementRoom = dungeonExit->getReplaces()->getRoomNum();
                        replacementSpawn = dungeonExit->getReplaces()->getSpawnId();
                    }
                }
            } 
        }

        const fspath filepath = getStageFilePath(entrance->getFilepathStage()).concat("@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat@EVENTS");
        RandoSession::CacheEntry& list = g_session.openGameFile(filepath);
        list.addAction([filepath, replacementRoom, replacementSpawn, replacementStage](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(event_list, FileTypes::EventList, data)

            if(event_list.Events_By_Name.count("WARP_WIND_AFTER") == 0) {
                ErrorLog::getInstance().log("No Event WARP_WIND_AFTER in " + filepath.string());
                return false;
            }

            std::shared_ptr<Action> exit2 = event_list.Events_By_Name.at("WARP_WIND_AFTER")->get_actor("DIRECTOR")->actions[2];
            std::get<std::vector<int32_t>>(exit2->properties[0]->value)[0] = replacementSpawn;
            exit2->properties[1]->value = replacementStage + "\0";
            std::get<std::vector<int32_t>>(exit2->properties[2]->value)[0] = replacementRoom;

            return true;
        });
    }

    return true;
}
