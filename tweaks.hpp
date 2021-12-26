#include <cmath>
#include <string>
#include <fstream>
#include <locale>

#include "libs/json.hpp"
#include "server/command/RandoSession.hpp"
#include "server/filetypes/bfres.hpp"
#include "server/filetypes/dzx.hpp"
#include "server/filetypes/events.hpp"
#include "server/filetypes/elf.hpp"
#include "server/filetypes/jpc.hpp"
#include "server/filetypes/msbt.hpp"
#include "server/utility/macros.hpp"



extern RandoSession g_session; //defined in randomizer.cpp, shared between main rando/logic/patches, easier than passing to every patch



bool containsAddress(const int address, const int memAddress, const int sectionLen);

std::pair<int, int> AddressToOffset(const uint32_t address);

std::pair<int, int> AddressToOffset(const uint32_t address, const int sectionIndex);

void write_u8_to_rpx(const std::pair<int, int>& offset, const uint8_t data);

void write_u16_to_rpx(const std::pair<int, int>& offset, const uint16_t data);

void write_u32_to_rpx(const std::pair<int, int>& offset, const uint32_t data);

void write_float_to_rpx(const std::pair<int, int>& offset, const float data);

void write_bytes_to_rpx(const std::pair<int, int>& offset, const std::vector<uint8_t>& Bytes);

uint8_t read_rpx_u8(const std::pair<int, int>& offset);

uint32_t read_rpx_u32(const std::pair<int, int>& offset);

float read_rpx_float(const std::pair<int, int>& offset);

std::vector<uint8_t> read_rpx_bytes(const std::pair<int, int>& offset, const int NumBytes);

nlohmann::json Load_Patches(const std::string& file_path);

void Apply_Patch_OLD(const nlohmann::json& patches, const std::string& name);

void Apply_Patch(const std::string& file_path);

nlohmann::json Load_Relocations(const std::string& file_path);

void Add_Relocations(const nlohmann::json& in);

void Remove_Relocation(const std::pair<int, int>& offset);

void Load_Custom_Symbols(const std::string& file_path);



std::u16string word_wrap_string(const std::u16string& string, const int max_line_len);

std::string get_indefinite_article(const std::string& string);

std::u16string get_indefinite_article(const std::u16string& string);

std::string pad_str_4_lines(std::string& string);

std::u16string pad_str_4_lines(std::u16string& string);

std::vector<std::string> split_lines(std::string& string);

std::vector<std::u16string> split_lines(std::u16string& string);

std::string merge_lines(std::vector<std::string>& lines);

std::u16string merge_lines(std::vector<std::u16string>& lines);



void set_new_game_starting_location(uint8_t spawn_id, uint8_t room_index);

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

void update_shop_item_descriptions(const std::string& beedle20Item, const std::string& beedle500Item, const std::string& beedle950Item, const std::string& beedle900Item);

//hints, text updates

void shorten_zephos_event();

void update_korl_dialog();

void set_num_starting_triforce_shards(const uint8_t numShards);

void set_starting_health(const uint16_t heartPieces, const uint16_t heartContainers);

void give_double_magic();

void add_pirate_ship_to_windfall();

void add_cross_dungeon_warps();

void remove_makar_kidnapping();

void increase_crawl_speed();

//chart numbers

void increase_grapple_animation_speed();

void increase_block_move_animation();

void increase_misc_animations();

//starting clothes

void hide_ship_sail();

void shorten_auction_intro_event();

void disable_invisible_walls();

//trials variable, sword mode, starting gear

void update_swordless_text();

void add_hint_signs();

void prevent_door_boulder_softlocks();

void update_tingle_statue_item_get_funcs();

void make_tingle_statue_reward_rupee_rainbow_colored();

//seed hash

//key bag

//required dungeon map markers

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
