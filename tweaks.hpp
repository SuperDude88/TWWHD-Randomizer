#include <cmath>
#include <string>
#include <fstream>
#include "libs/json.hpp"
#include "server/command/RandoSession.hpp"
#include "server/filetypes/bfres.hpp"
#include "server/filetypes/dzx.hpp"
#include "server/filetypes/events.hpp"
#include "server/filetypes/elf.hpp"
#include "server/filetypes/jpc.hpp"
#include "server/filetypes/msbt.hpp"
#include "server/utility/macros.hpp"

#include<iostream>

extern RandoSession g_session; //defined in randomizer.cpp, shared between main rando/logic/patches, easier than passing to every patch

bool containsAddress(int address, int memAddress, int sectionLen);

std::pair<int, int> AddressToOffset(int address);

std::pair<int, int> AddressToOffset(int address, int sectionIndex);

void write_u8_to_rpx(std::pair<int, int> offset, uint8_t data);

void write_u16_to_rpx(std::pair<int, int> offset, uint16_t data);

void write_u32_to_rpx(std::pair<int, int> offset, uint32_t data);

void write_float_to_rpx(std::pair<int, int> offset, float data);

void write_bytes_to_rpx(std::pair<int, int> offset, std::vector<uint8_t> Bytes);

uint8_t read_rpx_u8(std::pair<int, int> offset);

uint32_t read_rpx_u32(std::pair<int, int> offset);

float read_rpx_float(std::pair<int, int> offset);

std::vector<uint8_t> read_rpx_bytes(std::pair<int, int> offset, int NumBytes);

nlohmann::json Load_Patches(std::string file_path);

void Apply_Patch(std::string file_path);

nlohmann::json Load_Relocations(std::string file_path);

void Add_Relocations(nlohmann::json patches, std::string name);

//End of helper functions (might get moved into a separate file later)

void set_new_game_starting_location(uint8_t spawn_id, uint8_t room_index);

void change_ship_starting_island(int room_index);

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

std::u16string word_wrap_string(std::u16string string, int max_line_len = 34);

std::string get_indefinite_article(std::string string);

std::u16string get_indefinite_article(std::u16string string);

std::string upper_first_letter(std::string string);

std::u16string upper_first_letter(std::u16string string);

std::string pad_str_4_lines(std::string string);

std::u16string pad_str_4_lines(std::u16string string);

std::vector<std::string> split_lines(std::string string);

std::vector<std::u16string> split_lines(std::u16string string);

std::string merge_lines(std::vector<std::string> lines);

std::u16string merge_lines(std::vector<std::u16string> lines);

void remove_bog_warp_in_cs();

void fix_shop_item_y_offsets();

void update_shop_item_descriptions(std::string beedle20Item, std::string beedle500Item, std::string beedle950Item, std::string beedle900Item);

//hints

void shorten_zephos_event();

void update_korl_dialog();

//starting shards, health, magic

void add_pirate_ship_to_windfall();

void add_cross_dungeon_warps();

void remove_makar_kidnapping();

void increase_crawl_speed();

//chart numbers

void increase_grapple_animation_speed();

void increase_block_move_animation();

void increase_misc_animations();

//starting clothes, ship sail

void shorten_auction_intro_event();

void disable_invisible_walls();

//trials variable, sword mode, starting gear

void update_swordless_text();

void add_hint_signs();

void prevent_door_boulder_softlocks();

void update_tingle_statue_item_get_funcs();

//rainbow rupee

//seed hash

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
