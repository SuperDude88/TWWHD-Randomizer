#include "tweaks.hpp"

#define _USE_MATH_DEFINES

#include <cmath>
#include <typeinfo>
#include <memory>
#include <fstream>
#include <codecvt>
#include <filesystem>

#include "libs/tinyxml2.h"
#include "libs/json.hpp"
#include "seedgen/seed.hpp"
#include "server/filetypes/bflim.hpp"
#include "server/filetypes/bflyt.hpp"
#include "server/filetypes/bfres.hpp"
#include "server/filetypes/dzx.hpp"
#include "server/filetypes/elf.hpp"
#include "server/filetypes/util/elfUtil.hpp"
#include "server/filetypes/events.hpp"
#include "server/filetypes/jpc.hpp"
#include "server/filetypes/msbt.hpp"
#include "server/filetypes/util/msbtMacros.hpp"
#include "server/utility/stringUtil.hpp"
#include "server/command/Log.hpp"

#define EXTRACT_ERR_CHECK(fspath) { \
    if(fspath.empty()) {\
        ErrorLog::getInstance().log(std::string("Failed to open file on line ") + std::to_string(__LINE__)); \
        return TweakError::FILE_OPEN_FAILED;  \
    } \
}

#define FILETYPE_ERROR_CHECK(func) {  \
    if(const auto error = func; error != decltype(error)::NONE) {\
        ErrorLog::getInstance().log(std::string("Encountered ") + &(typeid(error).name()[5]) + " on line " TOSTRING(__LINE__)); \
        return TweakError::FILETYPE_ERROR;  \
    } \
}

#define RPX_ERROR_CHECK(func) { \
    if(const ELFError error = func; error != ELFError::NONE) {\
        ErrorLog::getInstance().log(std::string("Failed rpx operation on line ") + std::to_string(__LINE__)); \
        return TweakError::RPX_OPERATION_FAILED;  \
    } \
}

#define TWEAK_ERR_CHECK(func) { \
    if(const TweakError error = func; error != TweakError::NONE) { \
        ErrorLog::getInstance().log(std::string("Encountered error in tweak ") + #func); \
        return error;  \
    } \
}

using eType = Utility::Endian::Type;

struct dungeon_item_info {
	std::u16string short_name;
	std::u16string base_item_name;
	uint8_t item_id;
};

struct CyclicWarpPotData {
	std::string stage_name;
	uint8_t room_num;
	float x, y, z;
	uint16_t y_rot;
	uint8_t event_reg_index;
};

struct spawn_data {
	std::string stage_name;
	uint8_t room_num = 0;
};

struct spawn_data_to_copy {
	std::string stage_name;
	uint8_t room_num = 0;
	uint8_t spawn_id_to_copy = 0;
};

struct pan_cs_info {
	std::string stage_name;
	std::string szs_suffix;
	uint8_t evnt_index;
};


namespace {
	FileTypes::ELF gRPX;
	static std::unordered_map<std::string, uint32_t> custom_symbols;

	static const std::unordered_map<std::string, std::string> progress_hints {
		{"WindWaker", "the wand of the wind conductor"},
		{"SpoilsBag", "a storage for collectibles"},
		{"GrapplingHook", "an item to swing across platforms"},
		{"PowerBracelets", "the strength to lift heavy rocks"},
		{"IronBoots", "a very heavy item"},
		{"BaitBag", "a storage for food"},
		{"Boomerang", "an item that always comes back to you"},
		{"Hookshot", "an item that reels"},
		{"DeliveryBag", "a storage for letters"},
		{"Bombs", "an explosive item"},
		{"SkullHammer", "the hammer of the dead"},
		{"DekuLeaf", "a magic leaf"},
		{"ProgressiveShield", "a defensive item"},
		{"TriforceShard1", "a piece of the power of the gods"},
		{"TriforceShard2", "a piece of the power of the gods"},
		{"TriforceShard3", "a piece of the power of the gods"},
		{"TriforceShard4", "a piece of the power of the gods"},
		{"TriforceShard5", "a piece of the power of the gods"},
		{"TriforceShard6", "a piece of the power of the gods"},
		{"TriforceShard7", "a piece of the power of the gods"},
		{"TriforceShard8", "a piece of the power of the gods"},
		{"NayrusPearl", "a blue jewel"},
		{"DinsPearl", "a red jewel"},
		{"FaroresPearl", "a green jewel"},
		{"WindsRequiem", "the song of wind"},
		{"BalladOfGales", "the song of gales"},
		{"CommandMelody", "the song of command"},
		{"EarthGodsLyric", "the song of earths god"},
		{"WindGodsAria", "the song of winds god"},
		{"SongOfPassing", "the song of time"},
		{"BoatsSail", "the wind follower"},
		{"NoteToMom", "the writings of a letter sorter"},
		{"MaggiesLetter", "the writings of a woman"},
		{"MoblinsLetter", "the writings of a creature"},
		{"CabanaDeed", "a pass for a private residence"},
		{"ProgressiveMagicMeter", "an upgrade for your magic"},
		{"GhostShipChart", "the chart of fears"},
		{"ProgressiveSword", "an upgrade for your blade"},
		{"ProgressiveBow", "an upgrade for your bow"},
		{"ProgressiveWallet", "an upgrade for your Rupee bag"},
		{"ProgressivePicto Box", "an upgrade for your camera"},
		{"EmptyBottle", "a glass container"},
		{"SmallKey", "a key"},
		{"BigKey", "an ominous key"},
		{"TreasureChart", "a blue map"},
		{"TriforceChart", "a purple map"},
		{"DragonTingleStatue", "a statue of a fairy"},
		{"ForbiddenTingleStatue", "a statue of a fairy"},
		{"GoddessTingleStatue", "a statue of a fairy"},
		{"EarthTingleStatue", "a statue of a fairy"},
		{"WindTingleStatue", "a statue of a fairy"},
		{"ProgressiveBombBag", "an upgrade for your bomb bag"},
		{"ProgressiveQuiver", "an upgrade for your quiver"}
	};

	TweakError Load_Custom_Symbols(const std::string& file_path) {
		std::ifstream fptr(file_path, std::ios::in);
		if(!fptr.is_open()) LOG_ERR_AND_RETURN(TweakError::DATA_FILE_MISSING);

		nlohmann::json symbols = nlohmann::json::parse(fptr);
		for (const auto& symbol : symbols.items()) {
			const uint32_t address = std::stoi(symbol.value().get<std::string>(), nullptr, 16);
			custom_symbols[symbol.key()] = address;
		}

		return TweakError::NONE;
	}

	
	std::u16string word_wrap_string(const std::u16string& string, const size_t& max_line_len) {
		size_t index_in_str = 0;
		std::u16string wordwrapped_str;
		std::u16string current_word;
		size_t curr_word_len = 0;
		size_t len_curr_line = 0;
	
		while (index_in_str < string.length()) { //length is weird because its utf-16
			char16_t character = string[index_in_str];
	
			if (character == u'\x0E') { //need to parse the commands, only implementing a few necessary ones for now (will break with other commands)
				std::u16string substr;
				size_t code_len = 0;
				if (string[index_in_str + 1] == u'\x00') {
					if (string[index_in_str + 2] == u'\x03') { //color command
						if (string[index_in_str + 4] == u'\xFFFF') { //text color white, weird length
							code_len = 10;
						}
						else {
							code_len = 5;
						}
					}
				}
				else if (string[index_in_str + 1] == u'\x01') { //all implemented commands in this group have length 4
					code_len = 4;
				}
				else if (string[index_in_str + 1] == u'\x02') { //all implemented commands in this group have length 4
					code_len = 4;
				}
				else if (string[index_in_str + 1] == u'\x03') { //all implemented commands in this group have length 4
					code_len = 4;
				}
	
				substr = string.substr(index_in_str, code_len);
				current_word += substr;
				index_in_str += code_len;
			}
			else if (character == u'\n') {
				wordwrapped_str += current_word;
				wordwrapped_str += character;
				len_curr_line = 0;
				current_word = u"";
				curr_word_len = 0;
				index_in_str += 1;
			}
			else if (character == u' ') {
				wordwrapped_str += current_word;
				wordwrapped_str += character;
				len_curr_line += curr_word_len + 1;
				current_word = u"";
				curr_word_len = 0;
				index_in_str += 1;
			}
			else {
				current_word += character;
				curr_word_len += 1;
				index_in_str += 1;
	
				if (len_curr_line + curr_word_len > max_line_len) {
					wordwrapped_str += u'\n';
					len_curr_line = 0;
	
					if (curr_word_len > max_line_len) {
						wordwrapped_str += current_word + u'\n';
						current_word = u"";
					}
				}
			}
		}
		wordwrapped_str += current_word;
	
		return wordwrapped_str;
	}
	
	std::string get_indefinite_article(const std::string& string) {
		char first_letter = std::tolower(string[0]);
		if (first_letter == 'a' || first_letter == 'e' || first_letter == 'i' || first_letter == 'o' || first_letter == 'u') {
			return "an";
		}
		else {
			return "a";
		}
	}
	
	std::u16string get_indefinite_article(const std::u16string& string) {
		const char16_t first_letter = std::tolower(string[0]);
		if (first_letter == u'a' || first_letter == u'e' || first_letter == u'i' || first_letter == u'o' || first_letter == u'u') {
			return u"an";
		}
		else {
			return u"a";
		}
	}
	
	std::string pad_str_4_lines(const std::string& string) {
		std::vector<std::string> lines = Utility::Str::split(string, '\n');
	
		unsigned int padding_lines_needed = (4 - lines.size() % 4) % 4;
		for (unsigned int i = 0; i < padding_lines_needed; i++) {
			lines.push_back("");
		}
	
		return Utility::Str::merge(lines, '\n');
	}
	
	std::u16string pad_str_4_lines(const std::u16string& string) {
		std::vector<std::u16string> lines = Utility::Str::split(string, u'\n');
	
		unsigned int padding_lines_needed = (4 - lines.size() % 4) % 4;
		for (unsigned int i = 0; i < padding_lines_needed; i++) {
			lines.push_back(u"");
		}
	
		return Utility::Str::merge(lines, u'\n');
	}

	std::string get_hint_item_name(const std::string& item_name) {
		if (item_name.find("TriforceChart") != std::string::npos) {
			return "TriforceChart";
		}
		if (item_name.find("TreasureChart") != std::string::npos) {
			return "TreasureChart";
		}
		if (item_name.find("SmallKey") != std::string::npos) {
			return "SmallKey";
		}
		if(item_name.find("BigKey") != std::string::npos) {
			return "BigKey";
		}

		return item_name;
	}
}

TweakError Apply_Patch(const std::string& file_path) {
	std::ifstream fptr(file_path, std::ios::in);
	if(!fptr.is_open()) LOG_ERR_AND_RETURN(TweakError::DATA_FILE_MISSING);

	const nlohmann::json patches = nlohmann::json::parse(fptr);

	for (const auto& patch : patches.items()) {
		const uint32_t offset = std::stoi(patch.key(), nullptr, 16);
		offset_t sectionOffset = elfUtil::AddressToOffset(gRPX, offset);
		if (!sectionOffset) { //address not in section
			std::string data;
			for (const std::string& byte : patch.value().get<std::vector<std::string>>()) {
				const char val = std::stoi(byte, nullptr, 16);
				data += val;
			}
			RPX_ERROR_CHECK(gRPX.extend_section(2, offset, data)); //add data at the specified offset
		}
		else {
			for (const std::string& byte : patch.value().get<std::vector<std::string>>()) {
				const uint8_t toWrite = std::stoi(byte, nullptr, 16);
				RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, sectionOffset, toWrite));
				sectionOffset.offset++; //Cycles through the bytes individually, need to increase the offset by one each time
			}
		}
	}

	return TweakError::NONE;
}

//only applies relocations for .rela.text
TweakError Add_Relocations(const std::string file_path) {
	std::ifstream fptr(file_path, std::ios::in);
	if(!fptr.is_open()) LOG_ERR_AND_RETURN(TweakError::DATA_FILE_MISSING);

	nlohmann::json relocations = nlohmann::json::parse(fptr);

	for (const auto& relocation : relocations) {
		if(!relocation.contains("r_offset") || !relocation.contains("r_info") || !relocation.contains("r_addend")) {
			LOG_ERR_AND_RETURN(TweakError::RELOCATION_MISSING_KEY);
		}

		Elf32_Rela reloc;
		reloc.r_offset = std::stoi(relocation.at("r_offset").get<std::string>(), nullptr, 16);
		reloc.r_info = std::stoi(relocation.at("r_info").get<std::string>(), nullptr, 16);
		reloc.r_addend = std::stoi(relocation.at("r_addend").get<std::string>(), nullptr, 16);

		RPX_ERROR_CHECK(elfUtil::addRelocation(gRPX, 7, reloc));
	}

	return TweakError::NONE;
}



TweakError set_new_game_starting_location(const uint8_t spawn_id, const uint8_t room_index) {
	RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, 0x025B508F), room_index));
	RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, 0x025B50CB), room_index));
	RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, 0x025B5093), spawn_id));
	RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, 0x025B50CF), spawn_id));
	
	return TweakError::NONE;
}

TweakError change_ship_starting_island(const uint8_t room_index) {
	std::string path;
	if (room_index == 1 || room_index == 11 || room_index == 13 || room_index == 17 || room_index == 23) {
		path = "content/Common/Pack/szs_permanent1.pack@SARC@sea_Room" + std::to_string(room_index) + ".szs@YAZ0@SARC@Room" + std::to_string(room_index) + ".bfres@BFRES@room.dzr";
	}
	else if (room_index == 9 || room_index == 39 || room_index == 41 || room_index == 44) {
		path = "content/Common/Pack/szs_permanent2.pack@SARC@sea_Room" + std::to_string(room_index) + ".szs@YAZ0@SARC@Room" + std::to_string(room_index) + ".bfres@BFRES@room.dzr";
	}
	else {
		path = "content/Common/Stage/sea_Room" + std::to_string(room_index) + ".szs@YAZ0@SARC@Room" + std::to_string(room_index) + ".bfres@BFRES@room.dzr";
	}
	RandoSession::fspath inPath = g_session.openGameFile(path);
	EXTRACT_ERR_CHECK(inPath);

	FileTypes::DZXFile room_dzr;
	FILETYPE_ERROR_CHECK(room_dzr.loadFromFile(inPath.string()));
	std::vector<ChunkEntry*> ship_spawns = room_dzr.entries_by_type("SHIP");
	ChunkEntry* ship_spawn_0 = nullptr;
	for (ChunkEntry* spawn : ship_spawns) { //Find spawn with ID 0
		if (*reinterpret_cast<uint8_t*>(&spawn->data[0xE]) == 0) ship_spawn_0 = spawn;
	}
	if(ship_spawn_0 == nullptr) LOG_ERR_AND_RETURN(TweakError::MISSING_ENTITY);

	FileTypes::DZXFile stage_dzs;
	RandoSession::fspath stagePath = g_session.openGameFile("content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs");
	EXTRACT_ERR_CHECK(stagePath);
	FILETYPE_ERROR_CHECK(stage_dzs.loadFromFile(stagePath.string()));
	std::vector<ChunkEntry*> actors = stage_dzs.entries_by_type("ACTR");
	for (ChunkEntry* actor : actors) {
		if (std::strncmp(&actor->data[0], "SHIP", 4) == 0) {
			actor->data.replace(0xC, 0xC, ship_spawn_0->data, 0x0, 0xC);
			actor->data.replace(0x1A, 0x2, ship_spawn_0->data, 0xC, 0x2);
			actor->data.replace(0x10, 0x4, "\xC8\xF4\x24\x00", 0x0, 0x4); //prevent softlock on fire mountain (may be wrong offset)
		}
	}
	FILETYPE_ERROR_CHECK(room_dzr.writeToFile(inPath.string()));
	FILETYPE_ERROR_CHECK(stage_dzs.writeToFile(stagePath.string()));

	return TweakError::NONE;
}

TweakError make_all_text_instant() {
	const RandoSession::fspath paths[4] = {
		"content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt",
		"content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt",
		"content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message3_msbt.szs@YAZ0@SARC@message3.msbt",
		"content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message4_msbt.szs@YAZ0@SARC@message4.msbt"
	};

	for (const auto& path : paths) {
		RandoSession::fspath inPath = g_session.openGameFile(path);
		EXTRACT_ERR_CHECK(inPath);
		FileTypes::MSBTFile msbt;
		FILETYPE_ERROR_CHECK(msbt.loadFromFile(inPath.string()));

		for (auto& [label, message] : msbt.messages_by_label) {
			std::u16string& String = message.text.message;

			message.attributes.drawType = 1; //draw instant

			std::u16string::size_type wait = String.find(u"\x0e\x01\x06\x02"s); //dont use macro because duration shouldnt matter
			while (wait != std::u16string::npos) {
				String.erase(wait, 5);
				wait = String.find(u"\x0e\x01\x06\x02"s);
			}

			std::u16string::size_type wait_dismiss = String.find(u"\x0e\x01\x03\x02"s); //dont use macro because duration shouldnt matter
			if (label == "07726" || label == "02488") wait_dismiss = std::u16string::npos; //exclude messages that are broken by removing the command
			while (wait_dismiss != std::u16string::npos) {
				String.erase(wait_dismiss, 5);
				wait_dismiss = String.find(u"\x0e\x01\x03\x02"s);
			}

			std::u16string::size_type wait_dismiss_prompt = String.find(u"\x0e\x01\x02\x02"s); //dont use macro because duration shouldnt matter
			while (wait_dismiss_prompt != std::u16string::npos) {
				String.erase(wait_dismiss_prompt, 5);
				wait_dismiss_prompt = String.find(u"\x0e\x01\x02\x02"s);
			}
		}

		FILETYPE_ERROR_CHECK(msbt.writeToFile(inPath.string()));
	}

	return TweakError::NONE;
}

TweakError fix_deku_leaf_model() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/Omori_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path)
	const std::string data = "item\x00\x00\x00\x00\x01\xFF\x02\x34\xc4\x08\x7d\x81\x45\x9d\x59\xec\xc3\xf5\x8e\xd9\x00\x00\x00\x00\x00\xff\xff\xff"s;

	FileTypes::DZXFile dzr;
	FILETYPE_ERROR_CHECK(dzr.loadFromFile(path.string()));
	std::vector<ChunkEntry*> actors = dzr.entries_by_type("ACTR");
	for (ChunkEntry* actor : actors) {
		if (std::strncmp(&actor->data[0], "itemDek\x00", 8) == 0) actor->data = data;
	}
	FILETYPE_ERROR_CHECK(dzr.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError allow_all_items_to_be_field_items() {

	static constexpr uint32_t item_resources_list_start = 0x101E4674;
	static constexpr uint32_t field_item_resources_list_start = 0x101E6A74;

	const std::array<uint8_t, 172> Item_Ids_Without_Field_Model = {
	0x1a, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x35, 0x36, 0x39, 0x3a, 0x3c, 0x3e, 0x3f, 0x42, 0x43, 0x4c, 0x4d, 0x4e, 0x50, 0x51, 0x52,
	0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x98,
	0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd,
	0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xde, 0xdd, 0xdf, 0xe0, 0xe1, 0xe2,
	0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe
	};

	const std::unordered_map<uint8_t, uint32_t> szs_name_pointers{
		{0x1a, 0x1004e5d8}, {0x20, 0x1004e414}, {0x21, 0x1004ec54}, {0x22, 0x1004e578}, {0x23, 0x1004e4a8}, {0x24, 0x1004e4d0}, {0x25, 0x1004e548}, {0x26, 0x1004e658}, {0x27, 0x1004e730}, {0x28, 0x1004e4f0}, {0x29, 0x1004e498}, {0x2a, 0x1004e550}, {0x2b, 0x1004e4a0}, {0x2c, 0x1004e4d8}, {0x2d, 0x1004e6b0}, {0x2e, 0x1004e5c0}, {0x2f, 0x1004e4e8}, {0x30, 0x1004e4c8}, {0x31, 0x1004e41c}, {0x32, 0x1004e5c0},
		{0x33, 0x1004e510}, {0x35, 0x1004e580}, {0x36, 0x1004e590}, {0x38, 0x1004e558}, {0x3c, 0x1004e560}, {0x3f, 0x1004e440}, {0x42, 0x1004e518}, {0x43, 0x1004e520}, {0x4c, 0x1004e4b8}, {0x4d, 0x1004e4b0}, {0x4e, 0x1004e698}, {0x50, 0x1004e430}, {0x51, 0x1004e538}, {0x52, 0x1004e530}, {0x53, 0x1004e528}, {0x54, 0x1004e5b0}, {0x55, 0x1004e5b0}, {0x56, 0x1004e5b8}, {0x57, 0x1004e5a0}, {0x58, 0x1004e5a8},
		{0x59, 0x1004e598}, {0x61, 0x1004e570}, {0x62, 0x1004e600}, {0x63, 0x1004e608}, {0x64, 0x1004e610}, {0x65, 0x1004e618}, {0x66, 0x1004e620}, {0x67, 0x1004e628}, {0x68, 0x1004e630}, {0x69, 0x1004ec24}, {0x6a, 0x1004ec3c}, {0x6b, 0x1004ec48}, {0x6c, 0x1004e518}, {0x6d, 0x1004e518}, {0x6e, 0x1004e518}, {0x6f, 0x1004e518}, {0x70, 0x1004e518}, {0x71, 0x1004e518}, {0x72, 0x1004e518}, {0x77, 0x1004e434},
		{0x78, 0x1004e434}, {0x79, 0x1004e638}, {0x7a, 0x1004e638}, {0x7b, 0x1004e638}, {0x7c, 0x1004e638}, {0x7d, 0x1004e638}, {0x7e, 0x1004e638}, {0x7f, 0x1004e638}, {0x80, 0x1004e638}, {0x98, 0x1004e5e0}, {0x99, 0x1004e5e8}, {0x9a, 0x1004e5f0}, {0x9b, 0x1004e5f8}, {0x9c, 0x1004e688}, {0x9d, 0x1004e500}, {0x9e, 0x1004e4f8}, {0x9f, 0x1004e658}, {0xa0, 0x1004e518}, {0xa1, 0x1004e518}, {0xa2, 0x1004e518},
		{0xa3, 0x1004e660}, {0xa4, 0x1004e668}, {0xa5, 0x1004e670}, {0xa6, 0x1004e678}, {0xa7, 0x1004e680}, {0xab, 0x1004e470}, {0xac, 0x1004e478}, {0xad, 0x1004e490}, {0xae, 0x1004e4a0}, {0xaf, 0x1004e480}, {0xb0, 0x1004e488}, {0xb3, 0x1004e5d8}, {0xb4, 0x1004e5d8}, {0xb5, 0x1004e5d8}, {0xb6, 0x1004e5d8}, {0xb7, 0x1004e5d8}, {0xb8, 0x1004e5d8}, {0xb9, 0x1004e5d8}, {0xba, 0x1004e5d8}, {0xbb, 0x1004e5d8},
		{0xbc, 0x1004e5d8}, {0xbd, 0x1004e5d8}, {0xbe, 0x1004e5d8}, {0xbf, 0x1004e5d8}, {0xc0, 0x1004e5d8}, {0xc1, 0x1004e5d8}, {0xc2, 0x1004e588}, {0xc3, 0x1004e588}, {0xc4, 0x1004e588}, {0xc5, 0x1004e588}, {0xc6, 0x1004e588}, {0xc7, 0x1004e588}, {0xc8, 0x1004e588}, {0xc9, 0x1004e588}, {0xca, 0x1004e588}, {0xcb, 0x1004e468}, {0xcc, 0x1004e640}, {0xcd, 0x1004e640}, {0xce, 0x1004e640}, {0xcf, 0x1004e640},
		{0xd0, 0x1004e640}, {0xd1, 0x1004e640}, {0xd2, 0x1004e640}, {0xd3, 0x1004e640}, {0xd4, 0x1004e640}, {0xd5, 0x1004e640}, {0xd6, 0x1004e640}, {0xd7, 0x1004e640}, {0xd8, 0x1004e640}, {0xd9, 0x1004e640}, {0xda, 0x1004e640}, {0xdb, 0x1004e650}, {0xdc, 0x1004e468}, {0xdd, 0x1004e640}, {0xde, 0x1004e640}, {0xdf, 0x1004e640}, {0xe0, 0x1004e640}, {0xe1, 0x1004e640}, {0xe2, 0x1004e640}, {0xe3, 0x1004e640},
		{0xe4, 0x1004e640}, {0xe5, 0x1004e640}, {0xe6, 0x1004e640}, {0xe7, 0x1004e640}, {0xe8, 0x1004e640}, {0xe9, 0x1004e640}, {0xea, 0x1004e640}, {0xeb, 0x1004e640}, {0xec, 0x1004e640}, {0xed, 0x1004e648}, {0xee, 0x1004e648}, {0xef, 0x1004e648}, {0xf0, 0x1004e648}, {0xf1, 0x1004e648}, {0xf2, 0x1004e648}, {0xf3, 0x1004e648}, {0xf4, 0x1004e648}, {0xf5, 0x1004e648}, {0xf6, 0x1004e648}, {0xf7, 0x1004e648},
		{0xf8, 0x1004e648}, {0xf9, 0x1004e648}, {0xfa, 0x1004e638}, {0xfb, 0x1004e648}, {0xfc, 0x1004e638}, {0xfd, 0x1004e648}, {0xfe, 0x1004e638}
	};

	for (const uint8_t& item_id : Item_Ids_Without_Field_Model) {
		uint32_t item_resources_addr_to_fix = 0x0;
		uint8_t item_id_to_copy_from = 0x0;
		if (item_id == 0x39 || item_id == 0x3a || item_id == 0x3e) {
			item_id_to_copy_from = 0x38;
			item_resources_addr_to_fix = item_resources_list_start + item_id * 0x24;
		}
		else if (item_id >= 0x6d && item_id <= 0x72) {
			item_id_to_copy_from = 0x22;
			item_resources_addr_to_fix = item_resources_list_start + item_id * 0x24;
		}
		else if (item_id == 0xb1 || item_id == 0xb2) {
			item_id_to_copy_from = 0x52;
			item_resources_addr_to_fix = item_resources_list_start + item_id * 0x24;
		}
		else {
			item_id_to_copy_from = item_id;
		}

		const uint32_t item_resources_addr_to_copy_from = item_resources_list_start + item_id_to_copy_from * 0x24;
		const uint32_t field_item_resources_addr = field_item_resources_list_start + item_id * 0x1c;
		uint32_t szs_name_pointer = 0;
		uint32_t section_start = 0x10000000;

		if (item_id == 0xAA) {
			//szs_name_pointer = custom_symbols.at("hurricane_spin_item_resource_arc_name");
			szs_name_pointer = szs_name_pointers.at(0x38); //issues with custom .szs currently, use sword model instead
			item_resources_addr_to_fix = item_resources_list_start + item_id * 0x24;


			//section_start = 0x02000000; //custom stuff only gets put in .text
			//not needed because we use the sword model (would be for a custom szs)
		}
		else {
			szs_name_pointer = szs_name_pointers.at(item_id_to_copy_from);
		}

		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr), szs_name_pointer));
		if (item_resources_addr_to_fix) {
			RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_fix), szs_name_pointer));
		}

		//need relocation entries so pointers work on console
		Elf32_Rela relocation;
		relocation.r_offset = field_item_resources_addr;
		if (section_start == 0x02000000) {
			relocation.r_info = 0x00000101; //need different .symtab index for .text pointer
		}
		else {
			relocation.r_info = 0x00000201;
		}
		relocation.r_addend = szs_name_pointer - section_start; //needs offset into the .rodata section (.text for vscroll), subtract start address from data location

		RPX_ERROR_CHECK(elfUtil::addRelocation(gRPX, 9, relocation));

		if (item_resources_addr_to_fix) {
			Elf32_Rela relocation2;
			relocation2.r_offset = item_resources_addr_to_fix;
			relocation2.r_info = relocation.r_info; //same as first entry
			relocation2.r_addend = relocation.r_addend; //same as first entry

			RPX_ERROR_CHECK(elfUtil::addRelocation(gRPX, 9, relocation2));
		}

		const std::vector<uint8_t> data1 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_copy_from + 8), 0xD);
		const std::vector<uint8_t> data2 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_copy_from + 0x1C), 4);
		RPX_ERROR_CHECK(elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr + 4), data1));
		RPX_ERROR_CHECK(elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr + 0x14), data2));
		if (item_resources_addr_to_fix) {
			RPX_ERROR_CHECK(elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_fix + 8), data1));
			RPX_ERROR_CHECK(elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_fix + 0x1C) , data2));
		}
	}

	for (const uint32_t& address : { 0x0255220CU, 0x02552214U, 0x0255221CU, 0x02552224U, 0x0255222CU, 0x02552234U, 0x02552450U }) { //unsigned to make compiler happy
		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, address), 0x60000000));
	}

	LOG_AND_RETURN_IF_ERR(Apply_Patch("./asm/patch_diffs/field_items_diff.json")); //some special stuff because HD silly

	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0007a2d0, 7), 0x00011ed8)); //Update the Y offset that is being read (.rela.text edit)

	const uint32_t extra_item_data_list_start = 0x101E8674;
	for (unsigned int item_id = 0x00; item_id < 0xFF + 1; item_id++) {
		const uint32_t item_extra_data_entry_addr = extra_item_data_list_start + 4 * item_id;
		const uint8_t original_y_offset = elfUtil::read_u8(gRPX, elfUtil::AddressToOffset(gRPX, item_extra_data_entry_addr + 1));
		if (original_y_offset == 0) {
			RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, item_extra_data_entry_addr + 1), 0x28));
		}
		uint8_t original_radius = elfUtil::read_u8(gRPX, elfUtil::AddressToOffset(gRPX, item_extra_data_entry_addr + 2));
		if (original_radius == 0) {
			RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, item_extra_data_entry_addr + 2), 0x28));
		}
	}
	//IMPROVEMENT: make vscroll not crash + add it to code
	//WWHD changes make this much more complex ^

	return TweakError::NONE;
}

TweakError remove_shop_item_forced_uniqueness_bit() {
	const uint32_t shop_item_data_list_start = 0x101eaea4;

	for (const uint8_t& shop_item_index : { 0x0, 0xB, 0xC, 0xD }) {
		const uint32_t shop_item_data_addr = shop_item_data_list_start + shop_item_index * 0x10;
		uint8_t buy_requirements_bitfield = elfUtil::read_u8(gRPX, elfUtil::AddressToOffset(gRPX, shop_item_data_addr + 0xC));
		buy_requirements_bitfield = (buy_requirements_bitfield & (~0x2));
		RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, shop_item_data_addr + 0xC), buy_requirements_bitfield));
	}

	return TweakError::NONE;
}

TweakError remove_ff2_cutscenes() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/M2tower_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path);

	FileTypes::DZXFile dzr;
	FILETYPE_ERROR_CHECK(dzr.loadFromFile(path.string()));

	std::vector<ChunkEntry*> spawns = dzr.entries_by_type("PLYR");
	for (ChunkEntry* spawn : spawns) {
		if (spawn->data[29] == '\x10') {
			spawn->data[8] = '\xFF';
			break;
		}
	}

	std::vector<ChunkEntry*> exits = dzr.entries_by_type("SCLS");
	for (ChunkEntry* exit : exits) {
		if (std::strncmp(&exit->data[0], "M2ganon\x00", 8) == 0) {
			exit->data = "sea\x00\x00\x00\x00\x00\x00\x01\x00\xFF"s;
			break;
		}
	}
	FILETYPE_ERROR_CHECK(dzr.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError make_items_progressive() {
	LOG_AND_RETURN_IF_ERR(Apply_Patch("./asm/patch_diffs/make_items_progressive_diff.json"));

	const uint32_t item_get_func_pointer = 0x0001DA54; //First relevant relocation entry in .rela.data (overwrites .data section when loaded)

	for (const uint8_t sword_id : {0x38, 0x39, 0x3A, 0x3D, 0x3E}) {
		const uint32_t item_get_func_addr = item_get_func_pointer + (sword_id * 0xC) + 8;
		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_sword_item_func") - 0x02000000));
	}
	for (const uint8_t shield_id : {0x3B, 0x3C}) {
		const uint32_t item_get_func_addr = item_get_func_pointer + (shield_id * 0xC) + 8;
		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_shield_item_func") - 0x02000000));
	}
	for (const uint8_t bow_id : {0x27, 0x35, 0x36}) {
		const uint32_t item_get_func_addr = item_get_func_pointer + (bow_id * 0xC) + 8;
		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_bow_item_func") - 0x02000000));
	}
	for (const uint8_t wallet_id : {0xAB, 0xAC}) {
		const uint32_t item_get_func_addr = item_get_func_pointer + (wallet_id * 0xC) + 8;
		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_wallet_item_func") - 0x02000000));
	}
	for (const uint8_t bomb_bag_id : {0xAD, 0xAE}) {
		const uint32_t item_get_func_addr = item_get_func_pointer + (bomb_bag_id * 0xC) + 8;
		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_bomb_bag_item_func") - 0x02000000));
	}
	for (const uint8_t quiver_id : {0xAF, 0xB0}) {
		const uint32_t item_get_func_addr = item_get_func_pointer + (quiver_id * 0xC) + 8;
		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_quiver_item_func") - 0x02000000));
	}
	for (const uint8_t picto_id : {0x23, 0x26}) {
		const uint32_t item_get_func_addr = item_get_func_pointer + (picto_id * 0xC) + 8;
		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_picto_box_item_func") - 0x02000000));
	}
	for (const uint8_t sail_id : {0x77, 0x78}) {
		const uint32_t item_get_func_addr = item_get_func_pointer + (sail_id * 0xC) + 8;
		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_sail_item_func") - 0x02000000));
	}
	for (const uint8_t magic_id : {0xB1, 0xB2}) {
		const uint32_t item_get_func_addr = item_get_func_pointer + (magic_id * 0xC) + 8;
		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_magic_meter_item_func") - 0x02000000));
	}

	//nop some code that sets max bombs and arrows to 30
	//This avoids downgrading bomb bags or quivers
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0254e8c4), 0x60000000));
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0254e8cc), 0x60000000));
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0254e66c), 0x60000000));
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0254e674), 0x60000000));

	//Modify the deku leaf to skip giving you magic
	//Instead we make magic its own item that the player starts with by default
	//Allows other items to use magic before getting leaf
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0254e96c), 0x60000000));
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0254e97c), 0x60000000));

	// Add an item get message for the normal magic meter since it didn't have one in vanilla
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");
	EXTRACT_ERR_CHECK(path)

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));

	const Message& to_copy = msbt.messages_by_label["00" + std::to_string(101 + 0xB2)];
	const std::u16string message = DRAW_INSTANT + u"You got " + TEXT_COLOR_RED + u"magic power" + TEXT_COLOR_DEFAULT + u"!\nNow you can use magic items!\0"s;
	msbt.addMessage("00" + std::to_string(101 + 0xB1), to_copy.attributes, to_copy.style, message);

	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));
	return TweakError::NONE;
}

TweakError add_ganons_tower_warp_to_ff2() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/szs_permanent1.pack@SARC@sea_Room1.szs@YAZ0@SARC@Room1.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path);

	FileTypes::DZXFile dzr;
	FILETYPE_ERROR_CHECK(dzr.loadFromFile(path.string()));
	ChunkEntry& warp = dzr.add_entity("ACTR", 1);
	warp.data = "Warpmj\x00\x00\x00\x00\x00\x11\xc8\x93\x0f\xd9\x00\x00\x00\x00\xc8\x91\xf7\xfa\x00\x00\x00\x00\x00\x00\xff\xff"s;
	FILETYPE_ERROR_CHECK(dzr.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError add_chest_in_place_medli_gift() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/M_Dra09_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs");

	FileTypes::DZXFile dzs;
	FILETYPE_ERROR_CHECK(dzs.loadFromFile(path.string()));
	ChunkEntry& chest = dzs.add_entity("TRES");
	chest.data = "takara3\x00\xFF\x20\x08\x80\xc4\xca\x99\xec\x46\x54\x80\x00\x43\x83\x84\x5a\x00\x09\xcc\x16\x0f\xff\xff\xff"s;
	FILETYPE_ERROR_CHECK(dzs.writeToFile(path.string()));

	RandoSession::fspath path2 = g_session.openGameFile("content/Common/Stage/M_NewD2_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs");

	FileTypes::DZXFile dzs2;
	FILETYPE_ERROR_CHECK(dzs2.loadFromFile(path2.string()));
	ChunkEntry& dummyChest = dzs2.add_entity("TRES");
	dummyChest.data = "takara3\x00\xFF\x20\x08\x80\xc4\xca\x99\xec\x46\x54\x80\x00\x43\x83\x84\x5a\x00\x09\xcc\x16\x0f\xff\xff\xff"s;
	FILETYPE_ERROR_CHECK(dzs2.writeToFile(path2.string()));
	return TweakError::NONE;
}

TweakError add_chest_in_place_queen_fairy_cutscene() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/szs_permanent2.pack@SARC@sea_Room9.szs@YAZ0@SARC@Room9.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path);

	FileTypes::DZXFile dzr;
	FILETYPE_ERROR_CHECK(dzr.loadFromFile(path.string()));
	ChunkEntry& chest = dzr.add_entity("TRES");
	chest.data = "takara3\x00\xFF\x20\x0e\x00\xc8\x2f\xcf\xc0\x44\x34\xc0\x00\xc8\x43\x4e\xc0\x00\x09\x10\x00\xa5\xff\xff\xff"s;
	FILETYPE_ERROR_CHECK(dzr.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError add_more_magic_jars() {
	{
		RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/M_NewD2_Room2.szs@YAZ0@SARC@Room2.bfres@BFRES@room.dzr");
		EXTRACT_ERR_CHECK(path);

		FileTypes::DZXFile drc_hub;
		FILETYPE_ERROR_CHECK(drc_hub.loadFromFile(path.string()));
		std::vector<ChunkEntry*> actors = drc_hub.entries_by_type("ACTR");
		std::vector<ChunkEntry*> skulls;
		for (ChunkEntry* actor : actors) {
			if (std::strncmp(&actor->data[0], "Odokuro\x00", 8) == 0) skulls.push_back(actor);
		}

		if(skulls.size() < 6) LOG_ERR_AND_RETURN(TweakError::MISSING_ENTITY);
		
		skulls[2]->data.replace(0x8, 0x4, "\x75\x7f\xff\x09", 0, 4);
		skulls[5]->data.replace(0x8, 0x4, "\x75\x7f\xff\x0A", 0, 4);
		FILETYPE_ERROR_CHECK(drc_hub.writeToFile(path.string()));
	}

	{
		RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/M_NewD2_Room10.szs@YAZ0@SARC@Room10.bfres@BFRES@room.dzr");
		EXTRACT_ERR_CHECK(path)

		FileTypes::DZXFile drc_before_boss;
		FILETYPE_ERROR_CHECK(drc_before_boss.loadFromFile(path.string()));
		std::vector<ChunkEntry*> actors = drc_before_boss.entries_by_type("ACTR");
		std::vector<ChunkEntry*> skulls;
		for (ChunkEntry* actor : actors) {
			if (std::strncmp(&actor->data[0], "Odokuro\x00", 8) == 0) skulls.push_back(actor);
		}

		if(skulls.size() < 11) LOG_ERR_AND_RETURN(TweakError::MISSING_ENTITY);

		skulls[0]->data.replace(0x8, 0x4, "\x75\x7f\xff\x0A", 0, 4);
		skulls[9]->data.replace(0x8, 0x4, "\x75\x7f\xff\x0A", 0, 4);
		
		FILETYPE_ERROR_CHECK(drc_before_boss.writeToFile(path.string()));
	}

	{
		RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/szs_permanent1.pack@SARC@sea_Room13.szs@YAZ0@SARC@Room13.bfres@BFRES@room.dzr");
		EXTRACT_ERR_CHECK(path);

		FileTypes::DZXFile dri;
		FILETYPE_ERROR_CHECK(dri.loadFromFile(path.string()));
		ChunkEntry& grass1 = dri.add_entity("ACTR");
		grass1.data = "\x6B\x75\x73\x61\x78\x31\x00\x00\x00\x00\x0E\x00\x48\x4C\xC7\x80\x44\xED\x80\x00\xC8\x45\xB7\xC0\x00\x00\x00\x00\x00\x00\xFF\xFF"s;
		ChunkEntry& grass2 = dri.add_entity("ACTR");
		grass2.data = "\x6B\x75\x73\x61\x78\x31\x00\x00\x00\x00\x0E\x00\x48\x4C\x6D\x40\x44\xA2\x80\x00\xC8\x4D\x38\x40\x00\x00\x00\x00\x00\x00\xFF\xFF"s;
		FILETYPE_ERROR_CHECK(dri.writeToFile(path.string()));
	}

	{
		RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/Siren_Room14.szs@YAZ0@SARC@Room14.bfres@BFRES@room.dzr");
		EXTRACT_ERR_CHECK(path);

		FileTypes::DZXFile totg;
		FILETYPE_ERROR_CHECK(totg.loadFromFile(path.string()));
		std::vector<ChunkEntry*> actors = totg.entries_by_type("ACTR");
		std::vector<ChunkEntry*> pots;
		for (ChunkEntry* actor : actors) {
			if (std::strncmp(&actor->data[0], "kotubo\x00\x00", 8) == 0) pots.push_back(actor);
		}

		if(pots.size() < 2) LOG_ERR_AND_RETURN(TweakError::MISSING_ENTITY);

		pots[1]->data = "\x6B\x6F\x74\x75\x62\x6F\x00\x00\x70\x7F\xFF\x0A\xC5\x6E\x20\x00\x43\x66\x00\x05\xC5\xDF\xC0\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF"s;
		FILETYPE_ERROR_CHECK(totg.writeToFile(path.string()));
	}

	return TweakError::NONE;
}

TweakError modify_title_screen() {
	using namespace NintendoWare::Layout;

	RandoSession::fspath path = g_session.openGameFile("content/Common/Layout/Title_00.szs@YAZ0@SARC@blyt/Title_00.bflyt");
	EXTRACT_ERR_CHECK(path)

	FileTypes::FLYTFile layout;
	FILETYPE_ERROR_CHECK(layout.loadFromFile(path.string()));

	//add version number
	Pane& newPane = layout.rootPane.children[0].children[1].children[3].duplicateChildPane(1); //unused version number text
	newPane.pane->name = "T_Version";
	newPane.pane->name.resize(0x18);
	dynamic_cast<txt1*>(newPane.pane.get())->text = u"Ver " + Utility::Str::toUTF16(RANDOMIZER_VERSION) + u'\0';
	dynamic_cast<txt1*>(newPane.pane.get())->fontIndex = 0;
	dynamic_cast<txt1*>(newPane.pane.get())->restrictedLen = 0x1C;
	dynamic_cast<txt1*>(newPane.pane.get())->lineAlignment = txt1::LineAlignment::CENTER;

	//update "HD" image position
	layout.rootPane.children[0].children[1].children[1].children[0].pane->translation.X = -165.0f;
	layout.rootPane.children[0].children[1].children[1].children[0].pane->translation.Y = 12.0f;

	//update subtitle size/position
	layout.rootPane.children[0].children[1].children[1].children[1].children[0].pane->translation.Y = -30.0f;
	layout.rootPane.children[0].children[1].children[1].children[1].children[0].pane->height = 120.0f;

	//update subtitle mask size/position
	layout.rootPane.children[0].children[1].children[1].children[1].children[1].pane->translation.Y = -30.0f;
	layout.rootPane.children[0].children[1].children[1].children[1].children[1].pane->height = 120.0f;

	FILETYPE_ERROR_CHECK(layout.writeToFile(path.string()));

	//update textures
	FileTypes::FLIMFile title;
	FileTypes::FLIMFile subtitle;
	FileTypes::FLIMFile mask;

	path = g_session.openGameFile("content/Common/Layout/Title_00.szs@YAZ0@SARC@timg/TitleLogoZelda_00^l.bflim");
	EXTRACT_ERR_CHECK(path);
	
	FILETYPE_ERROR_CHECK(title.loadFromFile(path.string()));
	FILETYPE_ERROR_CHECK(title.replaceWithDDS("./assets/Title.dds", GX2TileMode::GX2_TILE_MODE_DEFAULT, 0, true));
	FILETYPE_ERROR_CHECK(title.writeToFile(path.string()));
	
	path = g_session.openGameFile("content/Common/Layout/Title_00.szs@YAZ0@SARC@timg/TitleLogoWindwaker_00^l.bflim");
	EXTRACT_ERR_CHECK(path);
	
	FILETYPE_ERROR_CHECK(subtitle.loadFromFile(path.string()));
	FILETYPE_ERROR_CHECK(subtitle.replaceWithDDS("./assets/Subtitle.dds", GX2TileMode::GX2_TILE_MODE_DEFAULT, 0, true));
	FILETYPE_ERROR_CHECK(subtitle.writeToFile(path.string()));
	
	path = g_session.openGameFile("content/Common/Layout/Title_00.szs@YAZ0@SARC@timg/TitleLogoWindwakerMask_00^s.bflim");
	EXTRACT_ERR_CHECK(path);
	
	FILETYPE_ERROR_CHECK(mask.loadFromFile(path.string()));
	FILETYPE_ERROR_CHECK(mask.replaceWithDDS("./assets/SubtitleMask.dds", GX2TileMode::GX2_TILE_MODE_DEFAULT, 0, false));
	FILETYPE_ERROR_CHECK(mask.writeToFile(path.string()));

	//update sparkle size/position
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x101f7048), 0x3fb33333)); //scale
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x101f7044), 0x40100000)); //possibly particle size, JP changes it for its larger title text
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x10108280), 0xc2180000)); //vertical position
	
	return TweakError::NONE;
}

TweakError update_name_and_icon() {
	if(!g_session.copyToGameFile("./assets/iconTex.tga", "meta/iconTex.tga")) LOG_ERR_AND_RETURN(TweakError::FILE_COPY_FAILED);

	tinyxml2::XMLDocument meta;
	const RandoSession::fspath metaPath = g_session.openGameFile("meta/meta.xml").string();
	EXTRACT_ERR_CHECK(metaPath);
	meta.LoadFile(metaPath.string().c_str());
	tinyxml2::XMLElement* root = meta.RootElement();
	root->FirstChildElement("longname_en")->SetText("THE LEGEND OF ZELDA\nThe Wind Waker HD Randomizer");
	root->FirstChildElement("longname_fr")->SetText("THE LEGEND OF ZELDA\nThe Wind Waker HD Randomizer");
	root->FirstChildElement("longname_es")->SetText("THE LEGEND OF ZELDA\nThe Wind Waker HD Randomizer");
	root->FirstChildElement("longname_pt")->SetText("THE LEGEND OF ZELDA\nThe Wind Waker HD Randomizer");

	root->FirstChildElement("shortname_en")->SetText("The Wind Waker HD Randomizer");
	root->FirstChildElement("shortname_fr")->SetText("The Wind Waker HD Randomizer");
	root->FirstChildElement("shortname_es")->SetText("The Wind Waker HD Randomizer");
	root->FirstChildElement("shortname_pt")->SetText("The Wind Waker HD Randomizer");

	meta.SaveFile(metaPath.string().c_str());
	
	return TweakError::NONE;
}

TweakError allow_dungeon_items_to_appear_anywhere() {
	const uint32_t item_get_func_pointer = 0x0001DA54; //First relevant relocation entry in .rela.data (overwrites .data section when loaded)
	const uint32_t item_resources_list_start = 0x101E4674;
	const uint32_t field_item_resources_list_start = 0x101E6A74;

	const std::unordered_map<std::u16string, std::u16string> dungeon_names = {
		{u"DRC", u"Dragon Roost Cavern"},
		{u"FW", u"Forbidden Woods"},
		{u"TotG", u"Tower of the Gods"},
		{u"FF", u"Forsaken Fortress"},
		{u"ET", u"Earth Temple"},
		{u"WT", u"Wind Temple"}
	};

	const std::unordered_map<std::u16string, uint8_t> item_name_to_id{ {
		{u"Small Key", 0x15},
		{u"Dungeon Map", 0x4C},
		{u"Compass", 0x4D},
		{u"Big Key", 0x4E}
	} };

	const std::array<dungeon_item_info, 22> dungeon_items{ {
		{u"DRC", u"Small Key", 0x13},
		{u"DRC", u"Big Key", 0x14},
		{u"DRC", u"Dungeon Map", 0x1B},
		{u"DRC", u"Compass", 0x1C},
		{u"FW", u"Small Key", 0x1D},
		{u"FW", u"Big Key", 0x40},
		{u"FW", u"Dungeon Map", 0x41},
		{u"FW", u"Compass", 0x5A},
		{u"TotG", u"Small Key", 0x5B},
		{u"TotG", u"Big Key", 0x5C},
		{u"TotG", u"Dungeon Map", 0x5D},
		{u"TotG", u"Compass", 0x5E},
		{u"FF", u"Dungeon Map", 0x5F},
		{u"FF", u"Compass", 0x60},
		{u"ET", u"Small Key", 0x73},
		{u"ET", u"Big Key", 0x74},
		{u"ET", u"Dungeon Map", 0x75},
		{u"ET", u"Compass", 0x76},
		{u"WT", u"Small Key", 0x81}, //0x77 is taken by swift sail in HD
		{u"WT", u"Big Key", 0x84},
		{u"WT", u"Dungeon Map", 0x85},
		{u"WT", u"Compass", 0x86}
	} };

	const std::unordered_map<uint8_t, std::string> idToFunc = {
		{0x13, "drc_small_key_item_get_func"},
		{0x14, "drc_big_key_item_get_func"},
		{0x1B, "drc_dungeon_map_item_get_func"},
		{0x1C, "drc_compass_item_get_func"},
		{0x1D, "fw_small_key_item_get_func"},
		{0x40, "fw_big_key_item_get_func"},
		{0x41, "fw_dungeon_map_item_get_func"},
		{0x5A, "fw_compass_item_get_func"},
		{0x5B, "totg_small_key_item_get_func"},
		{0x5C, "totg_big_key_item_get_func"},
		{0x5D, "totg_dungeon_map_item_get_func"},
		{0x5E, "totg_compass_item_get_func"},
		{0x5F, "ff_dungeon_map_item_get_func"},
		{0x60, "ff_compass_item_get_func"},
		{0x73, "et_small_key_item_get_func"},
		{0x74, "et_big_key_item_get_func"},
		{0x75, "et_dungeon_map_item_get_func"},
		{0x76, "et_compass_item_get_func"},
		{0x81, "wt_small_key_item_get_func"},
		{0x84, "wt_big_key_item_get_func"},
		{0x85, "wt_dungeon_map_item_get_func"},
		{0x86, "wt_compass_item_get_func"},
	};

	const std::unordered_map<uint8_t, uint32_t> szs_name_pointers{
		{0x15, 0x1004E448},
		{0x4C, 0x1004E4b8},
		{0x4D, 0x1004E4b0},
		{0x4E, 0x1004E698}
	};

	//remove a store that would overwrite a change to the DRC boss key model (nintendo added some item at this ID, seems unused?)
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x02551E30), 0x60000000));

	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");
	EXTRACT_ERR_CHECK(path)

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));
	for (const dungeon_item_info& item_data : dungeon_items) {
		const std::u16string item_name = item_data.short_name + u" " + item_data.base_item_name;
		const uint8_t base_item_id = item_name_to_id.at(item_data.base_item_name);
		const std::u16string dungeon_name = dungeon_names.at(item_data.short_name);

		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_pointer + (0xC * item_data.item_id) + 0x8, 9), custom_symbols.at(idToFunc.at(item_data.item_id)) - 0x02000000)); //write to the relocation entries
		
		const uint32_t message_id = 101 + item_data.item_id;
		const Message& to_copy = msbt.messages_by_label["00" + std::to_string(101 + base_item_id)];
		std::u16string message = DRAW_INSTANT + u"You got ";
		if (item_data.base_item_name == u"Small Key") {
			message += get_indefinite_article(dungeon_name) + u" " + TEXT_COLOR_RED + dungeon_name + u" small key" + TEXT_COLOR_DEFAULT + u"!\0"s;
		} 
		else if (item_data.base_item_name == u"Big Key") {
			message +=u"the " + TEXT_COLOR_RED + dungeon_name + u" Big Key" + TEXT_COLOR_DEFAULT + u"!\0"s;
		}
		else if (item_data.base_item_name == u"Dungeon Map") {
			message +=u"the " + TEXT_COLOR_RED + dungeon_name + u" Dungeon Map" + TEXT_COLOR_DEFAULT + u"!\0"s;
		}
		else if (item_data.base_item_name == u"Compass") {
			message +=u"the " + TEXT_COLOR_RED + dungeon_name + u" Compass" + TEXT_COLOR_DEFAULT + u"!\0"s;
		}
		
		word_wrap_string(message, 34);
		msbt.addMessage("00" + std::to_string(message_id), to_copy.attributes, to_copy.style, message);

		const uint32_t item_resources_addr_to_copy_from = item_resources_list_start + base_item_id * 0x24;
		const uint32_t field_item_resources_addr_to_copy_from = field_item_resources_list_start + base_item_id * 0x24;

		const uint32_t item_resources_addr = item_resources_list_start + item_data.item_id * 0x24;
		const uint32_t field_item_resources_addr = field_item_resources_list_start + item_data.item_id * 0x1C;

		const uint32_t szs_name_pointer = szs_name_pointers.at(base_item_id);

		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr), szs_name_pointer));
		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr), szs_name_pointer));

		//need relocation entries so pointers work on console
		Elf32_Rela relocation;
		relocation.r_offset = field_item_resources_addr;
		relocation.r_info = 0x00000201;
		relocation.r_addend = szs_name_pointer - 0x10000000; //needs offset into .rodata section, subtract start address from data location

		Elf32_Rela relocation2;
		relocation2.r_offset = item_resources_addr;
		relocation2.r_info = relocation.r_info; //same as first entry
		relocation2.r_addend = relocation.r_addend; //same as first entry

		RPX_ERROR_CHECK(elfUtil::addRelocation(gRPX, 9, relocation));
		RPX_ERROR_CHECK(elfUtil::addRelocation(gRPX, 9, relocation2));

		const std::vector<uint8_t> data1 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_copy_from + 8), 0xD);
		RPX_ERROR_CHECK(elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr + 8), data1));
		RPX_ERROR_CHECK(elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr + 4), data1));
		
		const std::vector<uint8_t> data2 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_copy_from + 0x1C), 4);
		RPX_ERROR_CHECK(elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr + 0x1C), data2));
		RPX_ERROR_CHECK(elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr + 0x14), data2));

		const std::vector<uint8_t> data3 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_copy_from + 0x15), 0x7);
		RPX_ERROR_CHECK(elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr + 0x15), data3));
		
		const std::vector<uint8_t> data4 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_copy_from + 0x20), 0x4);
		RPX_ERROR_CHECK(elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr + 0x20), data4));

		const std::vector<uint8_t> data5 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr_to_copy_from + 0x11), 0x3);
		RPX_ERROR_CHECK(elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr + 0x11), data5));
		
		const std::vector<uint8_t> data6 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr_to_copy_from + 0x18), 0x4);
		RPX_ERROR_CHECK(elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr + 0x18), data6));
		
	}
	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError remove_bog_warp_in_cs() {
	for (uint8_t i = 1; i < 50; i++) {
		std::string path;
		if (i == 1 || i == 11 || i == 13 || i == 17 || i == 23) {
			path = "content/Common/Pack/szs_permanent1.pack@SARC@sea_Room" + std::to_string(i) + ".szs@YAZ0@SARC@Room" + std::to_string(i) + ".bfres@BFRES@room.dzr";
		}
		else if (i == 9 || i == 39 || i == 41 || i == 44) {
			path = "content/Common/Pack/szs_permanent2.pack@SARC@sea_Room" + std::to_string(i) + ".szs@YAZ0@SARC@Room" + std::to_string(i) + ".bfres@BFRES@room.dzr";
		}
		else {
			path = "content/Common/Stage/sea_Room" + std::to_string(i) + ".szs@YAZ0@SARC@Room" + std::to_string(i) + ".bfres@BFRES@room.dzr";
		}
		RandoSession::fspath filePath = g_session.openGameFile(path);
		EXTRACT_ERR_CHECK(filePath);

		FileTypes::DZXFile room_dzr;
		FILETYPE_ERROR_CHECK(room_dzr.loadFromFile(filePath.string()));
		for (ChunkEntry* spawn : room_dzr.entries_by_type("PLYR")) {
			uint8_t spawn_type = ((*reinterpret_cast<uint8_t*>(&spawn->data[0xB]) & 0xF0) >> 4);
			if (spawn_type == 0x09) {
				spawn->data[0xB] = (spawn->data[0xB] & 0x0F) | 0x20;
			}
		}
		FILETYPE_ERROR_CHECK(room_dzr.writeToFile(filePath.string()));
	}

	return TweakError::NONE;
}

TweakError fix_shop_item_y_offsets() {
	const uint32_t shop_item_display_data_list_start = 0x1003A930;
	const std::unordered_set<uint8_t> ArrowID = { 0x10, 0x11, 0x12 };

	for (unsigned int id = 0; id < 0xFF + 1; id++) {
		const uint32_t display_data_addr = shop_item_display_data_list_start + id * 0x20;
		const float y_offset = elfUtil::read_float(gRPX, elfUtil::AddressToOffset(gRPX, display_data_addr + 0x10));

		if (y_offset == 0.0f && ArrowID.count(id) == 0) {
			//If the item didn't originally have a Y offset we need to give it one so it's not sunken into the pedestal.
			// Only exception are for items 10 11 and 12 - arrow refill pickups.Those have no Y offset but look fine already.
			static const float new_y_offset = 20.0f;
			RPX_ERROR_CHECK(elfUtil::write_float(gRPX, elfUtil::AddressToOffset(gRPX, display_data_addr + 0x10), new_y_offset));
		}
	}

	return TweakError::NONE;
}

TweakError update_shop_item_descriptions(const Location& beedle20, const Location& beedle500, const Location& beedle950, const Location& beedle900) {
	const GameItem beedle20Item = beedle20.currentItem.getGameItemId();
	const GameItem beedle500Item = beedle500.currentItem.getGameItemId();
	const GameItem beedle900Item = beedle900.currentItem.getGameItemId();
	const GameItem beedle950Item = beedle950.currentItem.getGameItemId();

	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));

	msbt.messages_by_label["03906"].text.message = TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle20Item)) + u"  20 Rupees" + TEXT_COLOR_DEFAULT + u'\0';
	msbt.messages_by_label["03909"].text.message = Utility::Str::toUTF16(gameItemToName(beedle20Item)) + u"  20 Rupees\nWill you buy it?\n" + TWO_CHOICES + u"I'll buy it\nNo thanks\0"s;

	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));

	path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message4_msbt.szs@YAZ0@SARC@message4.msbt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::MSBTFile msbt2;
	FILETYPE_ERROR_CHECK(msbt2.loadFromFile(path.string()));

	msbt2.messages_by_label["12106"].text.message = TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle500Item)) + u"  500 Rupees\n" + TEXT_COLOR_DEFAULT + u"This is my last one.";
	msbt2.messages_by_label["12109"].text.message = u"This " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle500Item)) + TEXT_COLOR_DEFAULT + u" is a mere " + TEXT_COLOR_RED + u"500 Rupees" + TEXT_COLOR_DEFAULT + u"!\nBuy it! Buy it! Buy buy buy!\n" + TWO_CHOICES + u"I'll buy it\nNo thanks\0"s;

	msbt2.messages_by_label["12107"].text.message = TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle950Item)) + u"  950 Rupees\n" + TEXT_COLOR_DEFAULT + u"This is my last one of these, too.";
	msbt2.messages_by_label["12110"].text.message = u"This " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle950Item)) + TEXT_COLOR_DEFAULT + u" is only " + TEXT_COLOR_RED + u"950 Rupees" + TEXT_COLOR_DEFAULT + u"!\nBuy it! Buy it! Buy buy buy!\n" + TWO_CHOICES + u"I'll buy it\nNo thanks\0"s;

	msbt2.messages_by_label["12108"].text.message = TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle900Item)) + u"  900 Rupees\n" + TEXT_COLOR_DEFAULT + u"The price may be high, but it'll pay\noff handsomely in the end!";
	msbt2.messages_by_label["12111"].text.message = u"This " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle900Item)) + TEXT_COLOR_DEFAULT + u" is just " + TEXT_COLOR_RED + u"900 Rupees" + TEXT_COLOR_DEFAULT + u"!\nBuy it! Buy it! Buy buy buy!\n" + TWO_CHOICES + u"I'll buy it\nNo thanks\0"s;

	FILETYPE_ERROR_CHECK(msbt2.writeToFile(path.string()));
	
	return TweakError::NONE;
}

TweakError update_auction_item_names(const Location& auction5_, const Location& auction40_, const Location& auction60_, const Location& auction80_, const Location& auction100_) {
	const std::u16string auction5 = Utility::Str::toUTF16(gameItemToName(auction5_.currentItem.getGameItemId()));
	const std::u16string auction40 = Utility::Str::toUTF16(gameItemToName(auction40_.currentItem.getGameItemId()));
	const std::u16string auction60 = Utility::Str::toUTF16(gameItemToName(auction60_.currentItem.getGameItemId()));
	const std::u16string auction80 = Utility::Str::toUTF16(gameItemToName(auction80_.currentItem.getGameItemId()));
	const std::u16string auction100 = Utility::Str::toUTF16(gameItemToName(auction100_.currentItem.getGameItemId()));

	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message3_msbt.szs@YAZ0@SARC@message3.msbt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));

	msbt.messages_by_label["07440"].text.message = TEXT_COLOR_RED + auction40 + TEXT_COLOR_DEFAULT + u'\0';
	msbt.messages_by_label["07441"].text.message = TEXT_COLOR_RED + auction5 + TEXT_COLOR_DEFAULT + u'\0';
	msbt.messages_by_label["07442"].text.message = TEXT_COLOR_RED + auction60 + TEXT_COLOR_DEFAULT + u'\0';
	msbt.messages_by_label["07443"].text.message = TEXT_COLOR_RED + auction80 + TEXT_COLOR_DEFAULT + u'\0';
	msbt.messages_by_label["07444"].text.message = TEXT_COLOR_RED + auction100 + TEXT_COLOR_DEFAULT + u'\0';

	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));

	//also add a hint to the flyer explaining what items the auction holds
	RandoSession::fspath path2 = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");
	EXTRACT_ERR_CHECK(path2);
	
	FileTypes::MSBTFile msbt2;
	FILETYPE_ERROR_CHECK(msbt2.loadFromFile(path2.string()));

	msbt2.messages_by_label["00804"].text.message.pop_back(); //remove null terminator, we want to add things before it

	std::u16string str = u"\n\n\nParticipate for the chance to win ";
	if (auction5.find(u"Treasure Chart") != std::string::npos) {
		str += u"a " + TEXT_COLOR_RED + u"Treasure Chart" + TEXT_COLOR_DEFAULT + u", ";
	}
	else if (auction5.find(u"Triforce Chart") != std::string::npos) {
		str += u"a " + TEXT_COLOR_RED + u"Triforce Chart" + TEXT_COLOR_DEFAULT + u", ";
	}
	else {
		str += get_indefinite_article(auction5) + u' ' + TEXT_COLOR_RED + auction5 + TEXT_COLOR_DEFAULT + u", ";
	}

	if (auction40.find(u"Treasure Chart") != std::string::npos) {
		str += u"a " + TEXT_COLOR_RED + u"Treasure Chart" + TEXT_COLOR_DEFAULT + u", ";
	}
	else if (auction40.find(u"Triforce Chart") != std::string::npos) {
		str += u"a " + TEXT_COLOR_RED + u"Triforce Chart" + TEXT_COLOR_DEFAULT + u", ";
	}
	else {
		str += get_indefinite_article(auction40) + u' ' + TEXT_COLOR_RED + auction40 + TEXT_COLOR_DEFAULT + u", ";
	}

	if (auction60.find(u"Treasure Chart") != std::string::npos) {
		str += u"a " + TEXT_COLOR_RED + u"Treasure Chart" + TEXT_COLOR_DEFAULT + u", ";
	}
	else if (auction60.find(u"Triforce Chart") != std::string::npos) {
		str += u"a " + TEXT_COLOR_RED + u"Triforce Chart" + TEXT_COLOR_DEFAULT + u", ";
	}
	else {
		str += get_indefinite_article(auction60) + u' ' + TEXT_COLOR_RED + auction60 + TEXT_COLOR_DEFAULT + u", ";
	}

	if (auction80.find(u"Treasure Chart") != std::string::npos) {
		str += u"a " + TEXT_COLOR_RED + u"Treasure Chart" + TEXT_COLOR_DEFAULT + u", ";
	}
	else if (auction80.find(u"Triforce Chart") != std::string::npos) {
		str += u"a " + TEXT_COLOR_RED + u"Triforce Chart" + TEXT_COLOR_DEFAULT + u", ";
	}
	else {
		str += get_indefinite_article(auction80) + u' ' + TEXT_COLOR_RED + auction80 + TEXT_COLOR_DEFAULT + u", ";
	}

	if (auction100.find(u"Treasure Chart") != std::string::npos) {
		str += u"or a " + TEXT_COLOR_RED + u"Treasure Chart" + TEXT_COLOR_DEFAULT + u"!";
	}
	else if (auction100.find(u"Triforce Chart") != std::string::npos) {
		str += u"or a " + TEXT_COLOR_RED + u"Triforce Chart" + TEXT_COLOR_DEFAULT + u"!";
	}
	else {
		str += u"or " + get_indefinite_article(auction100) + u' ' + TEXT_COLOR_RED + auction100 + TEXT_COLOR_DEFAULT + u"!";
	}

	msbt2.messages_by_label["00804"].text.message += word_wrap_string(str, 43);
	msbt2.messages_by_label["00804"].text.message += u'\0'; //add null terminator
	FILETYPE_ERROR_CHECK(msbt2.writeToFile(path2.string()));

	return TweakError::NONE;
}

TweakError update_battlesquid_item_names(const Location& firstPrize_, const Location& secondPrize_) {
	const GameItem firstPrize = firstPrize_.currentItem.getGameItemId();
	const GameItem secondPrize = secondPrize_.currentItem.getGameItemId();

	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message3_msbt.szs@YAZ0@SARC@message3.msbt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));

	msbt.messages_by_label["07520"].text.message = SOUND(0x8E) + u"Hoorayyy! Yayyy! Yayyy!\nOh, thank you, Mr. Sailor!\n\n\n" + word_wrap_string(u"Please take this " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(firstPrize)) + TEXT_COLOR_DEFAULT + u" as a sign of our gratitude.You are soooooo GREAT!\0"s, 43);
	msbt.messages_by_label["07521"].text.message = SOUND(0x8E) + u"Hoorayyy! Yayyy! Yayyy!\nOh, thank you so much, Mr. Sailor!\n\n\n" + word_wrap_string(u"This is our thanks to you! It's been passed down on our island for many years, so don't tell the island elder, OK? Here..." + TEXT_COLOR_RED + IMAGE(ImageTags::HEART) + TEXT_COLOR_DEFAULT + u"Please accept this " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(secondPrize)) + TEXT_COLOR_DEFAULT + u"!\0"s, 43);

	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError update_item_names_in_letter_advertising_rock_spire_shop(const Location& beedle500, const Location& beedle950, const Location& beedle900) {
	const std::u16string beedle500Item = Utility::Str::toUTF16(gameItemToName(beedle500.currentItem.getGameItemId()));
	const std::u16string beedle900Item = Utility::Str::toUTF16(gameItemToName(beedle900.currentItem.getGameItemId()));
	const std::u16string beedle950Item = Utility::Str::toUTF16(gameItemToName(beedle950.currentItem.getGameItemId()));

	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));

	const std::u16string stringBefore = msbt.messages_by_label["03325"].text.message.substr(0, 194);
	const std::u16string stringAfter = msbt.messages_by_label["03325"].text.message.substr(396, 323);
	std::u16string hintString = u"Do you have need of " + get_indefinite_article(beedle500Item) + u" " + TEXT_COLOR_RED + beedle500Item + TEXT_COLOR_DEFAULT + u", " + get_indefinite_article(beedle950Item) + u" " + TEXT_COLOR_RED + beedle950Item + TEXT_COLOR_DEFAULT + u", or " + get_indefinite_article(beedle900Item) + u" " + TEXT_COLOR_RED + beedle900Item + TEXT_COLOR_DEFAULT + u"? We have them at special bargain prices.";
	hintString = word_wrap_string(hintString, 39);
	hintString = pad_str_4_lines(hintString);
	std::vector<std::u16string> hintLines = Utility::Str::split(hintString, u'\n');

	msbt.messages_by_label["03325"].text.message = stringBefore;
	for (std::u16string& line : hintLines) {
		if (line != u"") {
			line = u"  " + line; //might be UB?
		}
	}

	hintString = Utility::Str::merge(hintLines, u'\n');
	msbt.messages_by_label["03325"].text.message += hintString;
	msbt.messages_by_label["03325"].text.message += stringAfter;

	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));
	return TweakError::NONE;
}

//TODO: test formatting with progress hints
TweakError update_savage_labyrinth_hint_tablet(const Location& floor30_, const Location& floor50_) {
	const bool floor30Progress = floor30_.currentItem.isMajorItem();
	const bool floor50Progress = floor50_.currentItem.isMajorItem();

	const std::string floor30item = get_hint_item_name(gameItemToName(floor30_.currentItem.getGameItemId()));
	const std::string floor50item = get_hint_item_name(gameItemToName(floor50_.currentItem.getGameItemId()));

	std::u16string hint;
	if(floor30Progress && floor50Progress) {
		const std::u16string floor30Hint = Utility::Str::toUTF16(progress_hints.at(floor30item));
		const std::u16string floor50Hint = Utility::Str::toUTF16(progress_hints.at(floor50item));
		hint = u"the way to " + TEXT_COLOR_RED + floor30Hint + TEXT_COLOR_DEFAULT + u" and " + TEXT_COLOR_RED + floor50Hint + TEXT_COLOR_DEFAULT + u" await.";
	}
	else if(floor30Progress) {
		const std::u16string floor30Hint = Utility::Str::toUTF16(progress_hints.at(floor30item));
		hint = u"the way to " + TEXT_COLOR_RED + floor30Hint + TEXT_COLOR_DEFAULT + u" and a challenge await.";
	}
	else if(floor50Progress) {
		const std::u16string floor50Hint = Utility::Str::toUTF16(progress_hints.at(floor50item));
		hint = u"a challenge and " + TEXT_COLOR_RED + floor50Hint + TEXT_COLOR_DEFAULT + u" await.";
	}
	else {
		hint = u"a challenge awaits.";
	}
	
	const RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));
	msbt.messages_by_label["00837"].text.message = u"\n" + TEXT_SIZE(150) + TEXT_COLOR_RED + u"The Savage Labyrinth" + TEXT_COLOR_DEFAULT + TEXT_SIZE(100) + u"\n\n\n";
	msbt.messages_by_label["00837"].text.message += word_wrap_string(u"Deep in the never-ending darkness, " + hint, 42) + u'\0';
	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));

	return TweakError::NONE;
}

//hints

TweakError shorten_zephos_event() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat");
	EXTRACT_ERR_CHECK(path);

	FileTypes::EventList event_list;
	FILETYPE_ERROR_CHECK(event_list.loadFromFile(path.string()));
	if(event_list.Events_By_Name.count("TACT_HT") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	std::shared_ptr<Event> wind_shrine_event = event_list.Events_By_Name.at("TACT_HT");

	std::shared_ptr<Actor> zephos = wind_shrine_event->get_actor("Hr");
	if (zephos == nullptr) {
		LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	}

	std::shared_ptr<Actor> link = wind_shrine_event->get_actor("Link");
	if (link == nullptr) {
		LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	}

	std::shared_ptr<Actor> camera = wind_shrine_event->get_actor("CAMERA");
	if (camera == nullptr) {
		LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	}

	zephos->actions.erase(zephos->actions.begin() + 7, zephos->actions.end());
	link->actions.erase(link->actions.begin() + 7, link->actions.end());
	camera->actions.erase(camera->actions.begin() + 5, camera->actions.end());
	wind_shrine_event->ending_flags = {
		zephos->actions.back()->flag_id_to_set,
		link->actions.back()->flag_id_to_set,
		camera->actions.back()->flag_id_to_set,
	};

	FILETYPE_ERROR_CHECK(event_list.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError update_korl_dialog() {
	const RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));
	msbt.messages_by_label["03443"].text.message = CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", the sea is all yours.\nMake sure you explore every corner\nin search of items to help you. Remember\nthat your quest is to defeat Ganondorf.\0"s;
	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError set_num_starting_triforce_shards(const uint8_t numShards) {
	if(custom_symbols.count("num_triforce_shards_to_start_with") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
	
	const uint32_t num_shards_address = custom_symbols.at("num_triforce_shards_to_start_with");
	RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, num_shards_address), numShards));
	
	return TweakError::NONE;
}

TweakError set_starting_health(const uint16_t heartPieces, const uint16_t heartContainers) {
	if(custom_symbols.count("starting_quarter_hearts") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
	const uint16_t base_health = 12;
	const uint16_t starting_health = base_health + (heartContainers * 4) + heartPieces;
	const uint32_t starting_quarter_hearts_address = custom_symbols.at("starting_quarter_hearts");

	RPX_ERROR_CHECK(elfUtil::write_u16(gRPX, elfUtil::AddressToOffset(gRPX, starting_quarter_hearts_address), starting_health));
	return TweakError::NONE;
}

TweakError set_starting_magic(const uint8_t& startingMagic) {
	if(custom_symbols.count("starting_magic") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
	const uint32_t starting_magic_address = custom_symbols.at("starting_magic");
	RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, starting_magic_address), startingMagic));
	return TweakError::NONE;
}

TweakError set_damage_multiplier(const float multiplier) {
	if(custom_symbols.count("custom_damage_multiplier") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
	const uint32_t damage_multiplier_address = custom_symbols.at("custom_damage_multiplier");
	RPX_ERROR_CHECK(elfUtil::write_float(gRPX, elfUtil::AddressToOffset(gRPX, damage_multiplier_address), multiplier));
	return TweakError::NONE;
}

TweakError set_pig_color(const PigColor& color) {
	if(custom_symbols.count("outset_pig_color") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
	const uint32_t pig_color_address = custom_symbols.at("outset_pig_color");
	RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, pig_color_address), static_cast<uint8_t>(color)));
	return TweakError::NONE;
}

TweakError add_pirate_ship_to_windfall() {
	RandoSession::fspath windfallPath = g_session.openGameFile("content/Common/Pack/szs_permanent1.pack@SARC@sea_Room11.szs@YAZ0@SARC@Room11.bfres@BFRES@room.dzr");
	RandoSession::fspath shipRoomPath = g_session.openGameFile("content/Common/Stage/Asoko_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(windfallPath);
	EXTRACT_ERR_CHECK(shipRoomPath);

	FileTypes::DZXFile windfallDzr;
	FileTypes::DZXFile shipDzr;

	FILETYPE_ERROR_CHECK(windfallDzr.loadFromFile(windfallPath.string()));
	FILETYPE_ERROR_CHECK(shipDzr.loadFromFile(shipRoomPath.string()));

	std::vector<ChunkEntry*> wf_layer_2_actors = windfallDzr.entries_by_type_and_layer("ACTR", 2);
	std::string layer_2_ship_data; //copy actor data, add_entity reallocates vector and invalidates pointer
	for (ChunkEntry* actor : wf_layer_2_actors) {
		if (std::strncmp(&actor->data[0], "Pirates\x00", 8) == 0) layer_2_ship_data = actor->data;
	}
	if(layer_2_ship_data.empty()) LOG_ERR_AND_RETURN(TweakError::MISSING_ENTITY);

	ChunkEntry& default_layer_ship = windfallDzr.add_entity("ACTR");
	default_layer_ship.data = layer_2_ship_data;
	default_layer_ship.data[0xA] = '\x00';

	FILETYPE_ERROR_CHECK(windfallDzr.writeToFile(windfallPath.string()));

	for (const int layer_num : {2, 3}) {
		std::vector<ChunkEntry*> actors = shipDzr.entries_by_type_and_layer("ACTR", layer_num);
		for (ChunkEntry* actor : actors) {
			if (std::strncmp(&actor->data[0], "P2b\x00\x00\x00\x00\x00", 8) == 0) shipDzr.remove_entity(actor);
		}
	}

	ChunkEntry& aryll = shipDzr.add_entity("ACTR");
	aryll.data = "Ls1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x44\x16\x00\x00\xC4\x09\x80\x00\xC3\x48\x00\x00\x00\x00\xC0\x00\x00\x00\xFF\xFF"s;

	RandoSession::fspath msbtPath = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");
	EXTRACT_ERR_CHECK(msbtPath);

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(msbtPath.string()));

	msbt.messages_by_label["03008"].attributes.soundEffect = 106;
	msbt.messages_by_label["03008"].text.message = u"'Hoy! Big Brother!\n";
	msbt.messages_by_label["03008"].text.message += u"Wanna play a game? It's fun, trust me!";
	msbt.messages_by_label["03008"].text.message = pad_str_4_lines(msbt.messages_by_label["03008"].text.message);
	msbt.messages_by_label["03008"].text.message += word_wrap_string(u"Just " + TEXT_COLOR_RED + u"step on this button" + TEXT_COLOR_DEFAULT + u", and try to swing across the ropes to reach that door over there before time's up!\0"s, 44);

	FILETYPE_ERROR_CHECK(msbt.writeToFile(msbtPath.string()));

	const uint32_t stage_bgm_info_list_start = 0x1018E428;
	const uint32_t second_dynamic_scene_waves_list_start = 0x1018E2EC;
	const uint8_t asoko_spot_id = 0xC;
	const uint8_t new_second_scene_wave_index = 0xE;
	const uint8_t isle_link_0_aw_index = 0x19;

	const uint32_t asoko_bgm_info_ptr = stage_bgm_info_list_start + asoko_spot_id * 0x4;
	const uint32_t new_second_scene_wave_ptr = second_dynamic_scene_waves_list_start + new_second_scene_wave_index * 2;
	RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, asoko_bgm_info_ptr + 3), new_second_scene_wave_index));
	RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, new_second_scene_wave_ptr), isle_link_0_aw_index));

	FILETYPE_ERROR_CHECK(shipDzr.writeToFile(shipRoomPath.string()));
	return TweakError::NONE;
}

TweakError add_cross_dungeon_warps() {
	std::array<std::array<CyclicWarpPotData, 3>, 2> loops = { {
		{ {
			{"M_NewD2", 2, 2185.0f, 0.0f, 590.0f, 0xA000, 2},
			{"kindan", 1, 986.0f, 3956.43f, 9588.0f, 0xB929, 2},
			{"Siren", 6, 277.0f, 229.42f, -6669.0f, 0xC000, 2}
		} },
		{ {
			{"ma2room", 2, 1556.0f, 728.46f, -7091.0f, 0xEAA6, 5},
			{"M_Dai", 1, -8010.0f, 1010.0f, -1610.0f, 0, 5},
			{"kaze", 3, -4333.0f, 1100.0f, 48.0f, 0x4000, 5}
		} }
	} };

	for(auto& loop : loops) {
		uint8_t warp_index = 0;
		for (CyclicWarpPotData& warp : loop) {
			RandoSession::fspath stagePath = g_session.openGameFile("content/Common/Stage/" + warp.stage_name + "_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs");
			RandoSession::fspath roomPath = g_session.openGameFile("content/Common/Stage/" + warp.stage_name + "_Room" + std::to_string(warp.room_num) + ".szs@YAZ0@SARC@Room" + std::to_string(warp.room_num) + ".bfres@BFRES@room.dzr");
			EXTRACT_ERR_CHECK(stagePath);
			EXTRACT_ERR_CHECK(roomPath);

			FileTypes::DZXFile stage;
			FileTypes::DZXFile room;
			FILETYPE_ERROR_CHECK(stage.loadFromFile(stagePath.string()));
			FILETYPE_ERROR_CHECK(room.loadFromFile(roomPath.string()));

			FileTypes::DZXFile* dzx_for_spawn;
			dzx_for_spawn = &room;
			if(warp.stage_name == "M_Dai" || warp.stage_name == "kaze") {
				dzx_for_spawn = &stage;
			}

			Utility::Endian::toPlatform_inplace(eType::Big, warp.x);
			Utility::Endian::toPlatform_inplace(eType::Big, warp.y);
			Utility::Endian::toPlatform_inplace(eType::Big, warp.z);
			Utility::Endian::toPlatform_inplace(eType::Big, warp.y_rot);

			ChunkEntry& spawn = dzx_for_spawn->add_entity("PLYR");
			spawn.data = "Link\x00\x00\x00\x00\xFF\xFF\x70"s;
			spawn.data.resize(0x20);
			spawn.data[0xB] = (spawn.data[0xB] & ~0x3F) | (warp.room_num & 0x3F);
			spawn.data.replace(0xC, 4, reinterpret_cast<const char*>(&warp.x), 4);
			spawn.data.replace(0x10, 4, reinterpret_cast<const char*>(&warp.y), 4);
			spawn.data.replace(0x14, 4, reinterpret_cast<const char*>(&warp.z), 4);
			spawn.data.replace(0x18, 2, "\x00\x00", 2);
			spawn.data.replace(0x1A, 2, reinterpret_cast<const char*>(&warp.y_rot), 2);
			spawn.data.replace(0x1C, 4, "\xFF\x45\xFF\xFF", 4);

			std::vector<ChunkEntry*> spawns = dzx_for_spawn->entries_by_type("PLYR");
			std::vector<ChunkEntry*> spawn_id_69;
			for (ChunkEntry* spawn_to_check : spawns) {
				if (std::strncmp(&spawn_to_check->data[0x1D], "\x45", 1) == 0) spawn_id_69.push_back(spawn_to_check);
			}
			if (spawn_id_69.size() != 1) LOG_ERR_AND_RETURN(TweakError::MISSING_ENTITY); //technically too many and not missing, but its close enough with line number

			std::vector<uint8_t> pot_index_to_exit;
			for (const CyclicWarpPotData& other_warp : loop) {
				ChunkEntry& scls_exit = room.add_entity("SCLS");
				std::string dest_stage = other_warp.stage_name;
				dest_stage.resize(8, '\0');
				scls_exit.data.resize(0xC);
				scls_exit.data.replace(0, 8, dest_stage);
				scls_exit.data.replace(8, 1, "\x45", 1);
				scls_exit.data.replace(0x9, 1, reinterpret_cast<const char*>(&other_warp.room_num), 1);
				scls_exit.data.replace(0xA, 2, "\x04\xFF", 2);
				pot_index_to_exit.push_back(room.entries_by_type("SCLS").size() - 1);
			}

			uint32_t params = 0x00000000;
			params = (params & ~0xFF000000) | ((pot_index_to_exit[2] << 24) & 0xFF000000);
			params = (params & ~0x00FF0000) | ((pot_index_to_exit[1] << 16) & 0x00FF0000);
			params = (params & ~0x0000FF00) | ((pot_index_to_exit[0] << 8) & 0x0000FF00);
			params = (params & ~0x000000F0) | ((warp.event_reg_index << 4) & 0x000000F0);
			params = (params & ~0x0000000F) | ((warp_index + 2) & 0x0000000F);
			Utility::Endian::toPlatform_inplace(eType::Big, params);

			ChunkEntry& warp_pot = room.add_entity("ACTR");
			warp_pot.data = "Warpts" + std::to_string(warp_index + 1);
			warp_pot.data.resize(0x20);
			warp_pot.data.replace(0x8, 4, reinterpret_cast<const char*>(&params), 4);
			warp_pot.data.replace(0xC, 4, reinterpret_cast<const char*>(&warp.x), 4);
			warp_pot.data.replace(0x10, 4, reinterpret_cast<const char*>(&warp.y), 4);
			warp_pot.data.replace(0x14, 4, reinterpret_cast<const char*>(&warp.z), 4);
			warp_pot.data.replace(0x18, 2, "\xFF\xFF", 2);
			warp_pot.data.replace(0x1A, 2, reinterpret_cast<const char*>(&warp.y_rot), 2);
			warp_pot.data.replace(0x1C, 4, "\xFF\xFF\xFF\xFF", 4);

			FILETYPE_ERROR_CHECK(room.writeToFile(roomPath.string()));
			FILETYPE_ERROR_CHECK(stage.writeToFile(stagePath.string()));

			warp_index++;
		}
	}

	FileTypes::JPC drc, totg, ff;
	RandoSession::fspath drcPath = g_session.openGameFile("content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@BFRES@Pscene035.jpc");
	RandoSession::fspath totgPath = g_session.openGameFile("content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@BFRES@Pscene050.jpc");
	RandoSession::fspath ffPath = g_session.openGameFile("content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@BFRES@Pscene043.jpc");
	EXTRACT_ERR_CHECK(drcPath);
	EXTRACT_ERR_CHECK(totgPath);
	EXTRACT_ERR_CHECK(ffPath);

	FILETYPE_ERROR_CHECK(drc.loadFromFile(drcPath.string()));
	FILETYPE_ERROR_CHECK(totg.loadFromFile(totgPath.string()));
	FILETYPE_ERROR_CHECK(ff.loadFromFile(ffPath.string()));

	for (const uint16_t particle_id : {0x8161, 0x8162, 0x8165, 0x8166, 0x8112}) {
		const Particle& particle = drc.particles[drc.particle_index_by_id[particle_id]];

		FILETYPE_ERROR_CHECK(totg.addParticle(particle));
		FILETYPE_ERROR_CHECK(ff.addParticle(particle));

		for (const std::string& textureFilename : particle.texDatabase.value().texFilenames) {
			if (totg.textures.find(textureFilename) == totg.textures.end()) {
				FILETYPE_ERROR_CHECK(totg.addTexture(textureFilename));
			}

			if (ff.textures.find(textureFilename) == ff.textures.end()) {
				FILETYPE_ERROR_CHECK(ff.addTexture(textureFilename));
			}
		}
	}
	
	FILETYPE_ERROR_CHECK(totg.writeToFile(totgPath.string()));
	FILETYPE_ERROR_CHECK(ff.writeToFile(ffPath.string()));

	return TweakError::NONE;
}

TweakError remove_makar_kidnapping() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/kaze_Room3.szs@YAZ0@SARC@Room3.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path);

	FileTypes::DZXFile dzr;
	FILETYPE_ERROR_CHECK(dzr.loadFromFile(path.string()));
	std::vector<ChunkEntry*> actors = dzr.entries_by_type("ACTR");

	ChunkEntry* switch_actor = nullptr; //initialization is just to make compiler happy
	for (ChunkEntry* actor : actors) {
		if (std::strncmp(&actor->data[0], "AND_SW2\x00", 8) == 0) switch_actor = actor;
	}
	if(switch_actor == nullptr) LOG_ERR_AND_RETURN(TweakError::MISSING_ENTITY);
	dzr.remove_entity(switch_actor);

	for (ChunkEntry* actor : actors) {
		if (std::strncmp(&actor->data[0], "wiz_r\x00\x00\x00", 8) == 0) {
			actor->data.replace(0xA, 1, "\xFF", 1);
		}
	}

	FILETYPE_ERROR_CHECK(dzr.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError increase_crawl_speed() {
	//The 3.0 float crawling uses is shared with other things in HD, can't change it directly
	//Redirect both instances to load 6.0 from elsewhere
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0014EC04, 7), 0x000355C4)); //update .rela.text entry
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0014EC4C, 7), 0x000355C4)); //update .rela.text entry
	
	return TweakError::NONE;
}

TweakError add_chart_number_to_item_get_messages() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));
	for (uint8_t item_id = 0xCC; item_id < 0xFF; item_id++) {
		if (item_id == 0xDB || item_id == 0xDC) continue; //skip ghost ship chart and tingle's chart

		const std::u16string itemName = Utility::Str::toUTF16(gameItemToName(idToGameItem(item_id)));
		msbt.messages_by_label["00" + std::to_string(101 + item_id)].text.message.replace(12, 21, TEXT_COLOR_RED + itemName);
	}
	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError increase_grapple_animation_speed() {
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x02170250), 0x394B000A));
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x00075170, 7), 0x00010FFC)); //update .rela.text entry
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x100110C8), 0x41C80000));
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x021711D4), 0x390B0006));
	
	return TweakError::NONE;
}

TweakError increase_block_move_animation() {
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x00153b00, 7), 0x00035AAC)); //update .rela.text entries
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x00153b48, 7), 0x00035AAC));

	uint32_t offset = 0x101CB424;
	for (unsigned int i = 0; i < 13; i++) { //13 types of blocks total
		RPX_ERROR_CHECK(elfUtil::write_u16(gRPX, elfUtil::AddressToOffset(gRPX, offset + 0x04), 0x000C)); // Reduce number frames for pushing to last from 20 to 12
		RPX_ERROR_CHECK(elfUtil::write_u16(gRPX, elfUtil::AddressToOffset(gRPX, offset + 0x0A), 0x000C)); // Reduce number frames for pulling to last from 20 to 12
		offset += 0x9C;
	}

	return TweakError::NONE;
}

TweakError increase_misc_animations() {
	//Float is shared, redirect it to read another float with the right value
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x00148820, 7), 0x000358D8));
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x001482a4, 7), 0x00035124));
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x00148430, 7), 0x000358D8));
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x00148310, 7), 0x00035AAC));
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0014e2d4, 7), 0x00035530));

	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x02508b50), 0x3880000A));
	
	return TweakError::NONE;
}

TweakError set_casual_clothes() {
	if(custom_symbols.count("should_start_with_heros_clothes") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
	const uint32_t starting_clothes_addr = custom_symbols.at("should_start_with_heros_clothes");
	RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, starting_clothes_addr), 0));

	return TweakError::NONE;
}

TweakError hide_ship_sail() {
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x02162B04), 0x4E800020));
	
	return TweakError::NONE;
}

TweakError shorten_auction_intro_event() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/Orichh_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat");
	EXTRACT_ERR_CHECK(path);

	FileTypes::EventList event_list;
	FILETYPE_ERROR_CHECK(event_list.loadFromFile(path.string()));
	if(event_list.Events_By_Name.count("AUCTION_START") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	std::shared_ptr<Event> auction_start_event = event_list.Events_By_Name.at("AUCTION_START");
	std::shared_ptr<Actor> camera = auction_start_event->get_actor("CAMERA");
	if (camera == nullptr) {
		LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	}

	camera->actions.erase(camera->actions.begin() + 3, camera->actions.begin() + 5); //last iterator not inclusive, only erase actions 3-4

	FILETYPE_ERROR_CHECK(event_list.writeToFile(path.string()));
	return TweakError::NONE;
}

TweakError disable_invisible_walls() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/M_NewD2_Room2.szs@YAZ0@SARC@Room2.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path);

	FileTypes::DZXFile dzr;
	FILETYPE_ERROR_CHECK(dzr.loadFromFile(path.string()));
	std::vector<ChunkEntry*> scobs = dzr.entries_by_type("SCOB");

	for (ChunkEntry* scob : scobs) {
		if (std::strncmp(&scob->data[0], "Akabe\x00\x00\x00", 8) == 0) {
			scob->data[0xB] = '\xFF';
		}
	}

	FILETYPE_ERROR_CHECK(dzr.writeToFile(path.string()));
	return TweakError::NONE;
}

TweakError update_skip_rematch_bosses_game_variable(const bool& skipRefights) {
	if(custom_symbols.count("skip_rematch_bosses") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
	const uint32_t skip_rematch_bosses_addr = custom_symbols.at("skip_rematch_bosses");
	if (skipRefights) {
		RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, skip_rematch_bosses_addr), 0x01));
	}
	else {
		RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, skip_rematch_bosses_addr), 0x00));
	}

	return TweakError::NONE;
}

TweakError update_sword_mode_game_variable(const SwordMode swordMode) {
	if(custom_symbols.count("sword_mode") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
	const uint32_t sword_mode_addr = custom_symbols.at("sword_mode");

	if (swordMode == SwordMode::StartWithSword) {
		RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, sword_mode_addr), 0x00));
	}
	else if (swordMode == SwordMode::RandomSword) {
		RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, sword_mode_addr), 0x01));
	}
	else if (swordMode == SwordMode::NoSword) {
		RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, sword_mode_addr), 0x02));
	}

	return TweakError::NONE;
}

TweakError update_starting_gear(const std::vector<GameItem>& startingItems) {
	std::vector<GameItem> startingGear = startingItems; //copy so we can edit without causing problems

	// Changing starting magic doesn't work when done via our normal starting items initialization code, so we need to handle it specially.
  	LOG_AND_RETURN_IF_ERR(set_starting_magic(16 * std::count(startingGear.begin(), startingGear.end(), GameItem::ProgressiveMagicMeter)));
	auto it = std::find(startingGear.begin(), startingGear.end(), GameItem::ProgressiveMagicMeter);
	while (it != startingGear.end()) {
		startingGear.erase(it);
		it = std::find(startingGear.begin(), startingGear.end(), GameItem::ProgressiveMagicMeter);
	}

	if (startingGear.size() > MAXIMUM_ADDITIONAL_STARTING_ITEMS) {
		LOG_ERR_AND_RETURN(TweakError::UNEXPECTED_VALUE); //error
	}

	if(custom_symbols.count("starting_gear") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
	const uint32_t starting_gear_array_addr = custom_symbols.at("starting_gear");
	for (size_t i = 0; i < startingGear.size(); i++) {
		const uint8_t item_id = static_cast<std::underlying_type_t<GameItem>>(startingGear[i]);
		RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, starting_gear_array_addr + i), item_id));
	}

	RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, starting_gear_array_addr + startingGear.size()), 0xFF));

	return TweakError::NONE;
}

TweakError update_swordless_text() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));
	msbt.messages_by_label["01128"].text.message = CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", you may not have the\nMaster Sword, but do not be afraid!\n\n\nThe hammer of the dead is all you\nneed to crush your foe...\n\n\nEven as his ball of fell magic bears down\non you, you can " + TEXT_COLOR_RED + u"knock it back\nwith an empty bottle" + TEXT_COLOR_DEFAULT + u"!\n\n...I am sure you will have a shot at victory!\0"s;
	msbt.messages_by_label["01590"].text.message = CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u"! Do not run! Trust in the\npower of the Skull Hammer!\0"s;
	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError add_hint_signs() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));
	const std::string new_message_label = "00847";
	Attributes attributes;
	attributes.character = 0xF; //sign
	attributes.boxStyle = 0x2;
	attributes.drawType = 0x1;
	attributes.screenPos = 0x2;
	attributes.lineAlignment = 3;
	TSY1Entry tsy;
	tsy.styleIndex = 0x12B;
	const std::u16string message = IMAGE(ImageTags::R_ARROW) + u"\0"s;
	msbt.addMessage(new_message_label, attributes, tsy, message);
	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));

	path = g_session.openGameFile("content/Common/Stage/M_NewD2_Room2.szs@YAZ0@SARC@Room2.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path);

	FileTypes::DZXFile dzr;
	FILETYPE_ERROR_CHECK(dzr.loadFromFile(path.string()));
	std::vector<ChunkEntry*> actors = dzr.entries_by_type("ACTR");

	std::vector<ChunkEntry*> bomb_flowers;
	for (ChunkEntry* actor : actors) {
		if (std::strncmp(&actor->data[0], "BFlower\0", 8) == 0) bomb_flowers.push_back(actor);
	}
	bomb_flowers[1]->data = "Kanban\x00\x00\x00\x00\x03\x4F\x44\x34\x96\xEB\x42\x47\xFF\xFF\xC2\x40\xB0\x3A\x00\x00\x20\x00\x00\x00\xFF\xFF"s;

	FILETYPE_ERROR_CHECK(dzr.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError prevent_door_boulder_softlocks() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/M_NewD2_Room13.szs@YAZ0@SARC@Room13.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path);

	FileTypes::DZXFile room13;
	FILETYPE_ERROR_CHECK(room13.loadFromFile(path.string()));

	ChunkEntry& swc00_13 = room13.add_entity("SCOB");
	swc00_13.data = "SW_C00\x00\x00\x00\x03\xFF\x05\x45\x24\xB0\x00\x00\x00\x00\x00\x43\x63\x00\x00\x00\x00\xC0\x00\xFF\xFF\xFF\xFF\x20\x10\x10\xFF"s;
	FILETYPE_ERROR_CHECK(room13.writeToFile(path.string()));

	path = g_session.openGameFile("content/Common/Stage/M_NewD2_Room14.szs@YAZ0@SARC@Room14.bfres@BFRES@room.dzr");

	FileTypes::DZXFile room14;
	FILETYPE_ERROR_CHECK(room14.loadFromFile(path.string()));

	ChunkEntry& swc00_14 = room14.add_entity("SCOB");
	swc00_14.data = "SW_C00\x00\x00\x00\x03\xFF\x06\xC5\x7A\x20\x00\x44\xF3\xC0\x00\xC5\x06\xC0\x00\x00\x00\xA0\x00\xFF\xFF\xFF\xFF\x20\x10\x10\xFF"s;
	FILETYPE_ERROR_CHECK(room14.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError update_tingle_statue_item_get_funcs() {
	const uint32_t item_get_func_ptr = 0x0001DA54; //First relevant relocation entry in .rela.data (overwrites .data section when loaded)
	const std::unordered_map<int, std::string> symbol_name_by_item_id = { {0xA3, "dragon_tingle_statue_item_get_func"}, {0xA4, "forbidden_tingle_statue_item_get_func"}, {0xA5, "goddess_tingle_statue_item_get_func"}, {0xA6, "earth_tingle_statue_item_get_func"}, {0xA7, "wind_tingle_statue_item_get_func"} };

	for (const uint8_t statue_id : {0xA3, 0xA4, 0xA5, 0xA6, 0xA7}) {
		if(custom_symbols.count(symbol_name_by_item_id.at(statue_id)) == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);

		const uint32_t item_func_addr = item_get_func_ptr + (statue_id * 0xC) + 8;
		const uint32_t item_func_ptr = custom_symbols.at(symbol_name_by_item_id.at(statue_id));
		RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_func_addr, 9), item_func_ptr - 0x02000000));
	}

	return TweakError::NONE;
}

TweakError make_tingle_statue_reward_rupee_rainbow_colored() {
	const uint32_t item_resources_list_start = 0x101e4674;
	const uint32_t rainbow_rupee_item_resource_addr = item_resources_list_start + 0xB8 * 0x24;

	RPX_ERROR_CHECK(elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, rainbow_rupee_item_resource_addr + 0x14), 0x07));

	return TweakError::NONE;
}

TweakError show_seed_hash_on_title_screen(const std::u16string& hash) { //make sure hash is null terminated
	using namespace NintendoWare::Layout;

	RandoSession::fspath path = g_session.openGameFile("content/Common/Layout/Title_00.szs@YAZ0@SARC@blyt/Title_00.bflyt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::FLYTFile layout;
	FILETYPE_ERROR_CHECK(layout.loadFromFile(path.string()));

	//add hash
	Pane& newPane = layout.rootPane.children[0].children[1].children[3].duplicateChildPane(1); //hidden version number text
	txt1& textPane = *dynamic_cast<txt1*>(newPane.pane.get());
	textPane.name = "T_Hash";
	textPane.name.resize(0x18);
	textPane.text = u"Seed Hash:\n" + hash;
	textPane.fontIndex = 0;
	textPane.restrictedLen = (12 + hash.length()) * 2;
	textPane.lineAlignment = txt1::LineAlignment::CENTER;
	textPane.translation.X = -491.0f;
	textPane.translation.Y = -113.0f;
	textPane.width = 205.0f;
	textPane.height = 100.0f;

	FILETYPE_ERROR_CHECK(layout.writeToFile(path.string()));
	return TweakError::NONE;
}

TweakError implement_key_bag() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));
	msbt.messages_by_label["00403"].text.message = u"Key Bag\0"s;
	msbt.messages_by_label["00603"].text.message = u"A handy bag for holding your keys!\n"s;
  	msbt.messages_by_label["00603"].text.message += u"Here's how many you've got with you:\n";
  	msbt.messages_by_label["00603"].text.message += u"DRC: \x000E\x0007\x004B\x0000         "s;
  	msbt.messages_by_label["00603"].text.message += u"FW: \x000E\x0007\x004C\x0000         "s;
  	msbt.messages_by_label["00603"].text.message += u"TotG: \x000E\x0007\x004D\x0000     \n"s;
  	msbt.messages_by_label["00603"].text.message += u"ET: \x000E\x0007\x004E\x0000           "s;
  	msbt.messages_by_label["00603"].text.message += u"WT: \x000E\x0007\x004F\x0000     \0"s;
	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));
	
	path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@BtnCollectIcon_00.szs@YAZ0@SARC@timg/CollectIcon118_08^l.bflim");
	EXTRACT_ERR_CHECK(path);
	FileTypes::FLIMFile pirates_charm;
	FILETYPE_ERROR_CHECK(pirates_charm.loadFromFile(path.string()));
	FILETYPE_ERROR_CHECK(pirates_charm.replaceWithDDS("./assets/KeyBag.dds", GX2TileMode::GX2_TILE_MODE_DEFAULT, 0, true));
	FILETYPE_ERROR_CHECK(pirates_charm.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError show_dungeon_markers_on_chart(World& world) {
	using namespace NintendoWare::Layout;

	static std::vector<size_t> quest_marker_indexes = {
		144, 145, 146, 147, 148, 149, 150, 151
	};

	std::unordered_set<uint8_t> room_indexes;
	for(const auto& [name, dungeon] : world.dungeons) {
    if (dungeon.isRaceModeDungeon)
    {
        const std::string& islandName = dungeon.island;
        room_indexes.emplace(islandNameToRoomIndex(islandName));
    }
	}

	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@Map_00.szs@YAZ0@SARC@blyt/Map_00.bflyt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::FLYTFile map;
	FILETYPE_ERROR_CHECK(map.loadFromFile(path.string()));
	
	for(const uint8_t& index : room_indexes) {
		const uint32_t column = (index - 1) % 7;
		const uint32_t row = std::floor((index - 1) / 7);
		const float x_pos = (column * 73.0f) - 148.0f;
		const float y_pos = 175.0f - (row * 73.0f);

		pan1* marker = dynamic_cast<pan1*>(map.rootPane.children[0].children[quest_marker_indexes.back()].pane.get());
		quest_marker_indexes.pop_back();
		marker->translation.X = x_pos;
		marker->translation.Y = y_pos;
	}

	for(const auto& index : quest_marker_indexes) { //hide any remaining markers
		pan1* marker = dynamic_cast<pan1*>(map.rootPane.children[0].children[index].pane.get());
		marker->alpha = 0;
	}

	FILETYPE_ERROR_CHECK(map.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError add_chest_in_place_jabun_cutscene() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/Pjavdou_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path);

	FileTypes::DZXFile dzr;
	FILETYPE_ERROR_CHECK(dzr.loadFromFile(path.string()));
	ChunkEntry& raft = dzr.add_entity("ACTR");
	ChunkEntry& chest = dzr.add_entity("TRES");
	raft.data = "Ikada\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\xFF\xFF"s;
	chest.data = "takara3\x00\xFF\x2F\xF3\x05\x00\x00\x00\x00\x43\x96\x00\x00\xC3\x48\x00\x00\x00\x00\x80\x00\x05\xFF\xFF\xFF"s;
	FILETYPE_ERROR_CHECK(dzr.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError add_jabun_obstacles_to_default_layer() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/szs_permanent2.pack@SARC@sea_Room44.szs@YAZ0@SARC@Room44.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path);

	FileTypes::DZXFile dzr;
	FILETYPE_ERROR_CHECK(dzr.loadFromFile(path.string()));

	std::vector<ChunkEntry*> layer_5_actors = dzr.entries_by_type_and_layer("ACTR", 5);
	const std::string layer_5_door_data = layer_5_actors[0]->data;
	const std::string layer_5_whirlpool_data = layer_5_actors[1]->data;

	dzr.remove_entity(layer_5_actors[0]);
	dzr.remove_entity(layer_5_actors[1]);

	ChunkEntry& newDoor = dzr.add_entity("ACTR");
	ChunkEntry& newWhirlpool = dzr.add_entity("ACTR");
	newDoor.data = layer_5_door_data;
	newWhirlpool.data = layer_5_whirlpool_data;

	FILETYPE_ERROR_CHECK(dzr.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError remove_jabun_stone_door_event() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat");
	EXTRACT_ERR_CHECK(path);

	FileTypes::EventList event_list;
	FILETYPE_ERROR_CHECK(event_list.loadFromFile(path.string()));
	if(event_list.Events_By_Name.count("ajav_uzu") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	std::shared_ptr<Event> unlock_cave_event = event_list.Events_By_Name.at("ajav_uzu");
	std::shared_ptr<Actor> director = unlock_cave_event->get_actor("DIRECTOR");
	if (director == nullptr) {
		LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	}
	std::shared_ptr<Actor> camera = unlock_cave_event->get_actor("CAMERA");
	if (camera == nullptr) {
		LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	}
	std::shared_ptr<Actor> ship = unlock_cave_event->get_actor("Ship");
	if (ship == nullptr) {
		LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	}

	director->actions.erase(director->actions.begin() + 1, director->actions.end());
	camera->actions.erase(camera->actions.begin() + 2, camera->actions.end());
	ship->actions.erase(ship->actions.begin() + 2, ship->actions.end());
	unlock_cave_event->ending_flags = {
		director->actions.back()->flag_id_to_set,
		camera->actions.back()->flag_id_to_set,
		-1
	};

	FILETYPE_ERROR_CHECK(event_list.writeToFile(path.string()));
	return TweakError::NONE;
}

TweakError add_chest_in_place_master_sword() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/kenroom_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path);

	FileTypes::DZXFile dzr;
	FILETYPE_ERROR_CHECK(dzr.loadFromFile(path.string()));

	const std::vector<ChunkEntry*> default_layer_actors = dzr.entries_by_type_and_layer("ACTR", DEFAULT_LAYER);
	dzr.remove_entity(default_layer_actors[5]);
	dzr.remove_entity(default_layer_actors[6]);

	const std::vector<ChunkEntry*> layer_5_actors = dzr.entries_by_type_and_layer("ACTR", 5);
	const std::array<ChunkEntry*, 4> layer_5_to_copy = { layer_5_actors[0], layer_5_actors[2], layer_5_actors[3], layer_5_actors[4] };

	for (ChunkEntry* orig_actor : layer_5_to_copy) {
		ChunkEntry& new_actor = dzr.add_entity("ACTR");
		new_actor.data = orig_actor->data;
		dzr.remove_entity(orig_actor);
	}

	ChunkEntry& chest = dzr.add_entity("TRES");
	chest.data = "takara3\x00\xFF\x20\x50\x04\xc2\xf6\xfd\x71\xc5\x49\x40\x00\xc5\xf4\xe9\x0a\x00\x00\x00\x00\x6a\xff\xff\xff"s;

	const std::vector<ChunkEntry*> spawns = dzr.entries_by_type("PLYR");
	for (ChunkEntry* spawn : spawns) {
		if (spawn->data[29] == '\x0A') {
			spawn->data.replace(0x14, 4, "\xC5\x84\x85\x9A", 4);
			spawn->data.replace(0x10, 4, "\xC5\x38\x56\x3D", 4);
			break;
		}
	}

	FILETYPE_ERROR_CHECK(dzr.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError update_spoil_sell_text() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));
	std::vector<std::u16string> lines = Utility::Str::split(msbt.messages_by_label["03957"].text.message, u'\n');
	if (lines.size() != 5) LOG_ERR_AND_RETURN(TweakError::UNEXPECTED_VALUE); //incorrect number of lines
	lines[2] = u"And no Blue Chu Jelly, either!";
	msbt.messages_by_label["03957"].text.message = Utility::Str::merge(lines, u'\n');
	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError fix_totg_warp_spawn() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/sea_Room26.szs@YAZ0@SARC@Room26.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path);

	FileTypes::DZXFile dzr;
	FILETYPE_ERROR_CHECK(dzr.loadFromFile(path.string()));

	const std::vector<ChunkEntry*> spawns = dzr.entries_by_type("PLYR");
	ChunkEntry* spawn = spawns[9];
	spawn->data = "\x4C\x69\x6E\x6B\x00\x00\x00\x00\x32\xFF\x20\x1A\x47\xC3\x4F\x5F\x00\x00\x00\x00\xBF\xBE\xBF\x90\x00\x00\x00\x00\x01\x01\xFF\xFF"s;

	FILETYPE_ERROR_CHECK(dzr.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError remove_phantom_ganon_req_for_reefs() {
	std::string path;
	for (const uint8_t room_index : {24, 46, 22, 8, 37, 25}) {
		path = "content/Common/Stage/sea_Room" + std::to_string(room_index) + ".szs@YAZ0@SARC@Room" + std::to_string(room_index) + ".bfres@BFRES@room.dzr";
		RandoSession::fspath filePath = g_session.openGameFile(path);
		EXTRACT_ERR_CHECK(filePath);

		FileTypes::DZXFile room_dzr;
		FILETYPE_ERROR_CHECK(room_dzr.loadFromFile(filePath.string()));
		const std::vector<ChunkEntry*> actors = room_dzr.entries_by_type("ACTR");
		for (ChunkEntry* actor : actors) {
			if (std::strncmp(&actor->data[0], "Ocanon\x00\x00", 8) == 0) {
				if (std::strncmp(&actor->data[0xA], "\x2A", 1) == 0) {
					actor->data[0xA] = '\xFF';
				}
			}
			else if (std::strncmp(&actor->data[0], "Oship\x00\x00\x00", 8) == 0) {
				if (std::strncmp(&actor->data[0x19], "\x2A", 1) == 0) {
					actor->data[0x19] = '\xFF';
				}
			}
		}
		FILETYPE_ERROR_CHECK(room_dzr.writeToFile(filePath.string()));
	}

	return TweakError::NONE;
}

TweakError fix_ff_door() {
	const int32_t face_index = 0x1493;
	const uint16_t new_prop_index = 0x0011;

	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/szs_permanent1.pack@SARC@sea_Room1.szs@YAZ0@SARC@Room1.bfres@BFRES@room.dzb");
	EXTRACT_ERR_CHECK(path);
	std::fstream fptr(path, std::ios::binary);

	fptr.seekg(0xC, std::ios::beg);
	uint32_t face_list_offset;
	fptr.read(reinterpret_cast<char*>(&face_list_offset), 4);
	Utility::Endian::toPlatform_inplace(eType::Big, face_list_offset);

	fptr.seekp((face_list_offset + face_index * 0xA) + 6, std::ios::beg);
	fptr.write(reinterpret_cast<const char*>(&new_prop_index), 2);

	return TweakError::NONE;
}

//add bog warp

//rat hole culling

//not needed until enemy rando is a thing
/*TweakError add_failsafe_id_0_spawns() {
	const std::array<spawn_data_to_copy, 32> spawns_to_copy{
	{
		{"Asoko", 0, 255},
		{"I_TestM", 0, 1},
		{"M_Dai", 20, 23},
		{"M_NewD2", 1, 20},
		{"M_NewD2", 2, 1},
		{"M_NewD2", 3, 6},
		{"M_NewD2", 4, 7},
		{"M_NewD2", 6, 9},
		{"M_NewD2", 8, 14},
		{"M_NewD2", 11, 2},
		{"M_NewD2", 12, 3},
		{"M_NewD2", 13, 4},
		{"M_NewD2", 14, 5},
		{"M_NewD2", 15, 18},
		{"TF_06", 1, 1},
		{"TF_06", 2, 2},
		{"TF_06", 3, 2},
		{"TF_06", 4, 2},
		{"TF_06", 5, 2},
		{"TF_06", 6, 6},
		{"ma2room", 1, 2},
		{"ma2room", 2, 15}, // Front door
		{"ma2room", 3, 9}, // In the water
		{"ma2room", 4, 6},
		{"ma3room", 1, 2},
		{"ma3room", 2, 15}, // Front door
		{"ma3room", 3, 9}, // In the water
		{"ma3room", 4, 6},
		{"majroom", 1, 2},
		{"majroom", 2, 15}, // Front door
		{"majroom", 3, 9}, // In the water
		{"majroom", 4, 6}
	}
	};

	const std::array<spawn_data, 32> spawns_to_create{
	{
		{"TF_01", 1},
		{"TF_01", 2},
		{"TF_01", 3},
		{"TF_01", 4},
		{"TF_01", 5},
		{"TF_01", 6},
		{"TF_02", 1},
		{"TF_02", 2},
		{"TF_02", 3},
		{"TF_02", 4},
		{"TF_02", 5},
		{"TF_02", 6}
	}
	};



	for (const spawn_data_to_copy& spawn_info : spawns_to_copy) {
		std::string path = "content/Common/Stage/" + spawn_info.stage_name + "_Room" + std::to_string(spawn_info.room_num) + ".szs@YAZ0@SARC@Room" + std::to_string(spawn_info.room_num) + ".bfres@BFRES@room.dzr";
		RandoSession::fspath filePath = g_session.openGameFile(path);
		FileTypes::DZXFile room_dzr;
		room_dzr.loadFromFile(filePath.string());

		std::vector<ChunkEntry*> spawns = room_dzr.entries_by_type("PLYR");

		for (ChunkEntry* spawn : spawns) {
			if (spawn->data[0x1D] == (char)spawn_info.spawn_id_to_copy) {
				ChunkEntry& new_spawn = room_dzr.add_entity("PLYR");
				new_spawn.data = spawn->data;
				new_spawn.data[0x1D] = '\x00';
			}
		}

		room_dzr.writeToFile(filePath.string());
	}

	for (const spawn_data& spawn_info : spawns_to_create) {
		std::string path = "content/Common/Stage/" + spawn_info.stage_name + "_Room" + std::to_string(spawn_info.room_num) + ".szs@YAZ0@SARC@Room" + std::to_string(spawn_info.room_num) + ".bfres@BFRES@room.dzr";
		RandoSession::fspath filePath = g_session.openGameFile(path);
		FileTypes::DZXFile room_dzr;
		room_dzr.loadFromFile(filePath.string());

		std::vector<ChunkEntry*> doors = room_dzr.entries_by_type("TGDR");

		float spawn_dist_from_door = 200.0f;
		float x_pos = 0.0f;
		float y_pos = 0.0f;
		float z_pos = 0.0f;
		uint16_t y_rot = 0;
		for (const ChunkEntry* door : doors) {
			if (((*(uint16_t*)&door->data[0x18]) & 0x0FC0) == spawn_info.room_num || ((*(uint16_t*)&door->data[0x18]) & 0x003F) == spawn_info.room_num) {
				y_rot = *(uint16_t*)&door->data[0x1A];
				if (((*(uint16_t*)&door->data[0x18]) & 0x003F) != spawn_info.room_num) {
					y_rot = (y_rot + 0x8000) % 0x10000;
				}
				
				int y_rot_degrees = y_rot * (90.0 / 0x4000);
				float x_offset = sin((y_rot_degrees * M_PI) / 180.0) * spawn_dist_from_door;
				float z_offset = cos((y_rot_degrees * M_PI) / 180.0) * spawn_dist_from_door;

				float door_x_pos = Utility::Endian::toPlatform(eType::Big, *(float*)&door->data[0xC]);
				float door_y_pos = Utility::Endian::toPlatform(eType::Big, *(float*)&door->data[0x10]);
				float door_z_pos = Utility::Endian::toPlatform(eType::Big, *(float*)&door->data[0x14]);
				x_pos = door_x_pos + x_offset;
				y_pos = door_y_pos;
				z_pos = door_z_pos + z_offset;
				break;
			}
		}

		ChunkEntry& newSpawn = room_dzr.add_entity("PLYR");
		newSpawn.data = "Link\x00\x00\x00\x00"s;
		newSpawn.data.resize(0x20);

		uint32_t params = 0xFFFFFFFF;
		//https://github.com/LagoLunatic/wwrando/blob/master/tweaks.py#L2160
	}
}*/

TweakError remove_minor_pan_cs() {
	const std::array<pan_cs_info, 7> panning_cs{
		{
			{"M_NewD2", "Room2", 4},
			{"kindan", "Stage", 2},
			{"Siren", "Room18", 2},
			{"M_Dai", "Room3", 7},
			{"sea", "Room41", 19},
			{"sea", "Room41", 22},
			{"sea", "Room41", 23}
		}
	};

	std::string path;
	for (const pan_cs_info& cs_info : panning_cs) {
		if (cs_info.stage_name == "sea") {
			path = "content/Common/Pack/szs_permanent2.pack@SARC@" + cs_info.stage_name + "_" + cs_info.szs_suffix + ".szs@YAZ0@SARC" + "@" + cs_info.szs_suffix + ".bfres@BFRES@room.dzr"; //hardcoding permanent2 because that's all this patch needs and coding more would be annoying
		}
		else {
			path = "content/Common/Stage/" + cs_info.stage_name + "_" + cs_info.szs_suffix + ".szs@YAZ0@SARC";
			if (cs_info.szs_suffix == "Stage") {
				path = path + "@Stage.bfres@BFRES@stage.dzs";
			}
			else {
				path = path + "@" + cs_info.szs_suffix + ".bfres@BFRES@room.dzr";
			}
		}

		RandoSession::fspath filePath = g_session.openGameFile(path);
		EXTRACT_ERR_CHECK(filePath);

		FileTypes::DZXFile dzx;
		FILETYPE_ERROR_CHECK(dzx.loadFromFile(filePath.string()));
		std::vector<ChunkEntry*> scobs = dzx.entries_by_type("SCOB");
		for (ChunkEntry* scob : scobs) {
			if (std::strncmp(&scob->data[0], "TagEv\x00\x00\x00", 8) == 0) {
				if (scob->data[0x8] == cs_info.evnt_index) {
					dzx.remove_entity(scob);
				}
			}
		}

		std::vector<ChunkEntry*> spawns = dzx.entries_by_type("PLYR");
		for (ChunkEntry* spawn : spawns) {
			if (spawn->data[8] == cs_info.evnt_index) {
				spawn->data[8] = '\xFF';
			}
		}

		FILETYPE_ERROR_CHECK(dzx.writeToFile(filePath.string()));
	}

	return TweakError::NONE;
}

//custom actors?

TweakError fix_stone_head_bugs() {
	uint32_t status_bits = elfUtil::read_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x101ca100));
	Utility::Endian::toPlatform_inplace(eType::Big, status_bits);
	status_bits &= ~0x00000080;
	Utility::Endian::toPlatform_inplace(eType::Big, status_bits);
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x101ca100), status_bits));

	return TweakError::NONE;
}

TweakError show_tingle_statues_on_quest_screen() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@BtnMapIcon_00.szs@YAZ0@SARC@timg/MapBtn_00^l.bflim");
	EXTRACT_ERR_CHECK(path);

	FileTypes::FLIMFile tingle;
	FILETYPE_ERROR_CHECK(tingle.loadFromFile(path.string()));
	FILETYPE_ERROR_CHECK(tingle.replaceWithDDS("./assets/Tingle.dds", GX2TileMode::GX2_TILE_MODE_DEFAULT, 0, true));
	FILETYPE_ERROR_CHECK(tingle.writeToFile(path.string()));
	
	path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@BtnMapIcon_00.szs@YAZ0@SARC@timg/MapBtn_07^t.bflim");
	EXTRACT_ERR_CHECK(path);

	FileTypes::FLIMFile shadow;
	FILETYPE_ERROR_CHECK(shadow.loadFromFile(path.string()));
	FILETYPE_ERROR_CHECK(shadow.replaceWithDDS("./assets/TingleShadow.dds", GX2TileMode::GX2_TILE_MODE_DEFAULT, 0, false));
	FILETYPE_ERROR_CHECK(shadow.writeToFile(path.string()));

	path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");
	EXTRACT_ERR_CHECK(path);

	FileTypes::MSBTFile msbt;
	FILETYPE_ERROR_CHECK(msbt.loadFromFile(path.string()));
	msbt.messages_by_label["00503"].text.message = u"Tingle Statues\0"s;
	msbt.messages_by_label["00703"].text.message = u"Golden statues of a mysterious dashing\n figure. They can be traded to " + TEXT_COLOR_RED + u"Ankle" + TEXT_COLOR_DEFAULT + u" on\n" + TEXT_COLOR_RED + u"Tingle Island" + TEXT_COLOR_DEFAULT + u" for a reward!\0"s;
	FILETYPE_ERROR_CHECK(msbt.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError add_shortcut_warps_into_dungeons() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/szs_permanent2.pack@SARC@sea_Room41.szs@YAZ0@SARC@Room41.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path);

	FileTypes::DZXFile dzr;
	FILETYPE_ERROR_CHECK(dzr.loadFromFile(path.string()));
	ChunkEntry& sw_c00 = dzr.add_entity("SCOB");
	sw_c00.data = "SW_C00\x00\x00\x00\x03\xFF\x7F\x48\x40\x24\xED\x45\x44\x99\xB1\x48\x41\x7B\x63\x00\x00\x00\x00\x00\x00\xFF\xFF\x96\x14\x28\xFF"s;

	ChunkEntry& warp = dzr.add_entity("SCOB");
	warp.data = "Ysdls00\x00\x10\xFF\x06\x7F\x48\x54\x16\x86\x42\x0B\xFF\xF8\x48\x3E\xD3\xED\x00\x00\x00\x00\x00\x00\xFF\xFF\x0A\x0A\x0A\xFF"s;

	FILETYPE_ERROR_CHECK(dzr.writeToFile(path.string()));
	return TweakError::NONE;
}

TweakError update_entrance_events() {
	//Some entrances have event triggers with hardcoded stages rather than a load zone with a SCLS entry
	//Update those edge cases based on the SCLS entries in their respective dzr

	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/szs_permanent2.pack@SARC@sea_Room41.szs@YAZ0@SARC@Room41.bfres@BFRES@room.dzr");
	EXTRACT_ERR_CHECK(path);
	
	FileTypes::DZXFile dzr;
	FILETYPE_ERROR_CHECK(dzr.loadFromFile(path.string()));

	path = g_session.openGameFile("content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat");
	EXTRACT_ERR_CHECK(path);
	
	FileTypes::EventList event_list;
	FILETYPE_ERROR_CHECK(event_list.loadFromFile(path.string()));

	const ChunkEntry* waterfall = dzr.entries_by_type("SCLS")[7];

	if(event_list.Events_By_Name.count("fall") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	std::shared_ptr<Action> loadRoom = event_list.Events_By_Name.at("fall")->get_actor("DIRECTOR")->actions[1];
	if(loadRoom == nullptr) LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	loadRoom->properties[0]->value = waterfall->data.substr(0, 8).c_str();
	std::get<std::string>(loadRoom->properties[0]->value) += '\0';
	std::get<std::vector<int32_t>>(loadRoom->properties[1]->value)[0] = waterfall->data[9];

	//This entrance isn't randomized yet, code can be used to patch it if it is ever shuffled
	//ChunkEntry* gallery = dzr.entries_by_type("SCLS")[8];
	//std::shared_ptr<Action> next2 = event_list.Events_By_Name.at("nitendo")->get_actor("DIRECTOR")->actions[1];
	//next2->properties[0]->value = gallery->data.c_str();
	//std::get<std::string>(next2->properties[0]->value) += '\0';
	//std::get<std::vector<int32_t>>(next2->properties[1]->value)[0] = gallery->data[9];

	FILETYPE_ERROR_CHECK(event_list.writeToFile(path.string()));

	//Also change the Forbidden Woods warp to put you on the sea instead of inside Forest Haven
	//This avoids a case where you can only access the lower entrances of FH, but they both dead end, and you need to savewarp to escape
	path = g_session.openGameFile("content/Common/Stage/kinBOSS_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat");
	EXTRACT_ERR_CHECK(path);
	
	FileTypes::EventList event_list_2;
	FILETYPE_ERROR_CHECK(event_list_2.loadFromFile(path.string()));
	
	if(event_list_2.Events_By_Name.count("WARP_WIND") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	std::shared_ptr<Action> exit = event_list_2.Events_By_Name.at("WARP_WIND")->get_actor("DIRECTOR")->actions[2];
	std::get<std::vector<int32_t>>(exit->properties[0]->value)[0] = 0;
	exit->properties[1]->value = "sea\0"s;
	std::get<std::vector<int32_t>>(exit->properties[2]->value)[0] = 41;

	if(event_list_2.Events_By_Name.count("WARP_WIND_AFTER") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_EVENT);
	std::shared_ptr<Action> exit2 = event_list_2.Events_By_Name.at("WARP_WIND_AFTER")->get_actor("DIRECTOR")->actions[2];
	std::get<std::vector<int32_t>>(exit2->properties[0]->value)[0] = 0;
	exit2->properties[1]->value = "sea\0"s;
	std::get<std::vector<int32_t>>(exit2->properties[2]->value)[0] = 41;
	
	FILETYPE_ERROR_CHECK(event_list_2.writeToFile(path.string()));

	return TweakError::NONE;
}

TweakError replace_ctmc_chest_texture() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_3d.pack@SARC@Dalways.szs@YAZ0@SARC@Dalways.bfres");
	EXTRACT_ERR_CHECK(path);

	FileTypes::resFile bfres;
	FILETYPE_ERROR_CHECK(bfres.loadFromFile(path.string()));
	FILETYPE_ERROR_CHECK(bfres.textures[3].replaceImageData("./assets/KeyChest.dds", GX2TileMode::GX2_TILE_MODE_DEFAULT, 0, true, true));
	FILETYPE_ERROR_CHECK(bfres.writeToFile(path.string()));

	return TweakError::NONE;
}



TweakError updateCodeSize() {
	//Increase the max codesize in cos.xml to load all our code
	tinyxml2::XMLDocument cos;
	const RandoSession::fspath cosPath = g_session.openGameFile("code/cos.xml").string();
	EXTRACT_ERR_CHECK(cosPath);
	
	cos.LoadFile(cosPath.string().c_str());
	tinyxml2::XMLElement* root = cos.RootElement();
	root->FirstChildElement("max_codesize")->SetText("03000000");
	cos.SaveFile(cosPath.string().c_str());

	//Also update the RPL info section of the RPX 
	//Change the textSize and loadSize to be large enough for the new code/relocations
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x00000004, 32), 0x00908B80));
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0000001C, 32), 0x00379000));
	
	return TweakError::NONE;
}

TweakError apply_necessary_tweaks(const Settings& settings) {
	LOG_AND_RETURN_IF_ERR(Load_Custom_Symbols("./asm/custom_symbols.json"));

	RandoSession::fspath rpxPath = g_session.openGameFile("code/cking.rpx@RPX");
	EXTRACT_ERR_CHECK(rpxPath);
	FILETYPE_ERROR_CHECK(gRPX.loadFromFile(rpxPath.string()));

	const std::string seedHash = LogInfo::getSeedHash();
	const std::u16string u16_seedHash = Utility::Str::toUTF16(seedHash);
	
	TWEAK_ERR_CHECK(updateCodeSize());

	LOG_AND_RETURN_IF_ERR(Apply_Patch("./asm/patch_diffs/custom_funcs_diff.json"));
	LOG_AND_RETURN_IF_ERR(Apply_Patch("./asm/patch_diffs/make_game_nonlinear_diff.json"));
	LOG_AND_RETURN_IF_ERR(Apply_Patch("./asm/patch_diffs/remove_cutscenes_diff.json"));
	LOG_AND_RETURN_IF_ERR(Apply_Patch("./asm/patch_diffs/flexible_item_locations_diff.json"));
	LOG_AND_RETURN_IF_ERR(Apply_Patch("./asm/patch_diffs/fix_vanilla_bugs_diff.json"));
	LOG_AND_RETURN_IF_ERR(Apply_Patch("./asm/patch_diffs/misc_rando_features_diff.json"));

	LOG_AND_RETURN_IF_ERR(Add_Relocations("./asm/patch_diffs/custom_funcs_reloc.json"));
	LOG_AND_RETURN_IF_ERR(Add_Relocations("./asm/patch_diffs/make_game_nonlinear_reloc.json"));
	LOG_AND_RETURN_IF_ERR(Add_Relocations("./asm/patch_diffs/remove_cutscenes_reloc.json"));
	LOG_AND_RETURN_IF_ERR(Add_Relocations("./asm/patch_diffs/flexible_item_locations_reloc.json"));
	LOG_AND_RETURN_IF_ERR(Add_Relocations("./asm/patch_diffs/fix_vanilla_bugs_reloc.json"));
	LOG_AND_RETURN_IF_ERR(Add_Relocations("./asm/patch_diffs/misc_rando_features_reloc.json"));

	RPX_ERROR_CHECK(elfUtil::removeRelocation(gRPX, {7, 0x001c0ae8})); //would mess with save init
	RPX_ERROR_CHECK(elfUtil::removeRelocation(gRPX, {7, 0x00160224})); //would mess with salvage point patch
	RPX_ERROR_CHECK(elfUtil::removeRelocation(gRPX, {7, 0x00199854})); //would overwrite getLayerNo patch

	//Update hurricane spin item func, not done through asm because of relocation things
	if(custom_symbols.count("hurricane_spin_item_func") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
	RPX_ERROR_CHECK(elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0001DA54 + (0xAA * 0xC) + 8, 9), custom_symbols.at("hurricane_spin_item_func") - 0x02000000));

	if (settings.instant_text_boxes) {
		LOG_AND_RETURN_IF_ERR(Apply_Patch("./asm/patch_diffs/b_button_skips_text_diff.json"));
		LOG_AND_RETURN_IF_ERR(Add_Relocations("./asm/patch_diffs/b_button_skips_text_reloc.json"));
		TWEAK_ERR_CHECK(make_all_text_instant());
	}
	if (settings.reveal_full_sea_chart) {
		LOG_AND_RETURN_IF_ERR(Apply_Patch("./asm/patch_diffs/reveal_sea_chart_diff.json"));
	}
	if (settings.invert_sea_compass_x_axis) {
		LOG_AND_RETURN_IF_ERR(Apply_Patch("./asm/patch_diffs/invert_sea_compass_x_axis_diff.json"));
	}
	if (settings.sword_mode == SwordMode::NoSword) {
		LOG_AND_RETURN_IF_ERR(Apply_Patch("./asm/patch_diffs/swordless_diff.json"));
		LOG_AND_RETURN_IF_ERR(Add_Relocations("./asm/patch_diffs/swordless_reloc.json"));
		RPX_ERROR_CHECK(elfUtil::removeRelocation(gRPX, {7, 0x001C1ED4})); //would overwrite branch to custom code
		TWEAK_ERR_CHECK(update_swordless_text());
	}
	if (settings.remove_music) {
		LOG_AND_RETURN_IF_ERR(Apply_Patch("./asm/patch_diffs/remove_music_diff.json"));
	}
	if(settings.chest_type_matches_contents) {
		TWEAK_ERR_CHECK(replace_ctmc_chest_texture());
	}

	TWEAK_ERR_CHECK(fix_deku_leaf_model());
	TWEAK_ERR_CHECK(allow_all_items_to_be_field_items());
	TWEAK_ERR_CHECK(remove_shop_item_forced_uniqueness_bit());
	TWEAK_ERR_CHECK(remove_ff2_cutscenes());
	TWEAK_ERR_CHECK(make_items_progressive());
	TWEAK_ERR_CHECK(add_chest_in_place_medli_gift());
	TWEAK_ERR_CHECK(add_chest_in_place_queen_fairy_cutscene());
	TWEAK_ERR_CHECK(modify_title_screen());
	TWEAK_ERR_CHECK(update_name_and_icon());
	TWEAK_ERR_CHECK(allow_dungeon_items_to_appear_anywhere());
	TWEAK_ERR_CHECK(fix_shop_item_y_offsets());
	TWEAK_ERR_CHECK(update_korl_dialog());
	TWEAK_ERR_CHECK(set_num_starting_triforce_shards(settings.num_starting_triforce_shards));
	TWEAK_ERR_CHECK(set_starting_health(settings.starting_pohs, settings.starting_hcs));
	TWEAK_ERR_CHECK(set_damage_multiplier(settings.damage_multiplier));
	TWEAK_ERR_CHECK(remove_makar_kidnapping());
	TWEAK_ERR_CHECK(increase_crawl_speed());
	TWEAK_ERR_CHECK(add_chart_number_to_item_get_messages());
	TWEAK_ERR_CHECK(increase_grapple_animation_speed());
	TWEAK_ERR_CHECK(increase_block_move_animation());
	TWEAK_ERR_CHECK(increase_misc_animations());
	TWEAK_ERR_CHECK(disable_invisible_walls());
	TWEAK_ERR_CHECK(update_tingle_statue_item_get_funcs());
	TWEAK_ERR_CHECK(make_tingle_statue_reward_rupee_rainbow_colored());
	TWEAK_ERR_CHECK(show_seed_hash_on_title_screen(u16_seedHash));
	TWEAK_ERR_CHECK(implement_key_bag());
	TWEAK_ERR_CHECK(add_chest_in_place_jabun_cutscene());
	TWEAK_ERR_CHECK(add_chest_in_place_master_sword());
	TWEAK_ERR_CHECK(update_spoil_sell_text());
	TWEAK_ERR_CHECK(fix_totg_warp_spawn());
	TWEAK_ERR_CHECK(remove_phantom_ganon_req_for_reefs());
	TWEAK_ERR_CHECK(fix_ff_door());
	TWEAK_ERR_CHECK(fix_stone_head_bugs());
	TWEAK_ERR_CHECK(show_tingle_statues_on_quest_screen());
	//key bag
	//bog warp
	//rat hole visibility
	//failsafe id 0 spawns

	TWEAK_ERR_CHECK(update_skip_rematch_bosses_game_variable(settings.skip_rematch_bosses));
	TWEAK_ERR_CHECK(update_sword_mode_game_variable(settings.sword_mode));
	TWEAK_ERR_CHECK(update_starting_gear(settings.starting_gear));
	if(settings.player_in_casual_clothes) {
		TWEAK_ERR_CHECK(set_casual_clothes());
	}
	TWEAK_ERR_CHECK(set_pig_color(settings.pig_color));

	RPX_ERROR_CHECK(gRPX.writeToFile(rpxPath.string()));
	
	return TweakError::NONE;
}

TweakError apply_necessary_post_randomization_tweaks(World& world, const bool& randomizeItems) {
	gRPX = FileTypes::ELF();
	RandoSession::fspath rpxPath = g_session.openGameFile("code/cking.rpx@RPX");
	EXTRACT_ERR_CHECK(rpxPath);
	FILETYPE_ERROR_CHECK(gRPX.loadFromFile(rpxPath.string())); //reload to avoid conflicts written between pre- and post- randomization tweaks

	std::map<std::string, Location>& itemLocations = world.locationEntries;
	const uint8_t startIsland = islandNameToRoomIndex(world.getArea("Link's Spawn").exits.front().getConnectedArea());

	TWEAK_ERR_CHECK(set_new_game_starting_location(0, startIsland));
	TWEAK_ERR_CHECK(change_ship_starting_island(startIsland));
	if (randomizeItems) {
		TWEAK_ERR_CHECK(update_shop_item_descriptions(itemLocations["Great Sea - Beedle Shop 20 Rupee Item"], itemLocations["Rock Spire Isle - Beedle 500 Rupee Item"], itemLocations["Rock Spire Isle - Beedle 950 Rupee Item"], itemLocations["Rock Spire Isle - Beedle 900 Rupee Item"]));
		TWEAK_ERR_CHECK(update_auction_item_names(itemLocations["Windfall Island - Auction 5 Rupee"], itemLocations["Windfall Island - Auction 40 Rupee"], itemLocations["Windfall Island - Auction 60 Rupee"], itemLocations["Windfall Island - Auction 80 Rupee"], itemLocations["Windfall Island - Auction 100 Rupee"]));
		TWEAK_ERR_CHECK(update_battlesquid_item_names(itemLocations["Windfall Island - Battle Squid First Prize"], itemLocations["Windfall Island - Battle Squid Second Prize"]));
		TWEAK_ERR_CHECK(update_item_names_in_letter_advertising_rock_spire_shop(itemLocations["Rock Spire Isle - Beedle 500 Rupee Item"], itemLocations["Rock Spire Isle - Beedle 950 Rupee Item"], itemLocations["Rock Spire Isle - Beedle 900 Rupee Item"]));
		TWEAK_ERR_CHECK(update_savage_labyrinth_hint_tablet(itemLocations["Outset Island - Savage Labyrinth Floor 30"], itemLocations["Outset Island - Savage Labyrinth Floor 50"]));
	}
	//Run some things after writing items to preserve offsets
	TWEAK_ERR_CHECK(add_ganons_tower_warp_to_ff2());
	TWEAK_ERR_CHECK(add_more_magic_jars());
	TWEAK_ERR_CHECK(add_pirate_ship_to_windfall()); //doesnt fix getting stuck behind door
	TWEAK_ERR_CHECK(add_hint_signs());
	TWEAK_ERR_CHECK(prevent_door_boulder_softlocks());
	TWEAK_ERR_CHECK(add_shortcut_warps_into_dungeons());
	TWEAK_ERR_CHECK(shorten_zephos_event());
	TWEAK_ERR_CHECK(shorten_auction_intro_event());
	TWEAK_ERR_CHECK(add_jabun_obstacles_to_default_layer());
	TWEAK_ERR_CHECK(remove_jabun_stone_door_event());
	TWEAK_ERR_CHECK(remove_minor_pan_cs());
	TWEAK_ERR_CHECK(show_dungeon_markers_on_chart(world));
	TWEAK_ERR_CHECK(update_entrance_events());

	if(world.getSettings().add_shortcut_warps_between_dungeons) {
		TWEAK_ERR_CHECK(add_cross_dungeon_warps());
	}
	
	RPX_ERROR_CHECK(gRPX.writeToFile(rpxPath.string()));

	return TweakError::NONE;
}
