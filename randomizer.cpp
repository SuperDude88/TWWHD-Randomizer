#include "tweaks.hpp"
#include "options.hpp"
#include "logic/SpoilerLog.hpp"
#include "logic/Generate.hpp"

RandoSession g_session("will be set up later", "will be set up later", "will be set up later"); //declared outside of class for extern stuff

class Randomizer {
private:
	Settings settings;
	std::string seed;
	std::string permalink; //will likely be replaced with config struct later?
	std::string seedHash;
	std::string seedString;

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
	{
	}

	void randomize() {

		//if (!dryRun) {
		//	init_tweaks();
		//	apply_necessary_tweaks(settings, seedHash);
		//	if (settings.instant_text_boxes) {
		//		make_all_text_instant();
		//	}
//
		//	if (settings.reveal_full_sea_chart) {
		//		Apply_Patch("./asm/patch_diffs/reveal_sea_chart_diff.json");
		//	}
//
		//	if (settings.add_shortcut_warps_between_dungeons) {
		//		add_cross_dungeon_warps();
		//	}
//
		//	if (settings.invert_sea_compass_x_axis) {
		//		Apply_Patch("./asm/patch_diffs/invert_sea_compass_x_axis_diff.json");
		//	}
//
		//	update_skip_rematch_bosses_game_variable(settings.skip_rematch_bosses);
		//	update_sword_mode_game_variable(settings.sword_mode);
//
		//	if (settings.sword_mode == SwordMode::NoSword) {
		//		Apply_Patch("./asm/patch_diffs/swordless_diff.json");
		//		Add_Relocations("./asm/patch_diffs/swordless_reloc.json");
		//		update_swordless_text();
		//	}
//
		//	update_starting_gear(settings.starting_gear);
//
		//	if (settings.remove_music) {
		//		Apply_Patch("./asm/patch_diffs/remove_music_diff.json");
		//	}
		//}

		// Create all necessary worlds (for any potential multiworld support in the future)
		WorldPool worlds;
		worlds.resize(numPlayers);
		std::vector<Settings> settingsVector (numPlayers, settings);

		if (randomizeItems) {
			if (!generateWorlds(worlds, settingsVector, 0/*put seed int here*/)) {
				// generating worlds failed
				return;
			}
		}

		if (randomizeItems && !dryRun) {
			//save items
			//get world locations with "worlds[playerId - 1].locationEntries"
		}

		if (!dryRun) {
			//get world with "worlds[playerId - 1]"
			//apply_necessary_post_randomization_tweaks(randomizeItems, logic.item_locations);

			// Info for entrance rando:
			auto entrances = worlds[playerId - 1].getShuffledEntrances(EntranceType::ALL);
	    for (auto entrance : entrances)
	    {
	        std::string fileStage = entrance->getFilepathStage();
	        std::string fileRoom = std::to_string(entrance->getFilepathRoomNum());
	        uint8_t sclsExitIndex = entrance->getSclsExitIndex();

	        std::string replacementStage = entrance->getReplaces()->getStageName();
	        uint8_t replacementRoom = entrance->getReplaces()->getRoomNum();
	        uint8_t replacementSpawn = entrance->getReplaces()->getSpawnId();

	        std::string filepath = "content/Common/Stage/" + fileStage + "_Room" + fileRoom + ".szs";
	        // In the above file, replace data at exit index <sclsExitIndex> with:
					// - replacementStage
					// - replacementRoom
					// - replacemntSpawn

	    }
		}

		if (!dryRun) {
			//save everything
		}

		if (randomizeItems) {
			if (!settings.do_not_generate_spoiler_log) {
				generateSpoilerLog(worlds);
			}
			generateNonSpoilerLog(worlds);
		}

		return;
	}

};

int main() {
	Settings settings;
	
	settings.progression_dungeons = true;
	settings.progression_great_fairies = true;
	settings.progression_puzzle_secret_caves = true;
	settings.progression_combat_secret_caves = true;
	settings.progression_short_sidequests = true;
	settings.progression_long_sidequests = false;
	settings.progression_spoils_trading = true;
	settings.progression_minigames = true;
	settings.progression_free_gifts = true;
	settings.progression_mail = true;
	settings.progression_platforms_rafts = true;
	settings.progression_submarines = true;
	settings.progression_eye_reef_chests = true;
	settings.progression_big_octos_gunboats = true;
	settings.progression_triforce_charts = false;
	settings.progression_treasure_charts = false;
	settings.progression_expensive_purchases = false;
	settings.progression_misc = true;
	settings.progression_tingle_chests = true;
	settings.progression_battlesquid = false;
	settings.progression_savage_labyrinth = false;
	settings.progression_island_puzzles = true;
	settings.progression_obscure = true;
	settings.keylunacy = false;
	settings.randomize_charts = false;
	settings.randomize_starting_island = true;
	settings.randomize_dungeon_entrances = true;
	settings.randomize_cave_entrances = false;
	settings.randomize_door_entrances = false;
	settings.randomize_misc_entrances = false;
	settings.mix_entrance_pools = false;
	settings.decouple_entrances = false;
	settings.instant_text_boxes = true;
	settings.reveal_full_sea_chart = true;
	settings.num_starting_triforce_shards = 0;
	settings.add_shortcut_warps_between_dungeons = false;
	settings.do_not_generate_spoiler_log = false;
	settings.sword_mode = SwordMode::StartWithSword;
	settings.skip_rematch_bosses = true;
	settings.invert_sea_compass_x_axis = false;
	settings.race_mode = true;
	settings.num_race_mode_dungeons = 3;
	settings.damage_multiplier = 1.0f;
	settings.chest_type_matches_contents = true;
	settings.player_in_casual_clothes = false;
	settings.pigColor = PigColor::BLACK;
	settings.starting_gear = {};
	settings.starting_pohs = 0;
	settings.starting_hcs = 0;
	settings.remove_music = false;

	Randomizer rando(settings, "TestSeed3");
	// TODO: picto box maybe fixed, test windfall postman
	// TODO: great fairy hints
	rando.randomize();

	//timing stuff
	//auto start = std::chrono::high_resolution_clock::now();
	//auto stop = std::chrono::high_resolution_clock::now();
	//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	//auto duration2 = duration.count();

	return 0;
}
