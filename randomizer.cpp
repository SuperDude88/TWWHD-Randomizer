#include "tweaks.hpp"
#include "options.hpp"
#include "logic/SpoilerLog.hpp"
#include "logic/Generate.hpp"
#include "server/command/WriteLocations.hpp"

#ifdef ENABLE_DEBUG
#include "Debug.hpp"
#endif

RandoSession g_session("to be set up later", "to be set up later", "to be set up later"); //declared outside of class for extern stuff

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
	void randomize() {

		if (!dryRun) {
			apply_necessary_tweaks(settings, seedHash);
			if (settings.instant_text_boxes) {
				make_all_text_instant();
			}

			if (settings.reveal_full_sea_chart) {
				Apply_Patch("./asm/patch_diffs/reveal_sea_chart_diff.json");
			}

			if (settings.add_shortcut_warps_between_dungeons) {
				add_cross_dungeon_warps();
			}

			if (settings.invert_sea_compass_x_axis) {
				Apply_Patch("./asm/patch_diffs/invert_sea_compass_x_axis_diff.json");
			}

			update_skip_rematch_bosses_game_variable(settings.skip_rematch_bosses);
			update_sword_mode_game_variable(settings.sword_mode);

			if (settings.sword_mode == SwordMode::NoSword) {
				Apply_Patch("./asm/patch_diffs/swordless_diff.json");
				Add_Relocations("./asm/patch_diffs/swordless_reloc.json");
				update_swordless_text();
			}

			update_starting_gear(settings.starting_gear);

			if (settings.remove_music) {
				Apply_Patch("./asm/patch_diffs/remove_music_diff.json");
			}
		}

		// Create all necessary worlds (for any potential multiworld support in the future)
		World blankWorld;
		WorldPool worlds (numPlayers, blankWorld);
		std::vector<Settings> settingsVector (numPlayers, settings);

		if (randomizeItems) {
			if (!generateWorlds(worlds, settingsVector, 0/*put seed int here*/)) {
				// generating worlds failed
				return;
			}
		}

		if (randomizeItems && !dryRun) {
			//get world locations with "worlds[playerNum - 1].locationEntries"
			//assume 1 world for now, modifying multiple copies needs work
			for (const Location& location : worlds[0].locationEntries) {
				if (ModificationError err = location.method->writeLocation(location.currentItem); err != ModificationError::NONE) return; //handle err somehow
			}
			saveRPX();
		}

		if (!dryRun) {
			//get world with "worlds[playerNum - 1]"
			//assume 1 world for now, modifying multiple copies needs work
			apply_necessary_post_randomization_tweaks(randomizeItems, worlds[0].locationEntries);
		}

		if (!dryRun) {
			g_session.repackCache();
		}

		if (randomizeItems) {
			if (!settings.do_not_generate_spoiler_log) {
				generateSpoilerLog(worlds);
			}
			generateNonSpoilerLog(worlds);
		}

		//done!

#ifdef ENABLE_DEBUG
		closeDebugLog();
#endif
		
		return;
	}

};
