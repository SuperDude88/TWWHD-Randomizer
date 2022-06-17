#pragma once

#include <string>
#include <utility>

#include "server/command/RandoSession.hpp"
#include "server/filetypes/util/elfUtil.hpp"
#include "options.hpp"
#include "logic/World.hpp"
#include "logic/Location.hpp"
#include "logic/GameItem.hpp"



extern RandoSession g_session; //defined in randomizer.cpp, shared between main rando/logic/patches, easier than passing to every patch


/*
void set_new_game_starting_location(const uint8_t spawn_id, const uint8_t room_index);

void change_ship_starting_island(const uint8_t room_index);

void start_at_outset_dock();

void start_ship_at_outset();

void make_all_text_instant();

void fix_deku_leaf_model();

void allow_all_items_to_be_field_items();

void remove_shop_item_forced_uniqueness_bit();

void remove_ff2_cutscenes();

void make_items_progressive();

void add_ganons_tower_warp_to_ff2();

void add_chest_in_place_medli_gift();

void add_chest_in_place_queen_fairy_cutscene();

void add_more_magic_jars();

void modify_title_screen();

void update_name_and_icon();

void allow_dungeon_items_to_appear_anywhere();

void remove_bog_warp_in_cs();

void fix_shop_item_y_offsets();

void update_shop_item_descriptions(const Location& beedle20, const Location& beedle500, const Location& beedle950, const Location& beedle900);

void update_auction_item_names(const Location& auction5, const Location& auction40, const Location& auction60, const Location& auction80, const Location& auction100);

void update_battlesquid_item_names(const Location& firstPrize_, const Location& secondPrize_);

void update_item_names_in_letter_advertising_rock_spire_shop(const Location& beedle500, const Location& beedle950, const Location& beedle900);

void update_savage_labyrinth_hint_tablet(const Location& floor30, const Location& floor50);
*/

//hints

/*
void shorten_zephos_event();

void update_korl_dialog();

void set_num_starting_triforce_shards(const uint8_t numShards);

void set_starting_health(const uint16_t heartPieces, const uint16_t heartContainers);

void give_double_magic();

void set_damage_multiplier(float multiplier);

void set_pig_color(const PigColor& color);

void add_pirate_ship_to_windfall();

void add_cross_dungeon_warps();

void remove_makar_kidnapping();

void increase_crawl_speed();

void add_chart_number_to_item_get_messages();

void increase_grapple_animation_speed();

void increase_block_move_animation();

void increase_misc_animations();

void set_casual_clothes();

void hide_ship_sail();

void shorten_auction_intro_event();

void disable_invisible_walls();

void update_skip_rematch_bosses_game_variable(const bool skipRefights);

void update_sword_mode_game_variable(const SwordMode swordMode);

void update_starting_gear(const std::vector<GameItem>& startingItems);

void update_swordless_text();

void add_hint_signs();

void prevent_door_boulder_softlocks();

void update_tingle_statue_item_get_funcs();

void make_tingle_statue_reward_rupee_rainbow_colored();

void show_seed_hash_on_title_screen();
*/

//key bag

//required dungeon map markers

/*
void add_chest_in_place_jabun_cutscene();

void add_jabun_obstacles_to_default_layer();

void remove_jabun_stone_door_event();

void add_chest_in_place_master_sword();

void update_spoil_sell_text();

void fix_totg_warp_spawn();

void remove_phantom_ganon_req_for_reefs();

void fix_ff_door();

void add_failsafe_id_0_spawns();

void remove_minor_pan_cs();

void fix_stone_head_bugs();

void show_tingle_statues_on_quest_screen();
*/



void apply_necessary_tweaks(const Settings& settings, const std::string& seedHash);

void apply_necessary_post_randomization_tweaks(const bool randomizeItems, WorldPool& worlds, const PigColor& pigColor, const uint8_t& startIsland);

