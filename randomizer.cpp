#include "tweaks.hpp"
#include "libs/tinyxml2.h"
#include "seedgen/config.hpp"
#include "seedgen/random.hpp"
#include "seedgen/seed.hpp"
#include "seedgen/permalink.hpp"
#include "logic/SpoilerLog.hpp"
#include "logic/Generate.hpp"
#include "logic/mass_test.hpp"
#include "server/filetypes/dzx.hpp"
#include "server/filetypes/charts.hpp"
#include "server/command/WriteLocations.hpp"
#include "server/command/RandoSession.hpp"
#include "server/command/Log.hpp"
#include "server/utility/platform.hpp"
#include "server/utility/file.hpp"
#include "server/utility/endian.hpp"
#include "gui/update_dialog_header.hpp"
#include <cstring>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

#ifdef DEVKITPRO
#include <unistd.h> //for chdir
#include <sysapp/title.h>
#include "server/platform/wiiutitles.hpp"

static std::vector<Utility::titleEntry> wiiuTitlesList{};
#endif

#include <thread>
#define SEED_KEY "SEED KEY TEST"

RandoSession g_session; //declared outside of class for extern stuff



class Randomizer {
private:
	Config config;
	std::string permalink;
	size_t integer_seed;

	bool dryRun = false;
	bool randomizeItems = true;
	unsigned int numPlayers = 1;
	//int playerId = 1;

	[[nodiscard]] bool checkBackupOrDump() {

		using namespace std::filesystem;
		Utility::platformLog("Verifying dump...\n");
		UPDATE_DIALOG_LABEL("Verifying dump...");
		UPDATE_DIALOG_VALUE(25);

		const RandoSession::fspath& base = g_session.getBaseDir();
		if(!is_directory(base / "code") || !is_directory(base / "content") || !is_directory(base / "meta")) {
			// Utility::platformLog("Could not find code/content/meta folders at base directory!\n");
			ErrorLog::getInstance().log("Could not find code/content/meta folders at base directory!");

			#ifdef DEVKITPRO
				Utility::platformLog("Attempting to dump game\n");
				if(!SYSCheckTitleExists(0x0005000010143500)) {
					Utility::platformLog("Could not find game! You must have a digital install of The Wind Waker HD (NTSC-U / US version).\n");
					std::this_thread::sleep_for(std::chrono::seconds(3));
					return false;
				}

				const std::vector<Utility::titleEntry> titles = *Utility::getLoadedTitles();
				auto game = std::find_if(titles.begin(), titles.end(),
					[](const Utility::titleEntry& entry) {return entry.titleLowID == 0x10143500; }
				);
				if(game == titles.end()) {
					Utility::platformLog("Title not loaded\n");
					std::this_thread::sleep_for(std::chrono::seconds(3));
					return false;
				}

				if(!Utility::dumpGame(*game, {.dumpTypes = Utility::dumpTypeFlags::Game}, base)) return false;
			#else
				// Utility::platformLog("Invalid path: you must specify the path to a decrypted dump of The Wind Waker HD (NTSC-U / US version).\n");
				ErrorLog::getInstance().log("Invalid base game path: you must specify the path to a\ndecrypted dump of The Wind Waker HD (NTSC-U / US version).");
				return false;
			#endif
		}

		//Check the meta.xml for other platforms (+ a sanity check on console)
		tinyxml2::XMLDocument metaXml;
		std::stringstream* meta = g_session.openGameFile("meta/meta.xml");
		if(meta == nullptr) {
			ErrorLog::getInstance().log("Failed extracting meta.xml");
			return false;
		}

		if(tinyxml2::XMLError err = metaXml.LoadFile(*meta); err != tinyxml2::XMLError::XML_SUCCESS) {
			ErrorLog::getInstance().log("Could not parse meta.xml, got error " + std::to_string(err));
			return false;
		}
		const tinyxml2::XMLElement* root = metaXml.RootElement();

		const std::string titleId = root->FirstChildElement("title_id")->GetText();
		const std::string nameEn = root->FirstChildElement("longname_en")->GetText();
		if(titleId != "0005000010143500" || nameEn != "THE LEGEND OF ZELDA\nThe Wind Waker HD")  {
			// Utility::platformLog("meta.xml does not match base game - dump is not valid\n");
			// Utility::platformLog("ID %s\n", titleId.c_str());
			// Utility::platformLog("Name %s\n", nameEn.c_str());
			ErrorLog::getInstance().log("meta.xml does not match base game - dump is not valid");
			ErrorLog::getInstance().log("ID " + titleId);
			ErrorLog::getInstance().log("Name " + nameEn);
			std::this_thread::sleep_for(std::chrono::seconds(3));
			return false;
		}

		const std::string region = root->FirstChildElement("region")->GetText();
		if(region != "00000002") {
			// Utility::platformLog("Incorrect region - game must be a NTSC-U / US copy\n");
			ErrorLog::getInstance().log("Incorrect region - game must be a NTSC-U / US copy");
			std::this_thread::sleep_for(std::chrono::seconds(3));
			return false;
		}

		return true;
	}

	void clearOldLogs() {
		if(std::filesystem::is_regular_file("./Debug Log.txt")) {
			std::filesystem::remove("./Debug Log.txt");
		}
		if(std::filesystem::is_regular_file("./Error Log.txt")) {
			std::filesystem::remove("./Error Log.txt");
		}
		if(std::filesystem::is_regular_file("./Non-Spoiler Log.txt")) {
			std::filesystem::remove("./Non-Spoiler Log.txt");
		}
		if(std::filesystem::is_regular_file("./Spoiler Log.txt")) {
			std::filesystem::remove("./Spoiler Log.txt");
		}

		return;
	}

	[[nodiscard]] bool writeCharts(WorldPool& worlds) {
		using namespace std::literals::string_literals;

		Utility::platformLog("Saving randomized charts...\n");
		UPDATE_DIALOG_LABEL("Saving randomized charts...");

		std::stringstream* chartStream = g_session.openGameFile("content/Common/Misc/Misc.szs@YAZ0@SARC@Misc.bfres@BFRES@cmapdat.bin");
		if(chartStream == nullptr) {
			ErrorLog::getInstance().log("Failed to open cmapdat.bin");
			return false;
		}
		FileTypes::ChartList charts;
		if(ChartError err = charts.loadFromBinary(*chartStream); err != ChartError::NONE) {
			ErrorLog::getInstance().log("Failed to load chart list");
			return false;
		}
		const std::vector<Chart> original_charts = charts.charts;

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

			const Chart& original_chart = *std::find_if(original_charts.begin(), original_charts.end(), [islandNumber](const Chart& chart) {return (chart.type == 0 || chart.type == 1 || chart.type == 2 || chart.type == 5 || chart.type == 6 || chart.type == 8) && chart.getIslandNumber() == islandNumber; });
			const GameItem new_chart_item = worlds[0].chartMappings[i];
			auto new_chart = std::find_if(charts.charts.begin(), charts.charts.end(), [&](const Chart& chart) {return chart.getItem() == new_chart_item;});
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

			std::string dzrPath;
			if (islandNumber == 1 || islandNumber == 11 || islandNumber == 13 || islandNumber == 17 || islandNumber == 23) {
				dzrPath = "content/Common/Pack/szs_permanent1.pack@SARC@sea_Room" + std::to_string(islandNumber) + ".szs@YAZ0@SARC@Room" + std::to_string(islandNumber) + ".bfres@BFRES@room.dzr";
			}
			else if (islandNumber == 9 || islandNumber == 39 || islandNumber == 41 || islandNumber == 44) {
				dzrPath = "content/Common/Pack/szs_permanent2.pack@SARC@sea_Room" + std::to_string(islandNumber) + ".szs@YAZ0@SARC@Room" + std::to_string(islandNumber) + ".bfres@BFRES@room.dzr";
			}
			else {
				dzrPath = "content/Common/Stage/sea_Room" + std::to_string(islandNumber) + ".szs@YAZ0@SARC@Room" + std::to_string(islandNumber) + ".bfres@BFRES@room.dzr";
			}
			std::stringstream* dzrStream = g_session.openGameFile(dzrPath);
			if(dzrStream == nullptr) {
				ErrorLog::getInstance().log("Failed to open file " + dzrPath);
				return false;
			}
			FileTypes::DZXFile dzr;
			if(DZXError err = dzr.loadFromBinary(*dzrStream); err != DZXError::NONE) {
				ErrorLog::getInstance().log("Failed to load dzr with path " + dzrPath);
				return false;
			}

			for(ChunkEntry* scob : dzr.entries_by_type("SCOB")) {
				if(salvage_object_names.count(scob->data.substr(0, 8)) > 0 && ((scob->data[8] & 0xF0) >> 4) == 0) {
					uint32_t& params = *reinterpret_cast<uint32_t*>(&scob->data[8]);
					Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, params);
					const uint32_t mask = 0x0FF00000;
					const uint8_t shiftAmount = 20;

    				params = (params & (~mask)) | ((new_chart->owned_chart_index_plus_1 << shiftAmount) & mask);
					Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, params);
				}
			}

			if(DZXError err = dzr.writeToStream(*dzrStream); err != DZXError::NONE) {
				ErrorLog::getInstance().log("Failed to save dzr with path " + dzrPath);
				return false;
			}
		}

		if(ChartError err = charts.writeToStream(*chartStream); err != ChartError::NONE) {
			ErrorLog::getInstance().log("Failed to save chart list");
			return false;
		}

		return true;
	}

	[[nodiscard]] bool restoreEntrances() {
		//Needs to restore data before anything is modified, worlds are generated after pre-randomization tweaks are applied
		//Get around this with a list of the entrance paths
		//Skip anything rando does edit to save time, they get restored during extraction
		static const std::list<std::pair<std::string, uint8_t>> vanillaEntrancePaths = {                                                     //----File path info------|---entrance info---//
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

		for(const std::string& path : paths) {
			if(g_session.isCached(path)) continue;
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
		std::unordered_map<std::stringstream*, FileTypes::DZXFile> dzr_by_path;
	
		const EntrancePool entrances = worlds[0].getShuffledEntrances(EntranceType::ALL);
	    for (const auto entrance : entrances)
	    {
	        const std::string fileStage = entrance->getFilepathStage();
	        const std::string fileRoom = std::to_string(entrance->getFilepathRoomNum());
	        const uint8_t sclsExitIndex = entrance->getSclsExitIndex();
	        std::string replacementStage = entrance->getReplaces()->getStageName();
	        const uint8_t replacementRoom = entrance->getReplaces()->getRoomNum();
	        const uint8_t replacementSpawn = entrance->getReplaces()->getSpawnId();

	        std::string filepath = "content/Common/Stage/" + fileStage + "_Room" + fileRoom + ".szs@YAZ0@SARC@Room" + fileRoom + ".bfres@BFRES@room.dzr";

			if (fileStage == "sea") {
				if (pack1.count(entrance->getFilepathRoomNum()) > 0) {
					filepath = "content/Common/Pack/szs_permanent1.pack@SARC@" + fileStage + "_Room" + fileRoom + ".szs@YAZ0@SARC@Room" + fileRoom + ".bfres@BFRES@room.dzr";
				}
				else if (pack2.count(entrance->getFilepathRoomNum()) > 0) {
					filepath = "content/Common/Pack/szs_permanent2.pack@SARC@" + fileStage + "_Room" + fileRoom + ".szs@YAZ0@SARC@Room" + fileRoom + ".bfres@BFRES@room.dzr";
				}
			}
	
			std::stringstream* dzrStream = g_session.openGameFile(filepath);
			if(dzrStream == nullptr) {
				ErrorLog::getInstance().log("Failed to open file " + filepath);
				return false;
			}

			if(dzr_by_path.count(dzrStream) == 0)
			{
				if(DZXError err = dzr_by_path[dzrStream].loadFromBinary(*dzrStream); err != DZXError::NONE) {
					ErrorLog::getInstance().log("Failed to load dzr with path " + filepath);
				}
			}
			const std::vector<ChunkEntry*> scls_entries = dzr_by_path[dzrStream].entries_by_type("SCLS");
			if(sclsExitIndex > (scls_entries.size() - 1)) {
				ErrorLog::getInstance().log("SCLS entry index outside of list!");
				return false;
			}

			ChunkEntry* exit = scls_entries[sclsExitIndex];
			replacementStage.resize(8, '\0');
			exit->data.replace(0, 8, replacementStage.c_str(), 8);
			exit->data[8] = replacementSpawn;
			exit->data[9] = replacementRoom;
		}
	
		for (auto& [stream, dzr] : dzr_by_path) {
			if(DZXError err = dzr.writeToStream(*stream); err != DZXError::NONE) {
				ErrorLog::getInstance().log("Failed to save entrance dzr");
				return false;
			}
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
			massTest(config);
			return 0;
		#endif

		LOG_TO_DEBUG(permalink);

		if(config.settings.do_not_generate_spoiler_log) permalink += SEED_KEY;

		std::hash<std::string> strHash;
		integer_seed = strHash(permalink);

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


		// Skip all game modification stuff if we're just doing fill algorithm testing
		#ifndef FILL_TESTING

			if(!checkBackupOrDump()) {
				return 1;
			}
			
			//IMPROVEMENT: custom model things

			Utility::platformLog("Modifying game code...\n");
			UPDATE_DIALOG_VALUE(30);
			UPDATE_DIALOG_LABEL("Modifying game code...");
			if (!dryRun) {
				if(TweakError err = apply_necessary_tweaks(config.settings); err != TweakError::NONE) {
					ErrorLog::getInstance().log("Encountered error in pre-randomization tweaks!");
					return 1;
				}
			}
		#else
			dryRun = true;
		#endif

		if (randomizeItems) {
			if(!dryRun) {
				if(config.settings.randomize_charts) {
					if(!writeCharts(worlds)) {
						ErrorLog::getInstance().log("Failed to save charts!");
						return 1;
					}
				}

				//get world locations with "worlds[playerNum - 1].locationEntries"
				//assume 1 world for now, modifying multiple copies needs work
				Utility::platformLog("Saving items...\n");
				UPDATE_DIALOG_VALUE(40);
				UPDATE_DIALOG_LABEL("Saving items...");
				resetRPX();
				ModifyChest::setCTMC(config.settings.chest_type_matches_contents, config.settings.race_mode, worlds[0].dungeons);
				for (auto& [name, location] : worlds[0].locationEntries) {
					if (ModificationError err = location.method->writeLocation(location.currentItem); err != ModificationError::NONE) {
						ErrorLog::getInstance().log("Failed to save location " + location.name);
						return 1;
					}
				}
				if(const ELFError& err = saveRPX(); err != ELFError::NONE) {
					ErrorLog::getInstance().log("Failed to save modified ELF!");
					return 1;
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
				if(TweakError err = apply_necessary_post_randomization_tweaks(worlds[0], randomizeItems); err != TweakError::NONE) {
					ErrorLog::getInstance().log("Encountered error in post-randomization tweaks!");
					return 1;
				}
			}

			if (!config.settings.do_not_generate_spoiler_log) {
				generateSpoilerLog(worlds);
			}
		}

		//Restore files that aren't changed (chart list, entrances, etc) so they don't persist across seeds
		//Do this at the end to check if the files were cached
		//Copying is slow so we skip all the ones we can
		Utility::platformLog("Restoring outdated files...\n");
		if(!g_session.isCached("content/Common/Misc/Misc.szs")) {
			if(!g_session.restoreGameFile("content/Common/Misc/Misc.szs")) {
				ErrorLog::getInstance().log("Failed to restore Misc.szs!");
				return 1;
			}
		}
		if(!g_session.isCached("content/Common/Pack/permanent_3d.pack")) {
			if(!g_session.restoreGameFile("content/Common/Pack/permanent_3d.pack")) {
				ErrorLog::getInstance().log("Failed to restore permanent_3d.pack!");
				return 1;
			}
		}
		if(!restoreEntrances()) {
			ErrorLog::getInstance().log("Failed to restore entrances!");
			return 1;
		}

		Utility::platformLog("Preparing to repack files...\n");
		if(!g_session.repackCache()) {
			ErrorLog::getInstance().log("Failed to repack file cache!");
			return 1;
		}

		//done!
		return 0;
	}

	bool restoreFromBackup() {
		Utility::platformLog("Restoring backup\n");

		return Utility::copy(g_session.getBaseDir(), g_session.getOutputDir());
	}
};

int mainRandomize() {
	using namespace std::chrono_literals;
	#ifdef ENABLE_TIMING
			auto start = std::chrono::high_resolution_clock::now();
	#endif

	ProgramTime::getInstance(); //create instance + set time

	Utility::platformInit();

	ErrorLog::getInstance().clearLastErrors();

	#ifdef DEVKITPRO
		if(!std::filesystem::is_directory("fs:/vol/external01/wiiu/apps/rando")) {
			ErrorLog::getInstance().log("Could not find randomizer path! This means the randomizer's folder is named incorrectly. Check that the .rpx is inside a folder named \"rando\"");
			Utility::platformLog("Could not find randomizer path!\n");
			Utility::platformLog("This means the randomizer's folder is named incorrectly.\n");
			Utility::platformLog("Check that the .rpx is inside a folder named \"rando\".\n");

			std::this_thread::sleep_for(3s);
			Utility::platformShutdown();
			return 1;
		}
		chdir("fs:/vol/external01/wiiu/apps/rando");
	#endif

	Config load;
	std::ifstream conf("./config.yaml");
	if(!conf.is_open()) {
		Utility::platformLog("Creating default config\n");
		ConfigError err = createDefaultConfig("./config.yaml");
		if(err != ConfigError::NONE) {
			ErrorLog::getInstance().log("Failed to create config, code " + std::to_string(static_cast<uint32_t>(err)));

			std::this_thread::sleep_for(3s);
			Utility::platformShutdown();
			return 1;
		}
	}
	conf.close();

	Utility::platformLog("Reading config\n");
	ConfigError err = loadFromFile("./config.yaml", load);
	if(err != ConfigError::NONE) {
		ErrorLog::getInstance().log("Failed to read config, code " + std::to_string(static_cast<uint32_t>(err)));

		std::this_thread::sleep_for(3s);
		Utility::platformShutdown();
		return 1;
	}

	Randomizer rando(load);

	// TODO: do a hundo seed to test everything
	// TODO: text wrapping on drc dungeon map
	// TODO: fix uncompressed things
	// TODO: jpc saving is broken
	// TODO: somehow broke multithreading on Wii U (doesn't crash, just softlock + black screen), appears fine on PC
	int retVal = rando.randomize();

	#ifdef ENABLE_TIMING
			auto stop = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
			auto seconds = static_cast<double>(duration.count()) / 1000000.0;
			Utility::platformLog(std::string("Total process took ") + std::to_string(seconds) + " seconds\n");
	#endif

	// Close logs
	ErrorLog::getInstance().close();
	DebugLog::getInstance().close();
	BasicLog::getInstance().close();

	std::this_thread::sleep_for(3s);
	Utility::platformShutdown();
	return retVal;
}
