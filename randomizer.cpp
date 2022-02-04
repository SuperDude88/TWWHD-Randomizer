#include "tweaks.hpp"
#include "options.hpp"
#include "logic/SpoilerLog.hpp"

RandoSession g_session{ "will be set up later", "will be set up later", "will be set up later" }; //declared outside of class for extern stuff

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

	//Logic logic; placeholder, might not be in a class like this

public:
	void randomize() {

		if (!dryRun) {
			init_tweaks(""); //replace with proper rpx path later
			apply_necessary_tweaks(settings.num_starting_triforce_shards, settings.starting_pohs, settings.starting_hcs);
			if (settings.instant_text_boxes) {
				make_all_text_instant();
			}
			if (settings.reveal_full_sea_chart) {
				Apply_Patch(RANDO_ROOT"/asm/patch_diffs/reveal_sea_chart_diff.json"); //replace with proper relative path
			}
			if (settings.add_shortcut_warps_between_dungeons) {
				add_cross_dungeon_warps();
			}
			if (settings.invert_sea_compass_x_axis) {
				Apply_Patch(RANDO_ROOT"/asm/patch_diffs/invert_sea_compass_x_axis_diff.json"); //replace with proper relative path
			}
			update_skip_rematch_bosses_game_variable(settings.skip_rematch_bosses);
			update_sword_mode_game_variable(settings.sword_mode);
			if (settings.sword_mode == SwordMode::NoSword) {
				Apply_Patch(RANDO_ROOT"/asm/patch_diffs/swordless_diff.json"); //replace with proper relative path
				Add_Relocations(RANDO_ROOT"/asm/patch_diffs/swordless_reloc.json"); //replace with proper relative path
				update_swordless_text();
			}
			update_starting_gear(settings.starting_gear);
			if (settings.remove_music) {
				Apply_Patch(RANDO_ROOT"/asm/patch_diffs/remove_music_diff.json"); //replace with proper relative path
			}
		}

		// Create all necessary worlds (for any potential multiworld support in the future)
		World blankWorld;
		WorldPool worlds (numPlayers, blankWorld);
		std::vector<Settings> settingsVector (numPlayers, settings);

		if (randomizeItems) {
			if (!generateWorlds(worlds, settingsVector, 0/*put seed int here*/) {
				// generating worlds failed
				return;
			}
		}

		if (randomizeItems && !dryRun) {
			//save items
			//get world locations with "worlds[playerNum - 1].locationEntries"
		}

		if (!dryRun) {
			//get world with "worlds[playerNum - 1]"
			//apply_necessary_post_randomization_tweaks(randomizeItems, logic.item_locations);
		}

		if (!dryRun) {
			//save everything
		}

		if (randomizeItems) {
			if (!settings.do_not_generate_spoiler_log) {
				generateSpoilerLog(worlds);
			}
			// generateNonSpoilerLog(worlds);
		}

		//done!
		closeDebugLog();
		return;
	}

};
