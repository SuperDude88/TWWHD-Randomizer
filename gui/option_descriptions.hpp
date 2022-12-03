#ifndef OPTION_DESCRIPTIONS_HPP
#define OPTION_DESCRIPTIONS_HPP

#include <string>
#include <unordered_map>

static std::unordered_map<std::string, std::string> optionDescriptions = {
    // Progression Locations
    {
      "progression_dungeons",
      "<b>Disabled</b>: Dungeons will not contain any items necessary to beat the game. <b>Standard</b>: Dungeons may contain items necessary to beat the game. <b>Require Bosses</b>: Certain randomly chosen dungeon bosses will drop required items (e.g. Triforce Shards). <b>Race Mode</b>: Same as Require Bosses, except dungeons without a required boss will not contain progress items. You can set the number of required bosses/dungeons in Additional Randmozation Settings."
    },
    {
      "progression_great_fairies",
      "This controls whether the items given by Great Fairies can be progress items.<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_puzzle_secret_caves",
      "This controls whether puzzle-focused secret caves can contain progress items.<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_combat_secret_caves",
      "This controls whether combat-focused secret caves (besides Savage Labyrinth) can contain progress items.<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_short_sidequests",
      "This controls whether sidequests that can be completed quickly can reward progress items.<br><u>If this is not checked, they will still be randomized</u>, but will only reward optional items you don't need to beat the game."
    },
    {
      "progression_long_sidequests",
      "This controls whether long sidequests (e.g. Lenzo's assistant, withered trees, goron trading) can reward progress items.<br><u>If this is not checked, they will still be randomized</u>, but will only reward optional items you don't need to beat the game."
    },
    {
      "progression_spoils_trading",
      "This controls whether the items you get by trading in spoils to NPCs can be progress items.<br><u>If this is not checked, they will still be randomized</u>, but will only reward optional items you don't need to beat the game."
    },
    {
      "progression_minigames",
      "This controls whether most minigames can reward progress items (auctions, mail sorting, barrel shooting, bird-man contest).<br><u>If this is not checked, minigames will still be randomized</u>, but will only reward optional items you don't need to beat the game."
    },
    {
      "progression_free_gifts",
      "This controls whether gifts freely given by NPCs can be progress items (Tott, Salvage Corp, imprisoned Tingle).<br><u>If this is not checked, they will still be randomized</u>, but will only be optional items you don't need to beat the game."
    },
    {
      "progression_mail",
      "This controls whether mail can contain progress items.<br><u>If this is not checked, mail will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_platforms_rafts",
      "This controls whether lookout platforms and rafts can contain progress items.<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_submarines",
      "This controls whether submarines can contain progress items.<br><u>If this is not checked, submarines will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_eye_reef_chests",
      "This controls whether the chests that appear after clearing out the eye reefs can contain progress items.<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_big_octos_gunboats",
      "This controls whether the items dropped by Big Octos and Gunboats can contain progress items.<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_triforce_charts",
      "This controls whether the sunken treasure chests marked on Triforce Charts can contain progress items.<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_treasure_charts",
      "This controls whether the sunken treasure chests marked on Treasure Charts can contain progress items.<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_expensive_purchases",
      "This controls whether items that cost a lot of rupees can be progress items (Rock Spire shop, auctions, Tingle's letter, trading quest).<br><u>If this is not checked, they will still be randomized</u>, but will only be optional items you don't need to beat the game."
    },
    {
      "progression_misc",
      "Miscellaneous locations that don't fit into any of the above categories (outdoors chests, wind shrine, Cyclos, etc).<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_tingle_chests",
      "Tingle Chests that are hidden in dungeons and must be bombed to make them appear. (2 in DRC, 1 each in FW, TotG, ET, and WT).<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_battlesquid",
      "This controls whether the Windfall battleship minigame can reward progress items.<br><u>If this is not checked, it will still be randomized</u>, but will only reward optional items you don't need to beat the game."
    },
    {
      "progression_savage_labyrinth",
      "This controls whether the Savage Labyrinth can contain progress items.<br><u>If this is not checked, it will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_island_puzzles",
      "This controls whether various island puzzles can contain progress items (e.g. chests hidden in unusual places).<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },
    {
      "progression_obscure",
      "This controls whether obscure checks can contain progression items (e.g. Kane Windfall gate decorations and Earth Temple freestanding spoils).<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },

    // Additional Randomization Options
    {
      "sword_mode",
      "Controls whether you start with the Hero's Sword, the Hero's Sword is randomized, or if there are no swords in the entire game.\nSwordless and No Starting Sword are challenge modes, not recommended for your first run. Also, FF's Phantom Ganon is vulnerable to Skull Hammer in Swordless mode only."
    },
    {
      "dungeon_small_keys",
      "<b>Vanilla</b>: Small Keys will appear in their vanilla locations. <b>Own Dungeon</b>: Small Keys can only appear in their respective dungeon. <b>Any Dungeon</b>: Small Keys can only appear in dungeon locations. <b>Overworld</b>: Small Keys can only appear in non-dungeon locations. <b>Keysanity</b>: Small Keys can appear anywhere."
    },
    {
      "dungeon_big_keys",
      "<b>Vanilla</b>: Big Keys will appear in their vanilla locations. <b>Own Dungeon</b>: Big Keys can only appear in their respective dungeon. <b>Any Dungeon</b>: Big Keys can only appear in dungeon locations. <b>Overworld</b>: Big Keys can only appear in non-dungeon locations. <b>Keysanity</b>: Big Keys can appear anywhere."
    },
    {
      "dungeon_maps_compasses",
      "<b>Vanilla</b>: Maps/Compasses will appear in their vanilla locations. <b>Own Dungeon</b>: Maps/Compasses can only appear in their respective dungeon. <b>Any Dungeon</b>: Maps/Compasses can only appear in dungeon locations. <b>Overworld</b>: Maps/Compasses can only appear in non-dungeon locations. <b>Keysanity</b>: Maps/Compasses can appear anywhere."
    },
    {
      "race_mode",
      "In Race Mode, certain randomly chosen dungeon bosses will drop required items (e.g. Triforce Shards). Nothing in the other dungeons will ever be required.\nYou can see which islands have the required dungeons on them by opening the sea chart and checking which islands have blue quest markers.",
    },
    {
      "num_race_mode_dungeons",
      "Select the number of requried bosses/dungeons that are required in Race Mode.\nRequired dungeon bosses will drop required items (e.g. Triforce Shards).",
    },
    {
      "num_starting_triforce_shards",
      "Change the number of Triforce Shards you start the game with.<br>The higher you set this, the fewer you will need to find placed randomly."
    },
    {
      "randomize_charts",
      "Randomizes which sector is drawn on each Triforce/Treasure Chart."
    },
    {
      "chest_type_matches_contents",
      "Changes the chest type to reflect its contents. A metal chest has a progress item, a key chest has a dungeon key, and a wooden chest has a non-progress item or a consumable.\nKey chests are dark wood chests that use a custom texture based on Big Key chests. Keys for non-required dungeons in race mode will be in wooden chests."
    },
    {
      "damage_multiplier",
      "Change the damage multiplier used when Hero Mode is enabled. By default Hero Mode applies a 2x damage multiplier. This will not affect damage taken in Normal Mode."
    },

    // Convenience Tweaks
    {
      "instant_text_boxes",
      "Text appears instantly.<br>Also, the B button is changed to instantly skip through text as long as you hold it down."
    },
    {
      "reveal_full_sea_chart",
      "Start the game with the sea chart fully drawn out."
    },

    {
      "add_shortcut_warps_between_dungeons",
      "Adds new warp pots that act as shortcuts connecting dungeons to each other directly. (DRC, FW, TotG, and separately FF, ET, WT.)\nEach pot must be unlocked before it can be used, so you cannot use them to access dungeons you wouldn't already have access to."
    },
    {
      "skip_rematch_bosses",
      "Removes the door in Ganon's Tower that only unlocks when you defeat the rematch versions of Gohma, Kalle Demos, Jalhalla, and Molgera."
    },
    {
      "invert_sea_compass_x_axis",
      "Inverts the east-west direction of the compass that shows while at sea.",
    },

    // {
    //   "randomize_music",
    //   "Shuffles around all the music in the game. This affects background music, combat music, fanfares, etc.",
    // },
    // {
    //   "randomize_enemy_palettes",
    //   "Gives all the enemies in the game random colors.",
    // },
    // {
    //   "randomize_enemies",
    //   "Randomizes the placement of non-boss enemies."
    // },

    // {
    //   "custom_player_model",
    //   "Replaces Link's model with a custom player model.\nThese are loaded from the /models folder."
    // },
    {
      "player_in_casual_clothes",
      "Enable this if you want to wear your casual clothes instead of the Hero's Clothes."
    },
    // {
    //   "disable_custom_player_voice",
    //   "If the chosen custom model comes with custom voice files, you can check this option to turn them off and simply use Link's normal voice instead."
    // },
    // {
    //   "disable_custom_player_items",
    //   "If the chosen custom model comes with custom item models, you can check this option to turn them off and simply use Link's normal item models instead."
    // },
    // {
    //   "custom_color_preset",
    //   "This allows you to select from preset color combinations chosen by the author of the selected player model."
    // },
    {
      "randomized_gear",
      "Inventory items that will be randomized."
    },
    {
      "starting_gear",
      "Items that will be in Link's inventory at the start of a new game."
    },
    {
      "starting_pohs",
      "Amount of extra pieces of heart that you start with."
    },
    {
      "starting_hcs",
      "Amount of extra heart containers that you start with."
    },
    {
      "remove_music",
      "Mutes all ingame music."
    },

    // Advanced Options
    {
      "do_not_generate_spoiler_log",
      "Prevents the randomizer from generating a text file listing out the location of every single item for this seed. (This also changes where items are placed in this seed.)<br><u>Generating a spoiler log is highly recommended even if you don't intend to use it</u>, just in case you get completely stuck."
    },
    {
      "start_with_random_item",
      "Randomly start with one extra item, selected uniformly at random from the item pool below.<br>Item Pool: Bait Bag, Bombs, Boomerang, Bow, Deku Leaf, Delivery Bag, Grappling Hook, Hookshot, Picto Box, Power Bracelets, and Skull Hammer."
    },
    {
      "plandomizer",
      "Allows you to select a file which can be used to manually place items in locations, or link specific entrances together in entrance randomizer."
    },

    // Hints
    {
      "ho_ho_hints",
      "Places hints on Old Man Ho Ho. Old Man Ho Ho appears at 10 different islands in the game. Simply talk to Old Man Ho Ho to get hints.",
    },
    {
      "korl_hints",
      "Places hints on the King of Red Lions. Talk to the King of Red Lions to get hints.",
    },
    {
      "path_hints",
      "Determines the number of path hints that will be placed in the game, distributed using the selected hint placement options.\nIf multiple hint placement options are selected, the hint count will be split evenly among the placement options.",
    },
    {
      "barren_hints",
      "Determines the number of barren hints that will be placed in the game, distributed using the selected hint placement options.\nIf multiple hint placement options are selected, the hint count will be split evenly among the placement options.",
    },
    {
      "location_hints",
      "Determines the number of location hints that will be placed in the game, distributed using the selected hint placement options.\nIf multiple hint placement options are selected, the hint count will be split evenly among the placement options.",
    },
    {
      "item_hints",
      "Determines the number of item hints that will be placed in the game, distributed using the selected hint placement options.\nIf multiple hint placement options are selected, the hint count will be split evenly among the placement options.",
    },
    {
      "clearer_hints",
      "When this option is selected, location and item hints will use the standard check or item name, instead of using cryptic hints.",
    },
    {
      "use_always_hints",
      "When the number of location hints is nonzero, certain locations that will always be hinted at will take precedence over normal location hints.",
    },

    // Entrance Randomizer
    {
      "randomize_dungeon_entrances",
      "Shuffles around which dungeon entrances take you into which dungeons."
    },
    {
      "randomize_cave_entrances",
      "Shuffles around which secret cave entrances take you into which secret caves."
    },
    {
      "randomize_door_entrances",
      "Shuffles around which door entrances take you into which interiors."
    },
    {
      "randomize_misc_entrances",
      "Shuffles around which misc entrances take you into which msic areas. Misc entrances are entrances which do not fall into any of the other categories and include entrances such as those that go in and out of Forest Haven and Dragon Roost Island. Hyrule is not included in this category."
    },
    {
      "mix_pools_combobox",
      "Shuffle the selected entrances into a mixed pool instead of separate ones. For example, randomizing dungeons, caves, and doors, and selecting dungeons and caves here will allow a dungeon to be inside a cave or vice versa, while doors are shuffled in their own separate pool."
    },
    {
      "decouple_entrances",
      "Decouple entrances when shuffling them. This means you are no longer guaranteed to end back up where you came from when you go back through an entrance."
    },
    {
      "randomize_starting_island",
      "Randomizes which island you start the game on."
    },
};
#endif // OPTION_DESCRIPTIONS_HPP
