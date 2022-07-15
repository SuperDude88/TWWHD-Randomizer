#include "tweaks.hpp"
#include "options.hpp"
#include "logic/SpoilerLog.hpp"
#include "logic/Generate.hpp"
#include "server/filetypes/dzx.hpp"
#include "server/command/WriteLocations.hpp"
#include "server/command/Log.hpp"
#include "logic/Random.hpp"
#include <cstring>
#include <fstream>
#include <functional>

#define SEED_KEY "SEED KEY TEST"

RandoSession g_session("base", "working", "output"); //declared outside of class for extern stuff



class Randomizer {
private:
	Settings settings;
	const std::string seed;
	size_t integer_seed;
	std::string permalink;

	bool dryRun = false;
	bool noLogs = false;
	bool bulkTest = false;
	bool randomizeItems = true;
	int numPlayers = 1;
	int playerId = 1;

public:
	Randomizer(const Settings& settings_, const std::string& seed_) :
		settings(settings_),
		seed(seed_)
	{}

	void randomize() {
		std::string seedString = seed;
		if(settings.do_not_generate_spoiler_log) seedString += SEED_KEY;

		std::hash<std::string> strHash;
		integer_seed = strHash(seedString);

		//init filesystem, session stuff

		if (!dryRun) {
			ModifyChest::setCTMC(settings.chest_type_matches_contents);
			apply_necessary_tweaks(settings, "Testing Hash"); //TODO: test and finish these
			if (settings.instant_text_boxes) {
				make_all_text_instant();
			}
			if (settings.reveal_full_sea_chart) {
				Apply_Patch("./asm/patch_diffs/reveal_sea_chart_diff.json");
			}
			if (settings.invert_sea_compass_x_axis) {
				Apply_Patch("./asm/patch_diffs/invert_sea_compass_x_axis_diff.json");
			}

			if (settings.sword_mode == SwordMode::NoSword) {
				Apply_Patch("./asm/patch_diffs/swordless_diff.json");
				Add_Relocations("./asm/patch_diffs/swordless_reloc.json");
				update_swordless_text();
			}

			if (settings.remove_music) {
				Apply_Patch("./asm/patch_diffs/remove_music_diff.json");
			}
		}

		// Create all necessary worlds (for any potential multiworld support in the future)
		WorldPool worlds(numPlayers);
		std::vector<Settings> settingsVector (numPlayers, settings);

		if (randomizeItems) {
			if (generateWorlds(worlds, settingsVector, integer_seed) != 0) {
				// generating worlds failed
				return;
			}

			if(!dryRun) {
				//TODO: write randomized charts
				//TODO: starting island
				if(getSetting(settings, Option::PigColor) == static_cast<int>(PigColor::RANDOM)) {
					settings.pigColor = static_cast<PigColor>(Random(0, 3));
				}

				//get world locations with "worlds[playerNum - 1].locationEntries"
				//assume 1 world for now, modifying multiple copies needs work
				for (const Location& location : worlds[0].locationEntries) {
					if (location.locationId == LocationId::INVALID) continue; //shouldnt be here maybe?
					if (ModificationError err = location.method->writeLocation(location.currentItem); err != ModificationError::NONE) return; //handle err somehow
				}
				saveRPX();

				if (settings.randomize_cave_entrances || settings.randomize_door_entrances || settings.randomize_dungeon_entrances || settings.randomize_misc_entrances) {
					// Info for entrance rando:
					const std::unordered_set<uint8_t> pack1 = {0, 1, 11, 13, 17, 23};
					const std::unordered_set<uint8_t> pack2 = {9, 39, 41, 44};
					std::unordered_map<std::string, FileTypes::DZXFile> dzr_by_path;
	
					auto entrances = worlds[playerId - 1].getShuffledEntrances(EntranceType::ALL);
	    			for (const auto entrance : entrances)
	    			{
	    			    std::string fileStage = entrance->getFilepathStage();
	    			    std::string fileRoom = std::to_string(entrance->getFilepathRoomNum());
	    			    uint8_t sclsExitIndex = entrance->getSclsExitIndex();
	
	    			    std::string replacementStage = entrance->getReplaces()->getStageName();
	    			    uint8_t replacementRoom = entrance->getReplaces()->getRoomNum();
	    			    uint8_t replacementSpawn = entrance->getReplaces()->getSpawnId();
	
	    			    std::string filepath = "content/Common/Stage/" + fileStage + "_Room" + fileRoom + ".szs@YAZ0@SARC@Room" + fileRoom + ".bfres@BFRES@room.dzr";
	
						if (fileStage == "sea") {
							if (pack1.count(entrance->getFilepathRoomNum()) > 0) {
								filepath = "content/Common/Pack/szs_permanent1.pack@SARC@" + fileStage + "_Room" + fileRoom + ".szs@YAZ0@SARC@Room" + fileRoom + ".bfres@BFRES@room.dzr";
							}
							else if (pack2.count(entrance->getFilepathRoomNum()) > 0) {
								filepath = "content/Common/Pack/szs_permanent2.pack@SARC@" + fileStage + "_Room" + fileRoom + ".szs@YAZ0@SARC@Room" + fileRoom + ".bfres@BFRES@room.dzr";
						
							}
						}
	
						const RandoSession::fspath roomPath = g_session.openGameFile(filepath);
						if(dzr_by_path.count(roomPath.string()) == 0)
						{
							dzr_by_path[roomPath.string()].loadFromFile(roomPath.string());
						}
	
						ChunkEntry* exit = dzr_by_path[roomPath.string()].entries_by_type("SCLS")[sclsExitIndex];
						replacementStage.resize(8);
						exit->data.replace(0, 8, replacementStage, 8);
						exit->data[8] = replacementSpawn;
						exit->data[9] = replacementRoom;
					}
	
					for (auto [path, dzr] : dzr_by_path) {
						dzr.writeToFile(path);
					}
				}
				
				apply_necessary_post_randomization_tweaks(randomizeItems, worlds[0].locationEntries, settings.pigColor); //TODO: finish post random tweaks
			}

			if (!settings.do_not_generate_spoiler_log) {
				generateSpoilerLog(worlds, seed);
			}
		}

		//Run some patches after writing items so offsets are correct
		if (settings.add_shortcut_warps_between_dungeons) {
			add_cross_dungeon_warps();
		}
		
		g_session.repackCache();

		generateNonSpoilerLog(worlds);

		//done!
		return;
	}

};


#include <iostream>
#include "server/filetypes/elf.hpp"
#include "server/filetypes/wiiurpx.hpp"

int main() {
	Settings settings;
	
	settings.progression_dungeons = true;
	settings.progression_mail = true;
	settings.progression_dungeons = true;
	settings.progression_great_fairies = true;
	settings.progression_puzzle_secret_caves = true;
	settings.progression_combat_secret_caves = true;
	settings.progression_submarines = true;
	settings.progression_eye_reef_chests = true;
	settings.progression_big_octos_gunboats = true;
	settings.progression_triforce_charts = true;
	settings.progression_treasure_charts = true;
	settings.progression_expensive_purchases = true;
	settings.progression_misc = true;
	settings.progression_tingle_chests = true;
	settings.progression_battlesquid = true;
	settings.progression_savage_labyrinth = true;
	settings.progression_island_puzzles = true;
	settings.progression_obscure = true;
	settings.keylunacy = true;
	settings.randomize_charts = true;
	settings.race_mode = true;
	settings.num_race_mode_dungeons = 3;

	Randomizer rando(settings, "TestSeed");
	rando.randomize();

	//timing stuff
	//auto start = std::chrono::high_resolution_clock::now();
	//auto stop = std::chrono::high_resolution_clock::now();
	//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	//auto duration2 = duration.count();
	return 0;
}
