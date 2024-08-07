#pragma once

#include <string>
#include <unordered_map>

static std::unordered_map<std::string, std::string> optionDescriptions = {
    // Progression Locations
    {
      "progression_dungeons",
      "<b>Disabled</b>: Dungeons will not contain any items necessary to beat the game. <b>Standard</b>: Dungeons may contain items necessary to beat the game. You can set the number of required bosses/dungeons in Additional Randomization Settings. With this setting certain randomly chosen dungeon bosses will drop required items (e.g. Triforce Shards). <b>Race Mode</b>: Dungeons without a required boss will not contain progress items."
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
      "Tingle Chests that are hidden in dungeons and must be bombed to make them appear (1 each in DRC, FW, TotG, ET, and WT).<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
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
      "progression_dungeon_secrets",
      "DRC, FW, TotG, ET, and WT each have 2-3 secret items within them (11 in total). This controls whether they can be progress items.<br>The items are fairly well-hidden (they aren't in chests), so don't select this option unless you're prepared to search each dungeon high and low!"
    },
    {
      "progression_obscure",
      "This controls whether obscure checks can contain progression items (e.g. Kane Windfall gate decorations).<br><u>If this is not checked, they will still be randomized</u>, but will only contain optional items you don't need to beat the game."
    },

    // Additional Randomization Options
    {
      "remove_swords",
      "Controls whether swords will be placed throughout the game. When they are removed, FF's Phantom Ganon is made vulnerable to Skull Hammer."
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
      "num_required_dungeons",
      "Select the number of requried bosses/dungeons. Required dungeon bosses will drop required items (e.g. Triforce Shards).",
    },
    {
      "randomize_charts",
      "Randomizes which sector is drawn on each Triforce/Treasure Chart."
    },
    {
      "chest_type_matches_contents",
      "Changes the chest type to reflect its contents. A metal chest has a progress item, a key chest has a dungeon key, and a wooden chest has a non-progress item or a consumable. Key chests are dark wood chests that use a custom texture based on Big Key chests. Keys for non-required dungeons in race mode will be in wooden chests."
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
      "Adds new warp pots that act as shortcuts connecting dungeons to each other directly (DRC, FW, TotG, and separately FF, ET, WT). Each pot must be unlocked before it can be used, so you cannot use them to access dungeons you wouldn't already have access to."
    },
    {
      "skip_rematch_bosses",
      "Removes the door in Ganon's Tower that only unlocks when you defeat the rematch versions of Gohma, Kalle Demos, Jalhalla, and Molgera."
    },
    {
      "invert_sea_compass_x_axis",
      "Inverts the east-west direction of the compass that shows while at sea.",
    },
    {
      "remove_music",
      "Mutes all ingame music."
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
      "progression_locations",
      "Randomized locations that can have progress items."
    },
    {
      "excluded_locations",
      "Randomized locations that cannot have progress items."
    },
    {
      "starting_pohs",
      "Amount of extra pieces of heart that you start with. If you start with less than 3 total hearts, and Chest Type Matches Contents is on, then heart containers will be put into metal chests."
    },
    {
      "starting_hcs",
      "Amount of heart containers that you start with. If you start with less than 3 total hearts, and Chest Type Matches Contents is on, then heart containers will be put into metal chests."
    },
    {
      "starting_joy_pendants",
      "Amount of extra joy pendants that you start with."
    },
    {
      "starting_skull_necklaces",
      "Amount of extra skull necklaces that you start with."
    },
    {
      "starting_boko_baba_seeds",
      "Amount of extra boko baba seeds that you start with."
    },
    {
      "starting_golden_feathers",
      "Amount of extra golden feathers that you start with."
    },
    {
      "starting_knights_crests",
      "Amount of extra knights crests that you start with."
    },
    {
      "starting_red_chu_jellys",
      "Amount of extra red chu jellys that you start with."
    },
    {
      "starting_green_chu_jellys",
      "Amount of extra green chu jellys that you start with."
    },
    {
      "starting_blue_chu_jellys",
      "Amount of extra blue chu jellys that you start with."
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
      "random_item_slide_item",
      "Randomly start with one first-person item to allow item sliding (Grappling Hook, Boomerang, Bow, or Hookshot). This option is aimed at glitch-heavy races where finding one of these items could massively change the outcome."
    },
    {
      "classic_mode",
      "Add back behaviors and glitches that were removed in the remake. Currently includes Wind Waker dives and dry storage. Only use these if you know what you are doing!"
    },
    {
      "plandomizer",
      "Allows you to select a file which can be used to manually place items in locations, or link specific entrances together in entrance randomizer."
    },
    {
      "fix_rng",
      "Certain RNG elements will have fixed outcomes. This currently only includes the Helmaroc King's attack pattern."
    },
    {
      "performance",
      "Mostly recommended for console users. Adjusts game code that causes performance issues, but may come at the cost of visual quality. Currently only affects particles."
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
      "Determines the number of path hints that will be placed in the game, distributed using the selected hint placement options.<br>If multiple hint placement options are selected, the hint count will be split evenly among the placement options.",
    },
    {
      "barren_hints",
      "Determines the number of barren hints that will be placed in the game, distributed using the selected hint placement options.<br>If multiple hint placement options are selected, the hint count will be split evenly among the placement options.",
    },
    {
      "location_hints",
      "Determines the number of location hints that will be placed in the game, distributed using the selected hint placement options.<br>If multiple hint placement options are selected, the hint count will be split evenly among the placement options.",
    },
    {
      "item_hints",
      "Determines the number of item hints that will be placed in the game, distributed using the selected hint placement options.<br>If multiple hint placement options are selected, the hint count will be split evenly among the placement options.",
    },
    {
      "clearer_hints",
      "When this option is selected, location and item hints will use the standard check or item name, instead of using cryptic hints.",
    },
    {
      "use_always_hints",
      "When the number of location hints is nonzero, certain locations that will always be hinted at will take precedence over normal location hints.",
    },
    {
      "hint_importance",
      "When this option is selected, item and location hints will also indicate if the hinted item is required, possibly required, or not required.<br>Only progress items will have these additions; non-progress items are trivially not required.",
    },

    // Entrance Randomizer
    {
      "randomize_dungeon_entrances",
      "Shuffles around which dungeon entrances take you into which dungeons."
    },
    {
      "randomize_boss_entrances",
      "Shuffles around which boss entrances take you to what boss."
    },
    {
      "randomize_miniboss_entrances",
      "Shuffles around which miniboss entrances take you to what miniboss. The DRC miniboss is not included in this due to having 2 entrances."
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
      "Shuffles around which misc entrances take you into which misc areas. Misc entrances are entrances which do not fall into any of the other categories and include entrances such as those that go in and out of Forest Haven and Dragon Roost Island. Hyrule is not included in this category."
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

    // In-Game Preferences
    {
      "target_type",
      "Set your in-game preference for the Target Type. This way you don't have to lose time changing it in game during a race."
    },
    {
      "camera",
      "Set your in-game preference for the Camera. This way you don't have to lose time changing it in game during a race."
    },
    {
      "first_person_camera",
      "Set your in-game preference for the First-Person Camera. This way you don't have to lose time changing it in game during a race."
    },
    {
      "gyroscope",
      "Set your in-game preference for the Gyroscope. This way you don't have to lose time changing it in game during a race."
    },
    {
      "ui_display",
      "Set your in-game preference for the UI Display. This way you don't have to lose time changing it in game during a race."
    },
};
