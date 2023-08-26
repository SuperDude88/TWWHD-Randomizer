#include <stdint.h>
#include <tweaks.hpp>

#include <cstring>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <thread>

#include <libs/tinyxml2.h>
#include <libs/zlib-ng/zlib-ng.h>
#include <seedgen/config.hpp>
#include <seedgen/random.hpp>
#include <seedgen/seed.hpp>
#include <seedgen/permalink.hpp>
#include <logic/SpoilerLog.hpp>
#include <logic/Generate.hpp>
#include <logic/mass_test.hpp>
#include <filetypes/dzx.hpp>
#include <filetypes/charts.hpp>
#include <filetypes/events.hpp>
#include <command/WriteLocations.hpp>
#include <command/RandoSession.hpp>
#include <command/Log.hpp>
#include <utility/platform.hpp>
#include <utility/file.hpp>
#include <utility/endian.hpp>
#include <nuspack/packer.hpp>

#include <gui/update_dialog_header.hpp>

#ifdef DEVKITPRO
#include <sysapp/title.h>
#include <platform/channel.hpp>
#include <platform/controller.hpp>
#endif

#define SEED_KEY "SEED KEY TEST"

RandoSession g_session; //declared outside of class for extern stuff

#define FILETYPE_ERROR_CHECK(func) {  \
    if(const auto error = func; error != decltype(error)::NONE) {\
        ErrorLog::getInstance().log(std::string("Encountered ") + &(typeid(error).name()[5]) + " on line " TOSTRING(__LINE__)); \
        return false;  \
    } \
}

class Randomizer {
private:
    Config config;
    std::string permalink;
    size_t integer_seed;

    #ifdef FILL_TESTING
        bool dryRun = true;
    #else
        bool dryRun = false;
    #endif
    //bool randomizeItems = true; not currently used
    unsigned int numPlayers = 1;
    //int playerId = 1;

    [[nodiscard]] bool verifyBase() {
        using namespace std::filesystem;

        Utility::platformLog("Verifying dump...\n");
        UPDATE_DIALOG_LABEL("Verifying dump...");
        UPDATE_DIALOG_VALUE(25);

        const RandoSession::fspath& base = g_session.getBaseDir();
        if(!is_directory(base / "code") || !is_directory(base / "content") || !is_directory(base / "meta")) {
            ErrorLog::getInstance().log("Invalid base path: could not find code/content/meta folders at " + base.string() + "!");
            return false;
        }

        //Check the meta.xml for other platforms (+ a sanity check on console)
        tinyxml2::XMLDocument metaXml;
        const RandoSession::fspath& metaPath = g_session.getBaseDir() / "meta/meta.xml";
        if(!is_regular_file(metaPath)) {
            ErrorLog::getInstance().log("Failed finding meta.xml");
            return false;
        }

        if(tinyxml2::XMLError err = metaXml.LoadFile(metaPath.string().c_str()); err != tinyxml2::XMLError::XML_SUCCESS) {
            ErrorLog::getInstance().log("Could not parse meta.xml, got error " + std::to_string(err));
            return false;
        }
        const tinyxml2::XMLElement* root = metaXml.RootElement();

        const std::string titleId = root->FirstChildElement("title_id")->GetText();
        const std::string nameEn = root->FirstChildElement("longname_en")->GetText();
        if(titleId != "0005000010143500" || nameEn != "THE LEGEND OF ZELDA\nThe Wind Waker HD")  {
            ErrorLog::getInstance().log("meta.xml does not match base game - dump is not valid");
            ErrorLog::getInstance().log("ID " + titleId);
            ErrorLog::getInstance().log("Name " + nameEn);
            return false;
        }

        const std::string region = root->FirstChildElement("region")->GetText();
        if(region != "00000002") {
            ErrorLog::getInstance().log("Incorrect region - game must be a NTSC-U / US copy");
            return false;
        }

        return true;
    }

    [[nodiscard]] bool verifyOutput() {
        using namespace std::filesystem;

        Utility::platformLog("Verifying output...\n");
        UPDATE_DIALOG_LABEL("Verifying output...");
        UPDATE_DIALOG_VALUE(25);
        
        const RandoSession::fspath& out = g_session.getOutputDir();
        if(!is_directory(out / "code") || !is_directory(out / "content") || !is_directory(out / "meta")) {
            ErrorLog::getInstance().log("Invalid output path: could not find code/content/meta folders at " + out.string() + "!");
            return false;
        }

        //Double check the meta.xml
        tinyxml2::XMLDocument metaXml;
        const RandoSession::fspath& metaPath = out / "meta/meta.xml";
        if(!is_regular_file(metaPath)) {
            ErrorLog::getInstance().log("Failed finding meta.xml");
            return false;
        }

        if(tinyxml2::XMLError err = metaXml.LoadFile(metaPath.string().c_str()); err != tinyxml2::XMLError::XML_SUCCESS) {
            ErrorLog::getInstance().log("Could not parse meta.xml, got error " + std::to_string(err));
            return false;
        }
        const tinyxml2::XMLElement* root = metaXml.RootElement();

        //Title ID won't be updated until after the first randomization on PC
        //But on console it should be correct once the channel is installed
        #ifdef DEVKITPRO
            const std::string titleId = root->FirstChildElement("title_id")->GetText();
            if(titleId != "0005000010143599")  {
                ErrorLog::getInstance().log("meta.xml does not match - custom channel is not valid");
                ErrorLog::getInstance().log("ID " + titleId);
                return false;
            }
        #endif

        const std::string region = root->FirstChildElement("region")->GetText();
        if(region != "00000002") {
            ErrorLog::getInstance().log("Incorrect region - game must be a NTSC-U / US copy");
            return false;
        }

        return true;
    }

    void clearOldLogs() {
        if(std::filesystem::is_regular_file(APP_SAVE_PATH "Debug Log.txt")) {
            std::filesystem::remove(APP_SAVE_PATH "Debug Log.txt");
        }
        if(std::filesystem::is_regular_file(APP_SAVE_PATH "Error Log.txt")) {
            std::filesystem::remove(APP_SAVE_PATH "Error Log.txt");
        }
        if(std::filesystem::is_regular_file(APP_SAVE_PATH "Non-Spoiler Log.txt")) {
            std::filesystem::remove(APP_SAVE_PATH "Non-Spoiler Log.txt");
        }
        if(std::filesystem::is_regular_file(APP_SAVE_PATH "Spoiler Log.txt")) {
            std::filesystem::remove(APP_SAVE_PATH "Spoiler Log.txt");
        }

        return;
    }

    [[nodiscard]] bool writeCharts(WorldPool& worlds) {
        using namespace std::literals::string_literals;
        using eType = Utility::Endian::Type;

        Utility::platformLog("Saving randomized charts...\n");
        UPDATE_DIALOG_LABEL("Saving randomized charts...");

        RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Misc/Misc.szs@YAZ0@SARC@Misc.bfres@BFRES@cmapdat.bin@CHARTS");

        static const std::unordered_set<std::string> salvage_object_names = {
            "Salvage\0"s,
            "SwSlvg\0\0"s,
            "Salvag2\0"s,
            "SalvagN\0"s,
            "SalvagE\0"s,
            "SalvFM\0\0"s
        };

        for (uint8_t i = 0; i < 49; i++) {
            const uint8_t islandNumber = i + 1;

            std::string dzrPath = get_island_room_dzx_filepath(islandNumber);
            RandoSession::CacheEntry& dzrEntry = g_session.openGameFile(dzrPath);

            entry.addAction([&worlds, &dzrEntry, i, islandNumber](RandoSession* session, FileType* data) -> int
            {
                CAST_ENTRY_TO_FILETYPE(charts, FileTypes::ChartList, data)
                static const auto original_charts = charts.charts;

                const Chart& original_chart = *std::find_if(original_charts.begin(), original_charts.end(), [islandNumber](const Chart& chart) {return (chart.type == 0 || chart.type == 1 || chart.type == 2 || chart.type == 5 || chart.type == 6 || chart.type == 8) && chart.getIslandNumber() == islandNumber; });
                const GameItem new_chart_item = worlds[0].chartMappings[i];
                const auto new_chart = std::find_if(charts.charts.begin(), charts.charts.end(), [&](const Chart& chart) {return chart.getItem() == new_chart_item;});
                if(new_chart == charts.charts.end()) return false;

                new_chart->texture_id = original_chart.texture_id;
                new_chart->sector_x = original_chart.sector_x;
                new_chart->sector_y = original_chart.sector_y;

                //Probably not needed on HD since they removed chart sets, but update anyway
                for(uint8_t pos_index = 0; pos_index < 4; pos_index++) {
                    ChartPos& new_pos = new_chart->possible_positions[pos_index];
                    const ChartPos& original_pos = original_chart.possible_positions[pos_index];

                    new_pos.tex_x_offset = original_pos.tex_x_offset;
                    new_pos.tex_y_offset = original_pos.tex_y_offset;
                    new_pos.salvage_x_pos = original_pos.salvage_x_pos;
                    new_pos.salvage_y_pos = original_pos.salvage_y_pos;
                }

                dzrEntry.addAction([new_chart = *new_chart](RandoSession* session, FileType* data) -> int
                {
                    CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

                    for(ChunkEntry* scob : dzr.entries_by_type("SCOB")) {
                        if(salvage_object_names.count(scob->data.substr(0, 8)) > 0 && ((scob->data[8] & 0xF0) >> 4) == 0) {
                            uint32_t& params = *reinterpret_cast<uint32_t*>(&scob->data[8]);
                            Utility::Endian::toPlatform_inplace(eType::Big, params);
                            const uint32_t mask = 0x0FF00000;
                            const uint8_t shiftAmount = 20;

                            params = (params & (~mask)) | (uint32_t(new_chart.owned_chart_index_plus_1 << shiftAmount) & mask);
                            Utility::Endian::toPlatform_inplace(eType::Big, params);
                        }
                    }

                    return true;
                });

                return true;
            });

            entry.addDependent(dzrEntry.getRoot());
        }

        return true;
    }

    [[nodiscard]] bool restoreEntrances() {
        //Needs to restore data before anything is modified, worlds are generated after pre-randomization tweaks are applied
        //Get around this with a list of the entrance paths
        //Skip anything rando does edit to save time, they get restored during extraction
        static const std::list<std::pair<std::string, uint8_t>> vanillaEntrancePaths = {
            {"Adanmae",  0},
            {"M_NewD2",  0},
            //{"sea",     41},
            {"kindan",   0},
            //{"sea",     26},
            {"Siren",    0},
            {"Edaichi",  0},
            {"M_Dai",    0},
            {"Ekaze",    0},
            {"kaze",    15},
            //{"sea",     44},
            {"Cave09",   0},
            //{"sea",     13},
            {"TF_06",    0},
            //{"sea",     20},
            {"MiniKaz",  0},
            //{"sea",     40},
            {"MiniHyo",  0},
            {"Abesso",   0},
            {"TF_04",    0},
            //{"sea",     29},
            {"SubD42",   0},
            //{"sea",     47},
            {"SubD43",   0},
            //{"sea",     48},
            {"SubD71",   0},
            //{"sea",     31},
            {"TF_01",    0},
            //{"sea",      7},
            {"TF_02",    0},
            //{"sea",     35},
            {"TF_03",    0},
            //{"sea",     12},
            {"TyuTyu",   0},
            //{"sea",     12},
            {"Cave07",   0},
            //{"sea",     36},
            {"WarpD",    0},
            //{"sea",     34},
            {"Cave01",   0},
            //{"sea",     16},
            {"Cave04",   0},
            //{"sea",     38},
            {"ITest62",  0},
            {"ITest63",  0},
            //{"sea",     42},
            {"Cave03",   0},
            //{"sea",     42},
            {"Cave03",   0},
            //{"sea",     43},
            {"Cave05",   0},
            //{"sea",      2},
            {"Cave02",   0},
            //{"sea",     11},
            {"Pnezumi",  0},
            //{"sea",     11},
            {"Nitiyou",  0},
            //{"sea",     11},
            {"Ocmera",   0},
            //{"sea",     11},
            {"Ocmera",   0},
            //{"sea",     11},
            {"Opub",     0},
            //{"sea",     11},
            {"Kaisen",   0},
            //{"sea",     11},
            {"Kaisen",   0},
            //{"sea",     11},
            //{"Orichh",   0},
            //{"sea",     11},
            //{"Orichh",   0},
            //{"sea",     11},
            {"Pdrgsh",   0},
            //{"sea",     11},
            {"Obombh",   0},
            {"Atorizk",  0},
            {"Comori",   0},
            //{"sea",     33},
            {"Abesso",   0},
            //{"sea",     44},
            {"LinkRM",   0},
            //{"sea",     44},
            {"Ojhous",   0},
            //{"sea",     44},
            {"Ojhous2",  1},
            //{"sea",     44},
            {"Onobuta",  0},
            //{"sea",     44},
            {"Omasao",   0},
            //{"sea",      4},
            {"Ekaze",    0},
            //{"sea",     11},
            {"Obombh",   0},
            //{"sea",     13},
            {"Atorizk",  0},
            //{"sea",     13},
            {"Atorizk",  0},
            {"Adanmae",  0},
            {"Atorizk",  0},
            {"Atorizk",  0},
            {"Adanmae",  0},
            //{"sea",     30},
            {"ShipD",    0},
            //{"sea",     41},
            //{"Omori",    0},
            //{"sea",     41},
            {"Otkura",   0},
            //{"Omori",    0},
            {"Ocrogh",   0},
            //{"sea",     41},
            //{"Omori",    0},
            //{"sea",     41},
            //{"Omori",    0},
            //{"sea",     41},
            //{"Omori",    0},
            //{"sea",     41},
            //{"Omori",    0},
            //{"sea",     44},
            {"LinkUG",   0},
            //{"sea",     44},
            {"A_mori",   0},
            //{"sea",     44},
            //{"Pjavdou",  0},
            //{"sea",     45},
            {"Edaichi",  0}
        };

        const std::unordered_set<uint8_t> pack1 = {0, 1, 11, 13, 17, 23};
        const std::unordered_set<uint8_t> pack2 = {9, 39, 41, 44};
        std::unordered_set<std::string> paths;

        for (const auto& [fileStage, roomNum] : vanillaEntrancePaths)
        {
            const std::string fileRoom = std::to_string(roomNum);
            const std::string filepath = "content/Common/Stage/" + fileStage + "_Room" + fileRoom + ".szs";

            if (fileStage == "sea") {
                if (pack1.count(roomNum) > 0) {
                    continue; //pack files are edited elsewhere which restores them
                }
                else if (pack2.count(roomNum) > 0) {
                    continue; //pack files are edited elsewhere which restores them
                }
            }

            paths.emplace(filepath);
        }

        // Also include boss rooms
        for (const std::string& bossStage : {"M_DragB", "kinBOSS", "SirenB", "M_DaiB", "kazeB"}) {
            paths.emplace("content/Common/Stage/" + bossStage + "_Stage.szs");
        }

        for(const std::string& path : paths) {
            if(!g_session.restoreGameFile(path)) {
                ErrorLog::getInstance().log("Failed to restore " + path + '\n');
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] bool writeEntrances(WorldPool& worlds) {
        Utility::platformLog("Saving entrances...\n");
        UPDATE_DIALOG_VALUE(45);
        UPDATE_DIALOG_LABEL("Saving entrances...");

        const std::unordered_set<uint8_t> pack1 = {0, 1, 11, 13, 17, 23};
        const std::unordered_set<uint8_t> pack2 = {9, 39, 41, 44};

        const EntrancePool entrances = worlds[0].getShuffledEntrances(EntranceType::ALL);
        for (const auto entrance : entrances)
        {
            const std::string fileStage = entrance->getFilepathStage();
            const std::string fileRoom = std::to_string(entrance->getFilepathRoomNum());
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

            std::string filepath = "content/Common/Stage/" + fileStage + "_Room" + fileRoom + ".szs@YAZ0@SARC@Room" + fileRoom + ".bfres@BFRES@room.dzr@DZX";

            if (fileStage == "sea") {
                if (pack1.count(entrance->getFilepathRoomNum()) > 0) {
                    filepath = "content/Common/Pack/szs_permanent1.pack@SARC@" + fileStage + "_Room" + fileRoom + ".szs@YAZ0@SARC@Room" + fileRoom + ".bfres@BFRES@room.dzr@DZX";
                }
                else if (pack2.count(entrance->getFilepathRoomNum()) > 0) {
                    filepath = "content/Common/Pack/szs_permanent2.pack@SARC@" + fileStage + "_Room" + fileRoom + ".szs@YAZ0@SARC@Room" + fileRoom + ".bfres@BFRES@room.dzr@DZX";
                }
            }

            // Room number of 0xFF in the entrance shuffle table indicates
            // that the SCLS entry is in the stage.dzs file instead of room.dzr
            if (entrance->getFilepathRoomNum() == 0xFF) {
                filepath = "content/Common/Stage/" + fileStage + "_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs@DZX";                 
            }

            RandoSession::CacheEntry& dzrEntry = g_session.openGameFile(filepath);

            // Modify the kill triggers inside Fire Mountain and Ice Ring to act appropriately
            // "MiniKaz" is the Fire Mountain stage name
            // "MiniHyo" is the Ice Ring stage name
            if (replacementStage == "MiniKaz" || replacementStage == "MiniHyo") {
                std::string exitFilepath = "content/Common/Stage/" + replacementStage + "_Room" + std::to_string(replacementRoom) + ".szs@YAZ0@SARC@Room" + std::to_string(replacementRoom) + ".bfres@BFRES@room.dzr@DZX";
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

            dzrEntry.addAction([entrance, sclsExitIndex, replacementStage, replacementRoom, replacementSpawn](RandoSession* session, FileType* data) mutable -> int
            {
                CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

                // If this is the savewarp exit of a boss room, update it appropriately
                if (entrance->getEntranceType() == EntranceType::BOSS_REVERSE) {
                    // If this boss room is accessed via a dungeon then set the savewarp
                    // as the dungeon entrance
                    auto dungeonName = entrance->getReplaces()->getReverse()->getConnectedAreaEntry()->dungeon;
                    if (dungeonName != "") {
                        auto dungeonEntrance = entrance->getWorld()->getDungeon(dungeonName).startingEntrance;
                        replacementStage = dungeonEntrance->getStageName();
                        replacementRoom = dungeonEntrance->getRoomNum();
                        replacementSpawn = dungeonEntrance->getSpawnId();

                    } else if (entrance->isDecoupled()) {
                        // If the entrance is decoupled, then set the savewarp as the reverse
                        // of the entrance used to enter the boss room
                        auto reverse = entrance->getReplaces()->getReverse();
                        replacementStage = reverse->getStageName();
                        replacementRoom = reverse->getRoomNum();
                        replacementSpawn = reverse->getSpawnId();
                    }
                }

                const std::vector<ChunkEntry*> scls_entries = dzr.entries_by_type("SCLS");
                if(sclsExitIndex > (scls_entries.size() - 1)) {
                    ErrorLog::getInstance().log("SCLS entry index outside of list!");
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

        // Update warp wind exits appropriately
        std::list<Entrance*> bossReverseEntrances = {};
        for (auto& [areaName, area] : worlds[0].areaEntries) {
            for (auto& exit : area.exits) {
                if (exit.getEntranceType() == EntranceType::BOSS_REVERSE) {
                    bossReverseEntrances.push_back(&exit);
                }
            }
        }

        for (auto entrance : bossReverseEntrances) {

            const std::string fileStage = entrance->getFilepathStage();
            const uint8_t sclsExitIndex = entrance->getSclsExitIndex();
            std::string replacementStage = entrance->getReplaces()->getStageName();
            uint8_t replacementRoom = entrance->getReplaces()->getRoomNum();
            uint8_t replacementSpawn = entrance->getReplaces()->getSpawnId();

            if (!entrance->isDecoupled()) {
                // If this boss room is connected to a dungeon, then send the player
                // back out the exit of the dungeon
                auto dungeonName = entrance->getReplaces()->getReverse()->getConnectedAreaEntry()->dungeon;
                if (dungeonName != "") {
                    auto dungeonExit = entrance->getWorld()->getDungeon(dungeonName).startingEntrance->getReverse();

                    // If dungeon entrances are not mixed, and misc entrances aren't shuffled
                    // then send players back to the appropriate natural warp wind exit
                    auto& settings = entrance->getWorld()->getSettings();
                    if (!settings.mix_dungeons && !settings.randomize_misc_entrances) {
                        // Get the warp wind exit of the dungeon entrance that was randomized to this dungeon
                        // If dungeons aren't randomized it'll just return the same one
                        auto warpWindExit = dungeonExit->getReplaces()->getReverse()->getReplaces()->getReverse();
                        replacementStage = warpWindExit->getBossOutStageName();
                        replacementRoom = warpWindExit->getBossOutRoomNum();
                        replacementSpawn = warpWindExit->getBossOutSpawnId();
                    } else {
                        replacementStage = dungeonExit->getReplaces()->getStageName();
                        replacementRoom = dungeonExit->getReplaces()->getRoomNum();
                        replacementSpawn = dungeonExit->getReplaces()->getSpawnId();
                    }
                } 
            }

            auto bossStage = entrance->getFilepathStage();
            const std::string filepath = "content/Common/Stage/" + bossStage + "_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat@EVENTS";

            RandoSession::CacheEntry& list = g_session.openGameFile(filepath);
            list.addAction([entrance, filepath, replacementRoom, replacementSpawn, replacementStage](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(event_list, FileTypes::EventList, data)

                if(event_list.Events_By_Name.count("WARP_WIND_AFTER") == 0) {
                    ErrorLog::getInstance().log("No Event WARP_WIND_AFTER in " + filepath);
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
public:
    Randomizer(const Config& config_) :
        config(config_),
        permalink(create_permalink(config_.settings, config_.seed))
    {
        g_session.init(config.gameBaseDir, config.outputDir);
        Utility::platformLog("Initialized session\n");
    }

    int randomize() {

        // Go through the setting testing process if mass testing is turned on and ignore everything else
        #ifdef MASS_TESTING
            #if TEST_COUNT
                testSettings(config, TEST_COUNT);
            #else
                massTest(config);
            #endif
            return 0;
        #endif

        std::hash<std::string> strHash;

        LOG_TO_DEBUG("Permalink: " + permalink);

        if(config.settings.do_not_generate_spoiler_log) permalink += SEED_KEY;

        // Add the plandomizer file contents to the permalink when plandomzier is enabled
        if (config.settings.plandomizer) {
            std::string plandoContents;
            if (Utility::getFileContents(config.settings.plandomizerFile, plandoContents) != 0) {
                ErrorLog::getInstance().log("Could not find plandomizer file at\n" + config.settings.plandomizerFile);
                return 1;
            }
            permalink += plandoContents;
        }

        // Seed RNG
        integer_seed = zng_crc32(0L, (uint8_t*)permalink.data(), permalink.length());

        Random_Init(integer_seed);

        LogInfo::setConfig(config);
        LogInfo::setSeedHash(generate_seed_hash());

        // clearOldLogs();

        // Create all necessary worlds (for any potential multiworld support in the future)
        WorldPool worlds(numPlayers);
        std::vector<Settings> settingsVector (numPlayers, config.settings);

        Utility::platformLog("Randomizing...\n");
        UPDATE_DIALOG_VALUE(5);
        if (generateWorlds(worlds, settingsVector) != 0) {
            // generating worlds failed
            // Utility::platformLog("An Error occurred when attempting to generate the worlds. Please see the Error Log for details.\n");
            return 1;
        }

        generateNonSpoilerLog(worlds);
        if (!config.settings.do_not_generate_spoiler_log) {
            generateSpoilerLog(worlds);
        }

        // Skip all game modification stuff if we're doing a dry run (fill testing)
        if (dryRun) return 0;

        if(!verifyBase()) {
            return 1;
        }
        if(!verifyOutput()) {
            return 1;
        }

        //IMPROVEMENT: custom model things

        Utility::platformLog("Modifying game code...\n");
        UPDATE_DIALOG_VALUE(30);
        UPDATE_DIALOG_LABEL("Modifying game code...");
        // TODO: update worlds indexing for multiworld eventually
        if(TweakError err = apply_necessary_tweaks(worlds[0].getSettings()); err != TweakError::NONE) {
            ErrorLog::getInstance().log("Encountered error in pre-randomization tweaks!");
            return 1;
        }

        // Flatten the playthrough into a single list
        // so that chests can check it for CTMC
        std::list<Location*> playthroughLocations = {};
        for (const auto& sphere : worlds[0].playthroughSpheres) {
            for (auto loc : sphere) {
                playthroughLocations.push_back(loc);
            }
        }

        // Assume 1 world for now, modifying multiple copies needs work
        Utility::platformLog("Saving items...\n");
        UPDATE_DIALOG_VALUE(40);
        UPDATE_DIALOG_LABEL("Saving items...");
        ModifyChest::setCTMC(config.settings.chest_type_matches_contents, config.settings.progression_dungeons == ProgressionDungeons::RaceMode, worlds[0].dungeons, playthroughLocations);
        for (auto& [name, location] : worlds[0].locationEntries) {
            if (ModificationError err = location.method->writeLocation(location.currentItem); err != ModificationError::NONE) {
                ErrorLog::getInstance().log("Failed to save location " + location.getName());
                return 1;
            }
        }

        // Write charts after saving our items so the hardcoded offsets don't change
        // Charts + entrances look through the actor list so offsets don't matter for these
        if(config.settings.randomize_charts) {
            if(!writeCharts(worlds)) {
                ErrorLog::getInstance().log("Failed to save charts!");
                return 1;
            }
        }

        if (config.settings.randomize_cave_entrances || config.settings.randomize_door_entrances || config.settings.randomize_dungeon_entrances || config.settings.randomize_misc_entrances) {
            if(!writeEntrances(worlds)) {
                ErrorLog::getInstance().log("Failed to save entrances!");
                return 1;
            }
        }

        Utility::platformLog("Applying final patches...\n");
        UPDATE_DIALOG_VALUE(50);
        UPDATE_DIALOG_LABEL("Applying final patches...");
        if(TweakError err = apply_necessary_post_randomization_tweaks(worlds[0]/* , randomizeItems */); err != TweakError::NONE) {
            ErrorLog::getInstance().log("Encountered error in post-randomization tweaks!");
            return 1;
        }

        // Restore files that aren't changed (chart list, entrances, etc) so they don't persist across seeds
        // restoreGameFile() does not need to check if the file is cached because it only tries to get the cache entry
        // Getting a cache entry doesn't overwrite anything if the file already had modifications
        Utility::platformLog("Restoring outdated files...\n");
        if(!g_session.restoreGameFile("content/Common/Misc/Misc.szs")) {
            ErrorLog::getInstance().log("Failed to restore Misc.szs!");
            return 1;
        }
        if(!g_session.restoreGameFile("content/Common/Particle/Particle.szs")) {
            ErrorLog::getInstance().log("Failed to restore Particle.szs!");
            return 1;
        }
        if(!g_session.restoreGameFile("content/Common/Pack/permanent_3d.pack")) {
            ErrorLog::getInstance().log("Failed to restore permanent_3d.pack!");
            return 1;
        }
        if(!restoreEntrances()) {
            ErrorLog::getInstance().log("Failed to restore entrances!");
            return 1;
        }

        Utility::platformLog("Preparing to edit files...\n");
        if(!g_session.modFiles()) {
            ErrorLog::getInstance().log("Failed to edit file cache!");
            return 1;
        }


        #ifndef DEVKITPRO
            // Repack for console if necessary
            if (config.repack_for_console)
            {
                UPDATE_DIALOG_LABEL("Repacking for console...\n(This will take a while)");
                Utility::platformLog("Repacking for console...\n");
                const std::filesystem::path dirPath = std::filesystem::path(config.outputDir);
                const std::filesystem::path outPath = std::filesystem::path(config.consoleOutputDir);

                Key commonKey;

                std::string inconspicuousStr1 = "d7b00402659ba2abd2cb0db27fa2b656";

                // Fill encryption keys from strings
                for (size_t i = 0; i < commonKey.size(); i++) {
                    commonKey[i] = static_cast<uint8_t>(strtoul(inconspicuousStr1.substr(i * 2, 2).c_str(), nullptr, 16));
                }

                // Delete any previous repacked files
                for (const auto& entry : std::filesystem::directory_iterator(outPath)) {
                    std::filesystem::remove_all(entry.path());
                }

                // Now repack the files
                if (createPackage(dirPath, outPath, defaultEncryptionKey, commonKey) != PackError::NONE) {
                    ErrorLog::getInstance().log("Failed to create console package");
                    return 1;
                }
            }
            UPDATE_DIALOG_VALUE(200);
        #endif

        //done!
        return 0;
    }
};

int mainRandomize() {
    int retVal = 0;
    { //timer scope
        #ifdef ENABLE_TIMING
            ScopedTimer<std::chrono::high_resolution_clock, "Total process took "> timer;
        #endif

        ErrorLog::getInstance().clearLastErrors();

        Config load;
        std::ifstream conf(APP_SAVE_PATH "config.yaml");
        if(!conf.is_open()) {
            Utility::platformLog("Creating default config\n");
            ConfigError err = createDefaultConfig(APP_SAVE_PATH "config.yaml");
            if(err != ConfigError::NONE) {
                ErrorLog::getInstance().log("Failed to create config, ERROR: " + errorToName(err));

                return 1;
            }
        }
        conf.close();

        #ifdef DEVKITPRO
            if(exitForConfig() == true) {
                return 0;
            }
        #endif

        Utility::platformLog("Reading config\n");
        ConfigError err = loadFromFile(APP_SAVE_PATH "config.yaml", load);
        if(err == ConfigError::DIFFERENT_RANDO_VERSION) {
            Utility::platformLog("Warning: config was made using a different randomizer version\n");
            Utility::platformLog("Item placement may be different than expected\n");
        }
        else if(err != ConfigError::NONE) {
            ErrorLog::getInstance().log("Failed to read config, ERROR: " + errorToName(err));
            Utility::platformLog("Failed to read config, ERROR: " + errorToName(err) + '\n');

            return 1;
        }

        #ifdef DEVKITPRO
            if(!SYSCheckTitleExists(0x0005000010143500)) {
                ErrorLog::getInstance().log("Could not find game: you must have a NTSC-U / US copy of TWWHD!");
                Utility::platformLog("Could not find game: you must have a NTSC-U / US copy of TWWHD!\n");

                return 1;
            }
            if(const auto& err = getTitlePath(0x0005000010143500, load.gameBaseDir); err < 0) {
                return 1;
            }
            if(!Utility::mountDeviceAndConvertPath(load.gameBaseDir)) {
                ErrorLog::getInstance().log("Failed mounting input device!");
                return 1;
            }
            //Utility::platformLog("Got game dir " + load.gameBaseDir.string() + '\n');
            
            
            if(!SYSCheckTitleExists(0x0005000010143599)) {
                Utility::platformLog("Output channel does not currently exist.\n");
                if(!createOutputChannel(load.gameBaseDir, pickInstallLocation())) {
                    return 1;
                }
                g_session.setFirstTimeSetup(true);
            }
            if(const auto& err = getTitlePath(0x0005000010143599, load.outputDir); err < 0) {
                return 1;
            }
            if(!Utility::mountDeviceAndConvertPath(load.outputDir)) {
                ErrorLog::getInstance().log("Failed mounting output device!");
                return 1;
            }
           //Utility::platformLog("Got output dir " + load.outputDir.string() + '\n');
        #endif

        Randomizer rando(load);

        // IMPROVEMENT: issue with seekp, find better solution than manual padding?
        // TODO: make things zoom
        // TODO: do a hundo seed to test everything

        retVal = rando.randomize();
    } // End timer scope

    return retVal;
}
