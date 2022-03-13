#include "tweaks.hpp"

#define _USE_MATH_DEFINES
#define MAXIMUM_ADDITIONAL_STARTING_ITEMS 47

#include <cmath>
#include <fstream>
#include <codecvt>

#include "libs/tinyxml2.h"
#include "server/filetypes/bfres.hpp"
#include "server/filetypes/dzx.hpp"
#include "server/filetypes/events.hpp"
#include "server/filetypes/elf.hpp"
#include "server/filetypes/jpc.hpp"
#include "server/filetypes/msbt.hpp"
#include "server/filetypes/bflyt.hpp"
#include "server/filetypes/util/elfUtil.hpp"
#include "server/filetypes/util/msbtMacros.hpp"
#include "server/utility/stringUtil.hpp"

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
	int evnt_index;
};


namespace {
	FileTypes::ELF gRPX;
	static std::unordered_map<std::string, uint32_t> custom_symbols;

	void Load_Custom_Symbols(const std::string& file_path) {
		std::ifstream fptr(file_path, std::ios::in);

		nlohmann::json symbols = nlohmann::json::parse(fptr);
		for (const auto& symbol : symbols.items()) {
			uint32_t address = std::stoi(symbol.value().get<std::string>(), nullptr, 16);
			custom_symbols[symbol.key()] = address;
		}

		return;
	}
}

void Apply_Patch(const std::string& file_path) {
	std::ifstream fptr(file_path, std::ios::in);

	nlohmann::json patches = nlohmann::json::parse(fptr);

	for (auto& patch : patches.items()) {
		uint32_t offset = std::stoi(patch.key(), nullptr, 16);
		std::pair<uint32_t, uint32_t> sectionOffset = elfUtil::AddressToOffset(gRPX, offset);
		if (sectionOffset.first == 0 && sectionOffset.second == 0) { //address not in section
			std::string data;
			for (const std::string& byte : patch.value().get<std::vector<std::string>>()) {
				unsigned char val = std::stoi(byte, nullptr, 16);
				data += val;
			}
			gRPX.extend_section(2, offset, data); //add data at the specified offset
		}
		else {
			for (const std::string& byte : patch.value().get<std::vector<std::string>>()) {
				uint8_t toWrite = std::stoi(byte, nullptr, 16);
				elfUtil::write_u8(gRPX, sectionOffset, toWrite);
				sectionOffset.second++; //Cycles through the bytes individually, need to increase the offset by one each time
			}
		}
	}

	return;
}

//only applies relocations for .rela.text
void Add_Relocations(const std::string file_path) { //untested
	std::ifstream fptr;
	fptr.open(file_path, std::ios::in);

	nlohmann::json relocations = nlohmann::json::parse(fptr);

	std::string entry;
	entry.resize(12);
	for (auto& relocation : relocations) {
		Elf32_Rela reloc;
		reloc.r_offset = std::stoi(relocation["r_offset"].get<std::string>(), nullptr, 16);
		reloc.r_info = std::stoi(relocation["r_info"].get<std::string>(), nullptr, 16);
		reloc.r_addend = std::stoi(relocation["r_addend"].get<std::string>(), nullptr, 16);

		entry.replace(0, 4, reinterpret_cast<const char*>(&reloc.r_offset), 4);
		entry.replace(4, 4, reinterpret_cast<const char*>(&reloc.r_info), 4);
		entry.replace(8, 4, reinterpret_cast<const char*>(&reloc.r_addend), 4);
		gRPX.extend_section(7, entry);
	}

	return;
}

void Remove_Relocation(const std::pair<int, int>& offset) {
	gRPX.shdr_table[offset.first].second.data.replace(offset.second, 0xC, 0xC, '\0');
	return;
}


std::u16string word_wrap_string(const std::u16string& string, const int max_line_len) {
	unsigned int index_in_str = 0;
	std::u16string wordwrapped_str;
	std::u16string current_word;
	int curr_word_len = 0;
	int len_curr_line = 0;

	while (index_in_str < string.length()) { //length is weird because its utf-16
		char16_t character = string[index_in_str];

		if (character == u'\x0E') { //need to parse the commands, only implementing a few necessary ones for now (will break with other commands)
			std::u16string substr;
			int code_len = 0;
			if (string[index_in_str + 1] == u'\x00') {
				if (string[index_in_str + 2] == u'\x03') { //color command
					if (string[index_in_str + 4] == u'\xFF') { //text color white, weird length
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
			len_curr_line = curr_word_len + 1;
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
	char16_t first_letter = std::tolower(string[0]);
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

/*std::u16string gameItemToName(const GameItem item) {
	static std::unordered_map<GameItem, std::u16string> itemNameMap = { //terrible indentation is to make copying/editing easier, will remove once stuff is more finalized
		{GameItem::HeartDrop, 						u"Heart (Pickup)"},
		{GameItem::GreenRupee, 						u"Green Rupee"},
		{GameItem::BlueRupee, 						u"Blue Rupee"},
		{GameItem::YellowRupee, 					u"Yellow Rupee"},
		{GameItem::RedRupee, 						u"Red Rupee"},
		{GameItem::PurpleRupee, 					u"Purple Rupee"},
		{GameItem::OrangeRupee, 					u"Orange Rupee"},
		{GameItem::PieceOfHeart, 					u"Piece of Heart"},
		{GameItem::HeartContainer, 					u"Heart Container"},
		{GameItem::SmallMagicDrop, 					u"Small Magic Jar(Pickup)"},
		{GameItem::LargeMagicDrop, 					u"Large Magic Jar(Pickup)"},
		{GameItem::FiveBombs, 						u"5 Bombs(Pickup)"},
		{GameItem::TenBombs, 						u"10 Bombs(Pickup)"},
		{GameItem::TwentyBombs, 					u"20 Bombs(Pickup)"},
		{GameItem::ThirtyBombs, 					u"30 Bombs(Pickup)"},
		{GameItem::SilverRupee, 					u"Silver Rupee"},
		{GameItem::TenArrows, 						u"10 Arrows(Pickup)"},
		{GameItem::TwentyArrows, 					u"20 Arrows(Pickup)"},
		{GameItem::ThirtyArrows, 					u"30 Arrows(Pickup)"},
		{GameItem::DRCSmallKey, 					u"DRC Small Key"},
		{GameItem::DRCBigKey, 						u"DRC Big Key"},
		{GameItem::SmallKey, 						u"Small Key"},
		{GameItem::Fairy, 							u"Fairy(Pickup)"},
		{GameItem::YellowRupee2, 					u"Yellow Rupee(Joke Message)"},
		{GameItem::DRCDungeonMap, 					u"DRC Dungeon Map"},
		{GameItem::DRCCompass, 						u"DRC Compass"},
		{GameItem::FWSmallKey, 						u"FW Small Key"},
		{GameItem::ThreeHearts, 					u"Three Hearts(Pickup)"},
		{GameItem::JoyPendant, 						u"Joy Pendant"},
		{GameItem::Telescope, 						u"Telescope"},
		{GameItem::TingleBottle, 					u"Tingle Bottle"},
		{GameItem::WindWaker, 						u"Wind Waker"},
		{GameItem::ProgressivePictoBox, 			u"Picto Box"},
		{GameItem::SpoilsBag, 						u"Spoils Bag"},
		{GameItem::GrapplingHook, 					u"Grappling Hook"},
		{GameItem::DeluxePicto, 					u"Deluxe Picto Box"},
		{GameItem::ProgressiveBow, 					u"Hero's Bow"},
		{GameItem::PowerBracelets, 					u"Power Bracelets"},
		{GameItem::IronBoots, 						u"Iron Boots"},
		{GameItem::MagicArmor, 						u"Magic Armor"},
		{GameItem::BaitBag, 						u"Bait Bag"},
		{GameItem::Boomerang, 						u"Boomerang"},
		{GameItem::Hookshot, 						u"Hookshot"},
		{GameItem::DeliveryBag, 					u"Delivery Bag"},
		{GameItem::Bombs, 							u"Bombs"},
		{GameItem::HerosClothes, 					u"Hero's Clothes"},
		{GameItem::SkullHammer, 					u"Skull Hammer"},
		{GameItem::DekuLeaf, 						u"Deku Leaf"},
		{GameItem::FireIceArrows, 					u"Fire and Ice Arrows"},
		{GameItem::LightArrow, 						u"Light Arrow"},
		{GameItem::HerosNewClothes, 				u"Hero's New Clothes"},
		{GameItem::ProgressiveSword, 				u"Hero's Sword"},
		{GameItem::MasterSwordPowerless, 			u"Master Sword(Powerless)"},
		{GameItem::MasterSwordHalf, 				u"Master Sword(Half Power)"},
		{GameItem::ProgressiveShield, 				u"Hero's Shield"},
		{GameItem::MirrorShield, 					u"Mirror Shield"},
		{GameItem::RecoveredHerosSword, 			u"Recovered Hero's Sword"},
		{GameItem::MasterSwordFull, 				u"Master Sword(Full Power)"},
		{GameItem::PieceOfHeart2, 					u"Piece of Heart(Alternate Message)"},
		{GameItem::FWBigKey, 						u"FW Big Key"},
		{GameItem::FWDungeonMap, 					u"FW Dungeon Map"},
		{GameItem::PiratesCharm, 					u"Pirate's Charm"},
		{GameItem::HerosCharm, 						u"Hero's Charm"},
		{GameItem::SkullNecklace, 					u"Skull Necklace"},
		{GameItem::BokoBabaSeed, 					u"Boko Baba Seed"},
		{GameItem::GoldenFeather, 					u"Golden Feather"},
		{GameItem::KnightsCrest, 					u"Knight's Crest"},
		{GameItem::RedChuJelly, 					u"Red Chu Jelly"},
		{GameItem::GreenChuJelly, 					u"Green Chu Jelly"},
		{GameItem::BlueChuJelly, 					u"Blue Chu Jelly"},
		{GameItem::DungeonMap, 						u"Dungeon Map"},
		{GameItem::Compass, 						u"Compass"},
		{GameItem::BigKey, 							u"Big Key"},
		{GameItem::EmptyBottle, 					u"Empty Bottle"},
		{GameItem::RedPotion, 						u"Red Potion"},
		{GameItem::GreenPotion, 					u"Green Potion"},
		{GameItem::BluePotion, 						u"Blue Potion"},
		{GameItem::ElixirSoupHalf, 					u"Elixir Soup(1 / 2)"},
		{GameItem::ElixirSoup, 						u"Elixir Soup"},
		{GameItem::BottledWater, 					u"Bottled Water"},
		{GameItem::FairyInBottle, 					u"Fairy in Bottle"},
		{GameItem::ForestFirefly, 					u"Forest Firefly"},
		{GameItem::ForestWater, 					u"Forest Water"},
		{GameItem::FWCompass, 						u"FW Compass"},
		{GameItem::TotGSmallKey, 					u"TotG Small Key"},
		{GameItem::TotGBigKey, 						u"TotG Big Key"},
		{GameItem::TotGDungeonMap, 					u"TotG Dungeon Map"},
		{GameItem::TotGCompass, 					u"TotG Compass"},
		{GameItem::FFDungeonMap, 					u"FF Dungeon Map"},
		{GameItem::FFCompass, 						u"FF Compass"},
		{GameItem::TriforceShard1, 					u"Triforce Shard 1"},
		{GameItem::TriforceShard2, 					u"Triforce Shard 2"},
		{GameItem::TriforceShard3, 					u"Triforce Shard 3"},
		{GameItem::TriforceShard4, 					u"Triforce Shard 4"},
		{GameItem::TriforceShard5, 					u"Triforce Shard 5"},
		{GameItem::TriforceShard6, 					u"Triforce Shard 6"},
		{GameItem::TriforceShard7, 					u"Triforce Shard 7"},
		{GameItem::TriforceShard8, 					u"Triforce Shard 8"},
		{GameItem::NayrusPearl, 					u"Nayru's Pearl"},
		{GameItem::DinsPearl, 						u"Din's Pearl"},
		{GameItem::FaroresPearl, 					u"Farore's Pearl"},
		{GameItem::WindsRequiem, 					u"Wind's Requiem"},
		{GameItem::BalladOfGales, 					u"Ballad of Gales"},
		{GameItem::CommandMelody, 					u"Command Melody"},
		{GameItem::EarthGodsLyric, 					u"Earth God's Lyric"},
		{GameItem::WindGodsAria, 					u"Wind God's Aria"},
		{GameItem::SongOfPassing, 					u"Song of Passing"},
		{GameItem::ETSmallKey, 						u"ET Small Key"},
		{GameItem::ETBigKey, 						u"ET Big Key"},
		{GameItem::ETDungeonMap, 					u"ET Dungeon Map"},
		{GameItem::ETCompass, 						u"ET Compass"},
		{GameItem::SwiftSail, 						u"Swift Sail"},
		{GameItem::ProgressiveSail, 				u"ProgressiveSail"},
		{GameItem::TriforceChart1Deciphered, 		u"Triforce Chart 1 got deciphered"},
		{GameItem::TriforceChart2Deciphered, 		u"Triforce Chart 2 got deciphered"},
		{GameItem::TriforceChart3Deciphered, 		u"Triforce Chart 3 got deciphered"},
		{GameItem::TriforceChart4Deciphered, 		u"Triforce Chart 4 got deciphered"},
		{GameItem::TriforceChart5Deciphered, 		u"Triforce Chart 5 got deciphered"},
		{GameItem::TriforceChart6Deciphered, 		u"Triforce Chart 6 got deciphered"},
		{GameItem::TriforceChart7Deciphered, 		u"Triforce Chart 7 got deciphered"},
		{GameItem::TriforceChart8Deciphered, 		u"Triforce Chart 8 got deciphered"},
		{GameItem::WTSmallKey, 						u"WT Small Key"},
		{GameItem::AllPurposeBait, 					u"All - Purpose Bait"},
		{GameItem::HyoiPear, 						u"Hyoi Pear"},
		{GameItem::WTBigKey, 						u"WT Big Key"},
		{GameItem::WTDungeonMap, 					u"WT Dungeon Map"},
		{GameItem::WTCompass, 						u"WT Compass"},
		{GameItem::TownFlower, 						u"Town Flower"},
		{GameItem::SeaFlower, 						u"Sea Flower"},
		{GameItem::ExoticFlower, 					u"Exotic Flower"},
		{GameItem::HerosFlag, 						u"Hero's Flag"},
		{GameItem::BigCatchFlag, 					u"Big Catch Flag"},
		{GameItem::BigSaleFlag, 					u"Big Sale Flag"},
		{GameItem::Pinwheel, 						u"Pinwheel"},
		{GameItem::SickleMoonFlag, 					u"Sickle Moon Flag"},
		{GameItem::SkullTowerIdol, 					u"Skull Tower Idol"},
		{GameItem::FountainIdol, 					u"Fountain Idol"},
		{GameItem::PostmanStatue, 					u"Postman Statue"},
		{GameItem::ShopGuruStatue, 					u"Shop Guru Statue"},
		{GameItem::FathersLetter, 					u"Father's Letter"},
		{GameItem::NoteToMom, 						u"Note to Mom"},
		{GameItem::MaggiesLetter, 					u"Maggie's Letter"},
		{GameItem::MoblinsLetter, 					u"Moblin's Letter"},
		{GameItem::CabanaDeed, 						u"Cabana Deed"},
		{GameItem::ComplimentaryID, 				u"Complimentary ID"},
		{GameItem::FillUpCoupon, 					u"Fill - Up Coupon"},
		{GameItem::LegendaryPictograph, 			u"Legendary Pictograph"},
		{GameItem::DragonTingleStatue, 				u"Dragon Tingle Statue"},
		{GameItem::ForbiddenTingleStatue, 			u"Forbidden Tingle Statue"},
		{GameItem::GoddessTingleStatue, 			u"Goddess Tingle Statue"},
		{GameItem::EarthTingleStatue, 				u"Earth Tingle Statue"},
		{GameItem::WindTingleStatue, 				u"Wind Tingle Statue"},
		{GameItem::HurricaneSpin, 					u"Hurricane Spin"},
		{GameItem::ProgressiveWallet, 				u"1000 Rupee Wallet"},
		{GameItem::FiveThousandWallet, 				u"5000 Rupee Wallet"},
		{GameItem::ProgressiveBombBag, 				u"60 Bomb Bomb Bag"},
		{GameItem::NinetyNineBombBag, 				u"99 Bomb Bomb Bag"},
		{GameItem::ProgressiveQuiver, 				u"60 Arrow Quiver"},
		{GameItem::NinetyNineQuiver, 				u"99 Arrow Quiver"},
		{GameItem::MagicMeterUpgrade, 				u"Magic Meter Upgrade"},
		{GameItem::FiftyRupees, 					u"50 Rupees, reward for finding 1 Tingle Statue"},
		{GameItem::HundredRupees, 					u"100 Rupees, reward for finding 2 Tingle Statues"},
		{GameItem::HundredFiftyRupees, 				u"150 Rupees, reward for finding 3 Tingle Statues"},
		{GameItem::TwoHundredRupees, 				u"200 Rupees, reward for finding 4 Tingle Statues"},
		{GameItem::TwoHundredFiftyRupees, 			u"250 Rupees, reward for finding 5 Tingle Statues"},
		{GameItem::RainbowRupee, 					u"Rainbow Rupee"},
		{GameItem::SubmarineChart, 					u"Submarine Chart"},
		{GameItem::BeedlesChart, 					u"Beedle's Chart"},
		{GameItem::PlatformChart, 					u"Platform Chart"},
		{GameItem::LightRingChart, 					u"Light Ring Chart"},
		{GameItem::SecretCaveChart, 				u"Secret Cave Chart"},
		{GameItem::SeaHeartsChart, 					u"Sea Hearts Chart"},
		{GameItem::IslandHeartsChart, 				u"Island Hearts Chart"},
		{GameItem::GreatFairyChart, 				u"Great Fairy Chart"},
		{GameItem::OctoChart, 						u"Octo Chart"},
		{GameItem::INcredibleChart, 				u"IN - credible Chart"},
		{GameItem::TreasureChart7, 					u"Treasure Chart 7"},
		{GameItem::TreasureChart27, 				u"Treasure Chart 27"},
		{GameItem::TreasureChart21, 				u"Treasure Chart 21"},
		{GameItem::TreasureChart13, 				u"Treasure Chart 13"},
		{GameItem::TreasureChart32, 				u"Treasure Chart 32"},
		{GameItem::TreasureChart19, 				u"Treasure Chart 19"},
		{GameItem::TreasureChart41, 				u"Treasure Chart 41"},
		{GameItem::TreasureChart26, 				u"Treasure Chart 26"},
		{GameItem::TreasureChart8, 					u"Treasure Chart 8"},
		{GameItem::TreasureChart37, 				u"Treasure Chart 37"},
		{GameItem::TreasureChart25, 				u"Treasure Chart 25"},
		{GameItem::TreasureChart17, 				u"Treasure Chart 17"},
		{GameItem::TreasureChart36, 				u"Treasure Chart 36"},
		{GameItem::TreasureChart22, 				u"Treasure Chart 22"},
		{GameItem::TreasureChart9, 					u"Treasure Chart 9"},
		{GameItem::GhostShipChart, 					u"Ghost Ship Chart"},
		{GameItem::TinglesChart, 					u"Tingle's Chart"},
		{GameItem::TreasureChart14, 				u"Treasure Chart 14"},
		{GameItem::TreasureChart10, 				u"Treasure Chart 10"},
		{GameItem::TreasureChart40, 				u"Treasure Chart 40"},
		{GameItem::TreasureChart3, 					u"Treasure Chart 3"},
		{GameItem::TreasureChart4, 					u"Treasure Chart 4"},
		{GameItem::TreasureChart28, 				u"Treasure Chart 28"},
		{GameItem::TreasureChart16, 				u"Treasure Chart 16"},
		{GameItem::TreasureChart18, 				u"Treasure Chart 18"},
		{GameItem::TreasureChart34, 				u"Treasure Chart 34"},
		{GameItem::TreasureChart29, 				u"Treasure Chart 29"},
		{GameItem::TreasureChart1, 					u"Treasure Chart 1"},
		{GameItem::TreasureChart35, 				u"Treasure Chart 35"},
		{GameItem::TreasureChart12, 				u"Treasure Chart 12"},
		{GameItem::TreasureChart6, 					u"Treasure Chart 6"},
		{GameItem::TreasureChart24, 				u"Treasure Chart 24"},
		{GameItem::TreasureChart39, 				u"Treasure Chart 39"},
		{GameItem::TreasureChart38, 				u"Treasure Chart 38"},
		{GameItem::TreasureChart2, 					u"Treasure Chart 2"},
		{GameItem::TreasureChart33, 				u"Treasure Chart 33"},
		{GameItem::TreasureChart31, 				u"Treasure Chart 31"},
		{GameItem::TreasureChart23, 				u"Treasure Chart 23"},
		{GameItem::TreasureChart5, 					u"Treasure Chart 5"},
		{GameItem::TreasureChart20, 				u"Treasure Chart 20"},
		{GameItem::TreasureChart30, 				u"Treasure Chart 30"},
		{GameItem::TreasureChart15, 				u"Treasure Chart 15"},
		{GameItem::TreasureChart11, 				u"Treasure Chart 11"},
		{GameItem::TreasureChart46, 				u"Treasure Chart 46"},
		{GameItem::TreasureChart45, 				u"Treasure Chart 45"},
		{GameItem::TreasureChart44, 				u"Treasure Chart 44"},
		{GameItem::TriforceChart3, 					u"Triforce Chart 3"},
		{GameItem::TreasureChart43, 				u"Treasure Chart 43"},
		{GameItem::TriforceChart2, 					u"Triforce Chart 2"},
		{GameItem::TreasureChart42, 				u"Treasure Chart 42"},
		{GameItem::TriforceChart1, 					u"Triforce Chart 1"},
		{GameItem::INVALID,							u"Nothing"}
	};

	if (itemNameMap.count(item) == 0)
	{
		return u"INVALID";
	}
	return itemNameMap.at(item);
}*/



void set_new_game_starting_location(const uint8_t spawn_id, const uint8_t room_index) {
	elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, 0x025B508F), room_index);
	elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, 0x025B50CB), room_index);
	elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, 0x025B5093), spawn_id);
	elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, 0x025B50CF), spawn_id);
	return;
}

void change_ship_starting_island(const uint8_t room_index) {
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
	FileTypes::DZXFile room_dzr;
	room_dzr.loadFromFile(inPath.string());
	std::vector<ChunkEntry*> ship_spawns = room_dzr.entries_by_type("SHIP");
	ChunkEntry* ship_spawn_0 = nullptr; //initialization is just to make compiler happy
	for (ChunkEntry* spawn : ship_spawns) { //Find spawn with ID 0
		if (reinterpret_cast<uint8_t*>(&spawn->data[0xE]) == 0) ship_spawn_0 = spawn;
	}

	FileTypes::DZXFile stage_dzs;
	RandoSession::fspath stagePath = g_session.openGameFile("content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs");
	stage_dzs.loadFromFile(stagePath.string());
	std::vector<ChunkEntry*> actors = stage_dzs.entries_by_type("ACTR");
	for (ChunkEntry* actor : actors) {
		if (std::strncmp(&actor->data[0], "SHIP", 4) == 0 && ship_spawn_0 != nullptr) {
			actor->data.replace(0xC, 0xC, ship_spawn_0->data, 0x0, 0xC);
			actor->data.replace(0x1A, 0x2, ship_spawn_0->data, 0xC, 0x2);
			actor->data.replace(0x10, 0x4, "\xC8\xF4\x24\x00", 0x0, 0x4); //prevent softlock on fire mountain (may be wrong offset)
		}
	}
	room_dzr.writeToFile(inPath.string());

	stage_dzs.writeToFile(stagePath.string());
	return;
}

void start_at_outset_dock() {
	set_new_game_starting_location(0, 44);
	return;
}

void start_ship_at_outset() {
	change_ship_starting_island(44);
}

void make_all_text_instant() {
	const RandoSession::fspath paths[4] = {
		"content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt",
		"content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt",
		"content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message3_msbt.szs@YAZ0@SARC@message3.msbt",
		"content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message4_msbt.szs@YAZ0@SARC@message4.msbt"
	};

	for (int i = 0; i < 4; i++) {
		RandoSession::fspath inPath = g_session.openGameFile(paths[i]);
		FileTypes::MSBTFile msbt;
		msbt.loadFromFile(inPath.string());

		for (auto& [label, message] : msbt.messages_by_label) {
			std::u16string& String = message.text.message;

			std::u16string::size_type wait = String.find(std::u16string(u"\x0e\x01\x06\x02", 4)); //dont use macro because duration shouldnt matter
			while (wait != std::u16string::npos) {
				String.erase(wait, 5);
				wait = String.find(std::u16string(u"\x0e\x01\x06\x02", 4));
			}

			std::u16string::size_type wait_dismiss = String.find(std::u16string(u"\x0e\x01\x03\x02", 4)); //dont use macro because duration shouldnt matter
			while (wait_dismiss != std::u16string::npos) {
				if (label == "07726" || label == "02488") { //exclude messages that are broken by removing the command
					wait_dismiss = String.find(std::u16string(u"\x0e\x01\x03\x02", 4));
					continue;
				}
				String.erase(wait_dismiss, 5);
				wait_dismiss = String.find(std::u16string(u"\x0e\x01\x03\x02", 4));
			}

			std::u16string::size_type wait_dismiss_prompt = String.find(std::u16string(u"\x0e\x01\x02\x02", 4)); //dont use macro because duration shouldnt matter
			while (wait_dismiss_prompt != std::u16string::npos) {
				String.erase(wait_dismiss_prompt, 5);
				wait_dismiss_prompt = String.find(std::u16string(u"\x0e\x01\x02\x02", 4));
			}
		}
		msbt.writeToFile(inPath.string());
	}

	return;
}

void fix_deku_leaf_model() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/Omori_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr");
	std::string data("item\x00\x00\x00\x00\x01\xFF\x02\x34\xc4\x08\x7d\x81\x45\x9d\x59\xec\xc3\xf5\x8e\xd9\x00\x00\x00\x00\x00\xff\xff\xff", 0x20);

	FileTypes::DZXFile dzr;
	dzr.loadFromFile(path.string());
	std::vector<ChunkEntry*> actors = dzr.entries_by_type("ACTR");
	for (ChunkEntry* actor : actors) {
		if (std::strncmp(&actor->data[0], "itemDek\x00", 8) == 0) actor->data = data;
	}
	dzr.writeToFile(path.string());

	return;
}

void allow_all_items_to_be_field_items() {

	uint32_t item_resources_list_start = 0x101e4674;
	uint32_t field_item_resources_list_start = 0x101e6a74;

	const std::array<uint8_t, 171> Item_Ids_Without_Field_Model = {
	0x1a, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x35, 0x36, 0x39, 0x3a, 0x3c, 0x3e, 0x3f, 0x42, 0x43, 0x4c, 0x4d, 0x4e, 0x50, 0x51, 0x52,
	0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x98,
	0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd,
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
		else if (item_id == 0xb2) {
			item_id_to_copy_from = 0x52;
			item_resources_addr_to_fix = item_resources_list_start + item_id * 0x24;
		}
		else {
			item_id_to_copy_from = item_id;
		}

		uint32_t item_resources_addr_to_copy_from = item_resources_list_start + item_id_to_copy_from * 0x24;
		uint32_t field_item_resources_addr = field_item_resources_list_start + item_id * 0x1c;
		uint32_t szs_name_pointer = 0;
		int section_start = 0x10000000;

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

		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr), szs_name_pointer);
		if (item_resources_addr_to_fix) {
			elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_fix), szs_name_pointer);
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

		std::string entry;
		entry.resize(12);
		entry.replace(0, 4, reinterpret_cast<const char*>(&relocation.r_offset), 4);
		entry.replace(4, 4, reinterpret_cast<const char*>(&relocation.r_info), 4);
		entry.replace(8, 4, reinterpret_cast<const char*>(&relocation.r_addend), 4);
		gRPX.extend_section(9, entry);

		if (item_resources_addr_to_fix) {
			Elf32_Rela relocation2;
			relocation2.r_offset = item_resources_addr_to_fix;
			relocation2.r_info = relocation.r_info; //same as first entry
			relocation2.r_addend = relocation.r_addend; //same as first entry

			std::string entry2;
			entry2.resize(12);
			entry2.replace(0, 4, reinterpret_cast<const char*>(&relocation2.r_offset), 4);
			entry2.replace(4, 4, reinterpret_cast<const char*>(&relocation2.r_info), 4);
			entry2.replace(8, 4, reinterpret_cast<const char*>(&relocation2.r_addend), 4);
			gRPX.extend_section(9, entry2);
		}

		std::vector<uint8_t> data1 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_copy_from + 8), 0xD);
		std::vector<uint8_t> data2 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_copy_from + 0x1C), 4);
		elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr + 4), data1);
		elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr + 0x14), data2);
		if (item_resources_addr_to_fix) {
			elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_fix + 8), data1);
			elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_fix + 0x1C) , data2);
		}
	}

	for (const uint32_t& address : { 0x0255220CU, 0x02552214U, 0x0255221CU, 0x02552224U, 0x0255222CU, 0x02552234U, 0x02552450U }) { //unsigned to make compiler happy
		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, address), 0x60000000);
	}

	Apply_Patch("./asm/patch_diffs/field_items_diff.json"); //some special stuff because HD silly

	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0007a2d0, 7), 0x00011ed8); //Update the Y offset that is being read (.rela.text edit)

	uint32_t extra_item_data_list_start = 0x101e8674;
	for (int item_id = 0x00; item_id < 0xFF + 1; item_id++) {
		uint32_t item_extra_data_entry_addr = extra_item_data_list_start + 4 * item_id;
		uint8_t original_y_offset = elfUtil::read_u8(gRPX, elfUtil::AddressToOffset(gRPX, item_extra_data_entry_addr + 1));
		if (original_y_offset == 0) {
			elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, item_extra_data_entry_addr + 1), 0x28);
		}
		uint8_t original_radius = elfUtil::read_u8(gRPX, elfUtil::AddressToOffset(gRPX, item_extra_data_entry_addr + 2));
		if (original_radius == 0) {
			elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, item_extra_data_entry_addr + 2), 0x28);
		}
	}
	//Add vscroll.szs

	return;
}

void remove_shop_item_forced_uniqueness_bit() {
	uint32_t shop_item_data_list_start = 0x101eaea4;

	const std::array<uint8_t, 4> item_indexes = { 0x0, 0xB, 0xC, 0xD };
	for (const uint8_t& shop_item_index : item_indexes) {
		uint32_t shop_item_data_addr = shop_item_data_list_start + shop_item_index * 0x10;
		uint8_t buy_requirements_bitfield = elfUtil::read_u8(gRPX, elfUtil::AddressToOffset(gRPX, shop_item_data_addr + 0xC));
		buy_requirements_bitfield = (buy_requirements_bitfield & (~0x2));
		elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, shop_item_data_addr + 0xC), buy_requirements_bitfield);
	}
}

void remove_ff2_cutscenes() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/M2tower_Room0.szs@YAZ0@SARC@Room0.bfres@room.dzr");

	FileTypes::DZXFile dzr;
	dzr.loadFromFile(path.string());

	std::vector<ChunkEntry*> spawns = dzr.entries_by_type("PLYR");
	for (ChunkEntry* spawn : spawns) {
		if (spawn->data[29] == '\x10') spawn->data[8] = '\xFF';
	}

	std::vector<ChunkEntry*> exits = dzr.entries_by_type("SCLS");
	for (ChunkEntry* exit : exits) {
		if (std::strncmp(&exit->data[0], "M2ganon\x00", 8) == 0) exit->data = std::string("sea\x00\x00\x00\x00\x00\x00\x01\x00\xFF", 0xC);
	}
	dzr.writeToFile(path.string());

	return;
}

void make_items_progressive() {
	Apply_Patch("./asm/patch_diffs/make_items_progressive_diff.json");

	uint32_t item_get_func_pointer = 0x0001da54; //First relevant relocation entry in .rela.data (overwrites .data section when loaded)

	for (const uint8_t sword_id : {0x38U, 0x39U, 0x3AU, 0x3DU, 0x3EU}) {
		uint32_t item_get_func_addr = item_get_func_pointer + (sword_id * 0xC) + 8;
		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_sword_item_func") - 0x02000000);
	}
	for (const uint8_t shield_id : {0x3BU, 0x3CU}) {
		uint32_t item_get_func_addr = item_get_func_pointer + (shield_id * 0xC) + 8;
		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_shield_item_func") - 0x02000000);
	}
	for (const uint8_t bow_id : {0x27U, 0x35U, 0x36U}) {
		uint32_t item_get_func_addr = item_get_func_pointer + (bow_id * 0xC) + 8;
		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_bow_item_func") - 0x02000000);
	}
	for (const uint8_t wallet_id : {0xABU, 0xACU}) {
		uint32_t item_get_func_addr = item_get_func_pointer + (wallet_id * 0xC) + 8;
		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_wallet_item_func") - 0x02000000);
	}
	for (const uint8_t bomb_bag_id : {0xADU, 0xAEU}) {
		uint32_t item_get_func_addr = item_get_func_pointer + (bomb_bag_id * 0xC) + 8;
		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_bomb_bag_item_func") - 0x02000000);
	}
	for (const uint8_t quiver_id : {0xAFU, 0xB0U}) {
		uint32_t item_get_func_addr = item_get_func_pointer + (quiver_id * 0xC) + 8;
		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_quiver_item_func") - 0x02000000);
	}
	for (const uint8_t picto_id : {0x23U, 0x26U}) {
		uint32_t item_get_func_addr = item_get_func_pointer + (picto_id * 0xC) + 8;
		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_picto_box_item_func") - 0x02000000);
	}
	for (const uint8_t sail_id : {0x77U, 0x78U}) {
		uint32_t item_get_func_addr = item_get_func_pointer + (sail_id * 0xC) + 8;
		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_addr, 9), custom_symbols.at("progressive_sail_item_func") - 0x02000000);
	}

	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0254e8c4), 0x60000000);
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0254e8cc), 0x60000000);
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0254e66c), 0x60000000);
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0254e674), 0x60000000);

	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0254e96c), 0x60000000);
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0254e97c), 0x60000000);

	return;
}

void add_ganons_tower_warp_to_ff2() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/szs_permanent1.pack@SARC@sea_Room1.szs@YAZ0@SARC@Room1.bfres@BFRES@room.dzr");

	FileTypes::DZXFile dzr;
	dzr.loadFromFile(path.string());
	ChunkEntry& warp = dzr.add_entity("ACTR", 1);
	warp.data = std::string("Warpmj\x00\x00\x00\x00\x00\x11\xc8\x93\x0f\xd9\x00\x00\x00\x00\xc8\x91\xf7\xfa\x00\x00\x00\x00\x00\x00\xff\xff", 0x20);
	dzr.writeToFile(path.string());

	return;
}

void add_chest_in_place_medli_gift() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/M_Dra09_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs");

	FileTypes::DZXFile dzs;
	dzs.loadFromFile(path.string());
	ChunkEntry& chest = dzs.add_entity("TRES");
	chest.data = std::string("takara3\x00\xFF\x20\x08\x80\xc4\xca\x99\xec\x46\x54\x80\x00\x43\x83\x84\x5a\x00\x09\xcc\x16\x0f\xff\xff\xff", 0x20);
	dzs.writeToFile(path.string());

	RandoSession::fspath path2 = g_session.openGameFile("content/Common/Stage/M_NewD2_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs");

	FileTypes::DZXFile dzs2;
	dzs2.loadFromFile(path2.string());
	ChunkEntry& dummyChest = dzs2.add_entity("TRES");
	dummyChest.data = std::string("takara3\x00\xFF\x20\x08\x80\xc4\xca\x99\xec\x46\x54\x80\x00\x43\x83\x84\x5a\x00\x09\xcc\x16\x0f\xff\xff\xff", 0x20);
	dzs2.writeToFile(path2.string());
	return;
}

void add_chest_in_place_queen_fairy_cutscene() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/szs_permanent2.pack@SARC@sea_Room9.szs@YAZ0@SARC@Room9.bfres@BFRES@room.dzr");

	FileTypes::DZXFile dzr;
	dzr.loadFromFile(path.string());
	ChunkEntry& chest = dzr.add_entity("TRES");
	chest.data = std::string("takara3\x00\xFF\x20\x0e\x00\xc8\x2f\xcf\xc0\x44\x34\xc0\x00\xc8\x43\x4e\xc0\x00\x09\x10\x00\xa5\xff\xff\xff", 0x20);
	dzr.writeToFile(path.string());

	return;
}

void add_more_magic_jars() {
	{
		RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/M_NewD2_Room2.szs@YAZ0@SARC@Room2.bfres@BFRES@room.dzr");

		FileTypes::DZXFile drc_hub;
		drc_hub.loadFromFile(path.string());
		std::vector<ChunkEntry*> actors = drc_hub.entries_by_type("ACTR");
		std::vector<ChunkEntry*> skulls;
		for (ChunkEntry* actor : actors) {
			if (std::strncmp(&actor->data[0], "Odokuro\x00", 8) == 0) skulls.push_back(actor);
		}
		skulls[2]->data.replace(0x8, 0x4, "\x75\x7f\xff\x09", 0, 4);
		skulls[5]->data.replace(0x8, 0x4, "\x75\x7f\xff\x0A", 0, 4);
		drc_hub.writeToFile(path.string());
	}

	{
		RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/M_NewD2_Room10.szs@YAZ0@SARC@Room10.bfres@BFRES@room.dzr");

		FileTypes::DZXFile drc_before_boss;
		drc_before_boss.loadFromFile(path.string());
		std::vector<ChunkEntry*> actors = drc_before_boss.entries_by_type("ACTR");
		std::vector<ChunkEntry*> skulls;
		for (ChunkEntry* actor : actors) {
			if (std::strncmp(&actor->data[0], "Odokuro\x00", 8) == 0) skulls.push_back(actor);
		}
		skulls[0]->data.replace(0x8, 0x4, "\x75\x7f\xff\x0A", 0, 4);
		skulls[9]->data.replace(0x8, 0x4, "\x75\x7f\xff\x0A", 0, 4);
		drc_before_boss.writeToFile(path.string());
	}

	{
		RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/szs_permanent1.pack@SARC@sea_Room13.szs@YAZ0@SARC@Room13.bfres@BFRES@room.dzr");

		FileTypes::DZXFile dri;
		dri.loadFromFile(path.string());
		ChunkEntry& grass1 = dri.add_entity("ACTR");
		grass1.data = std::string("\x6B\x75\x73\x61\x78\x31\x00\x00\x00\x00\x0E\x00\x48\x4C\xC7\x80\x44\xED\x80\x00\xC8\x45\xB7\xC0\x00\x00\x00\x00\x00\x00\xFF\xFF", 0x20);
		ChunkEntry& grass2 = dri.add_entity("ACTR");
		grass2.data = std::string("\x6B\x75\x73\x61\x78\x31\x00\x00\x00\x00\x0E\x00\x48\x4C\x6D\x40\x44\xA2\x80\x00\xC8\x4D\x38\x40\x00\x00\x00\x00\x00\x00\xFF\xFF", 0x20);
		dri.writeToFile(path.string());
	}

	{
		RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/Siren_Room14.szs@YAZ0@SARC@Room14.bfres@BFRES@room.dzr");

		FileTypes::DZXFile totg;
		totg.loadFromFile(path.string());
		std::vector<ChunkEntry*> actors = totg.entries_by_type("ACTR");
		std::vector<ChunkEntry*> pots;
		for (ChunkEntry* actor : actors) {
			if (std::strncmp(&actor->data[0], "kotubo\x00\x00", 8) == 0) pots.push_back(actor);
		}
		pots[1]->data = std::string("\x6B\x6F\x74\x75\x62\x6F\x00\x00\x70\x7F\xFF\x0A\xC5\x6E\x20\x00\x43\x66\x00\x05\xC5\xDF\xC0\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF", 0x20);
		totg.writeToFile(path.string());
	}
}

void modify_title_screen() {
	using namespace NintendoWare::Layout;

	RandoSession::fspath path = g_session.openGameFile("content/Common/Layout/Title_00.szs@YAZ0@SARC@blyt/Title_00.bflyt");

	FileTypes::FLYTFile layout;
	layout.loadFromFile(path.string());

	//add version number
	Pane& newPane = layout.rootPane.children[0].children[1].children[3].duplicateChildPane(1); //unused version number text
	newPane.pane->name = "T_Version";
	newPane.pane->name.resize(0x18);
	dynamic_cast<txt1*>(newPane.pane.get())->text = std::u16string(u"Ver 01.00.00a\0", 0xE);
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

	layout.writeToFile(path.string());

	//update title screen stuff
	g_session.copyToGameFile("./assets/Title.bflim", "content/Common/Layout/Title_00.szs@YAZ0@SARC@timg/TitleLogoZelda_00^l.bflim");

	g_session.copyToGameFile("./assets/Subtitle.bflim", "content/Common/Layout/Title_00.szs@YAZ0@SARC@timg/TitleLogoWindwaker_00^l.bflim");

	g_session.copyToGameFile("./assets/Subtitle_Mask.bflim", "content/Common/Layout/Title_00.szs@YAZ0@SARC@timg/TitleLogoWindwakerMask_00^s.bflim");

	//update sparkle size/position
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x101f7048), 0x3fb33333); //scale
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x101f7044), 0x40100000); //possibly particle size, JP changes it for its larger title text
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x10108280), 0xc2180000); //vertical position
	return;
}

void update_name_and_icon() {
	g_session.copyToGameFile("./assets/iconTex.tga", "meta/iconTex.tga");

	tinyxml2::XMLDocument meta;
	std::string metaPath = g_session.openGameFile("meta/meta.xml").string();
	meta.LoadFile(metaPath.c_str());
	tinyxml2::XMLElement* root = meta.RootElement();
	root->FirstChildElement("longname_en")->SetText("THE LEGEND OF ZELDA\nThe Wind Waker HD Randomizer");
	root->FirstChildElement("longname_fr")->SetText("THE LEGEND OF ZELDA\nThe Wind Waker HD Randomizer");
	root->FirstChildElement("longname_es")->SetText("THE LEGEND OF ZELDA\nThe Wind Waker HD Randomizer");
	root->FirstChildElement("longname_pt")->SetText("THE LEGEND OF ZELDA\nThe Wind Waker HD Randomizer");

	root->FirstChildElement("shortname_en")->SetText("The Wind Waker HD Randomizer");
	root->FirstChildElement("shortname_fr")->SetText("The Wind Waker HD Randomizer");
	root->FirstChildElement("shortname_es")->SetText("The Wind Waker HD Randomizer");
	root->FirstChildElement("shortname_pt")->SetText("The Wind Waker HD Randomizer");

	meta.SaveFile(metaPath.c_str());
	return;
}

void allow_dungeon_items_to_appear_anywhere() {
	uint32_t item_get_func_pointer = 0x0001da54; //First relevant relocation entry in .rela.data (overwrites .data section when loaded)
	uint32_t item_resources_list_start = 0x101e4674;
	uint32_t field_item_resources_list_start = 0x101e6a74;

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
		{0x15, 0x1004e448},
		{0x4C, 0x1004e4b8},
		{0x4D, 0x1004e4b0},
		{0x4E, 0x1004e698}
	};

	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");

	FileTypes::MSBTFile msbt;
	msbt.loadFromFile(path.string());
	for (const dungeon_item_info& item_data : dungeon_items) {
		std::u16string item_name = item_data.short_name + u" " + item_data.base_item_name;
		uint8_t base_item_id = item_name_to_id.at(item_data.base_item_name);
		std::u16string dungeon_name = dungeon_names.at(item_data.short_name);

		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_get_func_pointer + 0x8 + (0xC * item_data.item_id)), custom_symbols.at(idToFunc.at(item_data.item_id)) - 0x02000000); //write to the relocation entries
		
		int message_id = 101 + item_data.item_id;
		Message& to_copy = msbt.messages_by_label["00" + std::to_string(101 + base_item_id)];
		if (item_data.base_item_name == u"Small Key") {
			std::u16string article = get_indefinite_article(dungeon_name); //this is to avoid a duplicate function call
			std::u16string message(DRAW_INSTANT + u"You got " + article + u" " + TEXT_COLOR_RED + dungeon_name + u" small key" + TEXT_COLOR_DEFAULT + u"!", 39 + article.size() + dungeon_name.size()); //calculate string length for initalizer to handle null characters inside string
			//word_wrap_string
			msbt.addMessage("00" + std::to_string(message_id), to_copy.attributes, to_copy.style, message);
		} 
		else if (item_data.base_item_name == u"Big Key") {
			std::u16string message(DRAW_INSTANT + u"You got the " + TEXT_COLOR_RED + dungeon_name + u" Big Key" + TEXT_COLOR_DEFAULT + u"!", 40 + dungeon_name.size()); //calculate string length for initalizer to handle null characters inside string
			//word_wrap_string
			msbt.addMessage("00" + std::to_string(message_id), to_copy.attributes, to_copy.style, message);
		}
		else if (item_data.base_item_name == u"Dungeon Map") {
			std::u16string message(DRAW_INSTANT + u"You got the " + TEXT_COLOR_RED + dungeon_name + u" Dungeon Map" + TEXT_COLOR_DEFAULT + u"!", 44 + dungeon_name.size()); //calculate string length for initalizer to handle null characters inside string
			//word_wrap_string
			msbt.addMessage("00" + std::to_string(message_id), to_copy.attributes, to_copy.style, message);
		}
		else if (item_data.base_item_name == u"Compass") {
			std::u16string message(DRAW_INSTANT + u"You got the " + TEXT_COLOR_RED + dungeon_name + u" Compass" + TEXT_COLOR_DEFAULT + u"!", 40 + dungeon_name.size()); //calculate string length for initalizer to handle null characters inside string
			//word_wrap_string
			msbt.addMessage("00" + std::to_string(message_id), to_copy.attributes, to_copy.style, message);
		}

		uint32_t item_resources_addr_to_copy_from = item_resources_list_start + base_item_id * 0x24;
		uint32_t field_item_resources_addr_to_copy_from = field_item_resources_list_start + base_item_id * 0x1c;

		uint32_t item_resources_addr = item_resources_list_start + item_data.item_id * 0x24;
		uint32_t field_item_resources_addr = item_resources_list_start + item_data.item_id * 0x24;

		uint32_t szs_name_pointer = szs_name_pointers.at(base_item_id);

		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr), szs_name_pointer);
		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr), szs_name_pointer);

		//need relocation entries so pointers work on console
		Elf32_Rela relocation;
		relocation.r_offset = field_item_resources_addr;
		relocation.r_info = 0x00000201;
		relocation.r_addend = szs_name_pointer - 0x10000000; //needs offset into .rodata section, subtract start address from data location

		std::string entry;
		entry.resize(12);
		entry.replace(0, 4, reinterpret_cast<const char*>(&relocation.r_offset), 4);
		entry.replace(4, 4, reinterpret_cast<const char*>(&relocation.r_info), 4);
		entry.replace(8, 4, reinterpret_cast<const char*>(&relocation.r_addend), 4);
		gRPX.extend_section(9, entry);

		Elf32_Rela relocation2;
		relocation2.r_offset = item_resources_addr;
		relocation2.r_info = relocation.r_info; //same as first entry
		relocation2.r_addend = relocation.r_addend; //same as first entry

		std::string entry2;
		entry2.resize(12);
		entry2.replace(0, 4, reinterpret_cast<const char*>(&relocation2.r_offset), 4);
		entry2.replace(4, 4, reinterpret_cast<const char*>(&relocation2.r_info), 4);
		entry2.replace(8, 4, reinterpret_cast<const char*>(&relocation2.r_addend), 4);
		gRPX.extend_section(9, entry2);

		std::vector<uint8_t> data1 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_copy_from + 8), 0xD);
		elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr + 8), data1);
		elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr + 4), data1);
		std::vector<uint8_t> data2 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_copy_from + 0x1C), 4);
		elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr + 0x1C), data2);
		elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr + 0x14), data2);

		std::vector<uint8_t> data3 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_copy_from + 0x15), 0x7);
		elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr + 0x15), data3);
		std::vector<uint8_t> data4 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr_to_copy_from + 0x20), 0x4);
		elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, item_resources_addr + 0x20), data4);

		std::vector<uint8_t> data5 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr_to_copy_from + 0x11), 0x3);
		elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr + 0x11), data5);
		std::vector<uint8_t> data6 = elfUtil::read_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr_to_copy_from + 0x18), 0x4);
		elfUtil::write_bytes(gRPX, elfUtil::AddressToOffset(gRPX, field_item_resources_addr + 0x18), data6);
		
	}
	msbt.writeToFile(path.string());

	return;
}

void remove_bog_warp_in_cs() {
	for (int i = 1; i < 50; i++) {
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
		FileTypes::DZXFile room_dzr;
		room_dzr.loadFromFile(filePath.string());
		for (ChunkEntry* spawn : room_dzr.entries_by_type("PLYR")) {
			uint8_t spawn_type = ((*reinterpret_cast<uint8_t*>(&spawn->data[0xB]) & 0xF0) >> 4);
			if (spawn_type == 0x09) {
				spawn->data[0xB] = (spawn->data[0xB] & 0x0F) | 0x20;
			}
		}
		room_dzr.writeToFile(filePath.string());
	}
}

void fix_shop_item_y_offsets() {
	uint32_t shop_item_display_data_list_start = 0x1003a930;

	for (uint8_t id = 0; id < 0xFF; id++) {
		uint32_t display_data_addr = shop_item_display_data_list_start + id * 0x20;
		float y_offset = elfUtil::read_float(gRPX, elfUtil::AddressToOffset(gRPX, display_data_addr + 0x10));
		uint8_t ArrowID[] = { 0x10, 0x11, 0x12 };

		if (y_offset == 0 && std::find(std::begin(ArrowID), std::end(ArrowID), id) == std::end(ArrowID)) {
			//If the item didn't originally have a Y offset we need to give it one so it's not sunken into the pedestal.
			// Only exception are for items 10 11 and 12 - arrow refill pickups.Those have no Y offset but look fine already.
			float new_y_offset = 20.0f;
			elfUtil::write_float(gRPX, elfUtil::AddressToOffset(gRPX, display_data_addr + 0x10), new_y_offset);
		}
	}
}

void update_shop_item_descriptions(const GameItem& beedle20Item, const GameItem& beedle500Item, const GameItem& beedle950Item, const GameItem& beedle900Item) {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt");

	FileTypes::MSBTFile msbt;
	msbt.loadFromFile(path.string());
	
	msbt.messages_by_label["03906"].text.message = TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle20Item)) + u"  20 Rupees" + TEXT_COLOR_DEFAULT;
	msbt.messages_by_label["03909"].text.message = Utility::Str::toUTF16(gameItemToName(beedle20Item)) + u"  20 Rupees\nWill you buy it?\n" + TWO_CHOICES + u"I'll buy it\nNo thanks";

	msbt.writeToFile(path.string());

	path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message4.msbt");

	FileTypes::MSBTFile msbt2;
	msbt2.loadFromFile(path.string());

	msbt2.messages_by_label["12106"].text.message = TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle500Item)) + u"  500 Rupees\n" + TEXT_COLOR_DEFAULT + u"This is my last one.";
	msbt2.messages_by_label["12109"].text.message = u"This " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle500Item)) + TEXT_COLOR_DEFAULT + u" is a mere " + TEXT_COLOR_RED + u"500 Rupees" + TEXT_COLOR_DEFAULT + u"!\nBuy it! Buy it! Buy buy buy!\n" + TWO_CHOICES + u"I'll buy it\nNo thanks";

	msbt2.messages_by_label["12107"].text.message = TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle950Item)) + u"  950 Rupees\n" + TEXT_COLOR_DEFAULT + u"This is my last one of these, too.";
	msbt2.messages_by_label["12110"].text.message = u"This " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle950Item)) + TEXT_COLOR_DEFAULT + u" is only " + TEXT_COLOR_RED + u"950 Rupees" + TEXT_COLOR_DEFAULT + u"!\nBuy it! Buy it! Buy buy buy!\n" + TWO_CHOICES + u"I'll buy it\nNo thanks";

	msbt2.messages_by_label["12108"].text.message = TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle900Item)) + u"  900 Rupees\n" + TEXT_COLOR_DEFAULT + u"The price may be high, but it'll pay\noff handsomely in the end!";
	msbt2.messages_by_label["12111"].text.message = u"This " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle900Item)) + TEXT_COLOR_DEFAULT + u" is just " + TEXT_COLOR_RED + u"900 Rupees" + TEXT_COLOR_DEFAULT + u"!\nBuy it! Buy it! Buy buy buy!\n" + TWO_CHOICES + u"I'll buy it\nNo thanks";

	msbt2.writeToFile(path.string());
	return;
}

void update_auction_item_names(const GameItem& auction5, const GameItem& auction40, const GameItem& auction60, const GameItem& auction80, const GameItem& auction100) {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message3_msbt.szs@YAZ0@SARC@message3.msbt");

	FileTypes::MSBTFile msbt;
	msbt.loadFromFile(path.string());

	msbt.messages_by_label["07440"].text.message = TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(auction40)) + TEXT_COLOR_DEFAULT;
	msbt.messages_by_label["07441"].text.message = TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(auction5)) + TEXT_COLOR_DEFAULT;
	msbt.messages_by_label["07442"].text.message = TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(auction60)) + TEXT_COLOR_DEFAULT;
	msbt.messages_by_label["07443"].text.message = TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(auction80)) + TEXT_COLOR_DEFAULT;
	msbt.messages_by_label["07444"].text.message = TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(auction100)) + TEXT_COLOR_DEFAULT;

	msbt.writeToFile(path.string());

	//also add a hint to the flyer explaining what items the auction holds
	RandoSession::fspath path2 = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");
	FileTypes::MSBTFile msbt2;
	msbt2.loadFromFile(path2.string());

	msbt2.messages_by_label["00804"].text.message.pop_back(); //remove null terminator, we want to add things before it
	msbt2.messages_by_label["00804"].text.message += u"\n\nParticipate for the chance to win ";
	std::u16string itemStr = std::u16string(TEXT_COLOR_RED, 5);
	if (gameItemToName(auction5).find("Treasure Chart") != std::string::npos) {
		itemStr += std::u16string(u"Treasure Chart" + TEXT_COLOR_DEFAULT + u", ", 26);
	}
	else if (gameItemToName(auction5).find("Triforce Chart") != std::string::npos) {
		itemStr += std::u16string(u"Triforce Chart" + TEXT_COLOR_DEFAULT + u", ", 26);
	}
	else {
		itemStr += Utility::Str::toUTF16(gameItemToName(auction5)) + std::u16string(TEXT_COLOR_DEFAULT + u", ", 12);
	}

	if (gameItemToName(auction40).find("Treasure Chart") != std::string::npos) {
		itemStr += std::u16string(TEXT_COLOR_RED + u"Treasure Chart" + TEXT_COLOR_DEFAULT + u", ", 31);
	}
	else if (gameItemToName(auction40).find("Triforce Chart") != std::string::npos) {
		itemStr += std::u16string(TEXT_COLOR_RED + u"Triforce Chart" + TEXT_COLOR_DEFAULT + u", ", 31);
	}
	else {
		itemStr += std::u16string(TEXT_COLOR_RED, 5) + Utility::Str::toUTF16(gameItemToName(auction40)) + std::u16string(TEXT_COLOR_DEFAULT + u", ", 12);
	}

	if (gameItemToName(auction60).find("Treasure Chart") != std::string::npos) {
		itemStr += std::u16string(TEXT_COLOR_RED + u"Treasure Chart" + TEXT_COLOR_DEFAULT + u", ", 31);
	}
	else if (gameItemToName(auction60).find("Triforce Chart") != std::string::npos) {
		itemStr += std::u16string(TEXT_COLOR_RED + u"Triforce Chart" + TEXT_COLOR_DEFAULT + u", ", 31);
	}
	else {
		itemStr += std::u16string(TEXT_COLOR_RED, 5) + Utility::Str::toUTF16(gameItemToName(auction60)) + std::u16string(TEXT_COLOR_DEFAULT + u", ", 12);
	}

	if (gameItemToName(auction80).find("Treasure Chart") != std::string::npos) {
		itemStr += std::u16string(TEXT_COLOR_RED + u"Treasure Chart" + TEXT_COLOR_DEFAULT + u", ", 31);
	}
	else if (gameItemToName(auction80).find("Triforce Chart") != std::string::npos) {
		itemStr += std::u16string(TEXT_COLOR_RED + u"Triforce Chart" + TEXT_COLOR_DEFAULT + u", ", 31);
	}
	else {
		itemStr += std::u16string(TEXT_COLOR_RED, 5) + Utility::Str::toUTF16(gameItemToName(auction80)) + std::u16string(TEXT_COLOR_DEFAULT + u", ", 12);
	}

	if (gameItemToName(auction100).find("Treasure Chart") != std::string::npos) {
		itemStr += std::u16string(u"or " + TEXT_COLOR_RED + u"Treasure Chart" + TEXT_COLOR_DEFAULT + u"!", 33);
	}
	else if (gameItemToName(auction100).find("Triforce Chart") != std::string::npos) {
		itemStr += std::u16string(u"or " + TEXT_COLOR_RED + u"Triforce Chart" + TEXT_COLOR_DEFAULT + u"!", 33);
	}
	else {
		itemStr += std::u16string(u"or " + TEXT_COLOR_RED, 8) + Utility::Str::toUTF16(gameItemToName(auction100)) + std::u16string(TEXT_COLOR_DEFAULT + u"!", 11);
	}

	msbt2.messages_by_label["00804"].text.message += word_wrap_string(itemStr, 44);
	msbt2.messages_by_label["00804"].text.message += u'\0'; //add null terminator
	msbt2.writeToFile(path2.string());

	return;
}

void update_battlesquid_item_names(const GameItem& firstPrize, const GameItem& secondPrize) {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message3.msbt");

	FileTypes::MSBTFile msbt;
	msbt.loadFromFile(path.string());

	msbt.messages_by_label["07520"].text.message = SOUND(0x8E) + u"Hoorayyy! Yayyy! Yayyy!\nOh, thank you, Mr. Sailor!\n\n\n" + word_wrap_string(u"Please take this " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(firstPrize)) + TEXT_COLOR_DEFAULT + u" as a sign of our gratitude.You are soooooo GREAT!", 43);
	msbt.messages_by_label["07521"].text.message = SOUND(0x8E) + u"Hoorayyy! Yayyy! Yayyy!\nOh, thank you so much, Mr. Sailor!\n\n\n" + word_wrap_string(u"This is our thanks to you! It's been passed down on our island for many years, so don't tell the island elder, OK? Here..." + TEXT_COLOR_RED + IMAGE(ImageTags::HEART) + TEXT_COLOR_DEFAULT + u"Please accept this " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(secondPrize)) + TEXT_COLOR_DEFAULT + u"!", 43);

	msbt.writeToFile(path.string());
	return;
}

void update_item_names_in_letter_advertising_rock_spire_shop(const GameItem& beedle500Item, const GameItem& beedle950Item, const GameItem& beedle900Item) {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt");

	FileTypes::MSBTFile msbt;
	msbt.loadFromFile(path.string());

	std::u16string stringBefore = msbt.messages_by_label["03325"].text.message.substr(0, 194);
	std::u16string stringAfter = msbt.messages_by_label["03325"].text.message.substr(396, 323);
	std::u16string hintString = u"Do you have need of " + get_indefinite_article(Utility::Str::toUTF16(gameItemToName(beedle500Item))) + u" " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle500Item)) + TEXT_COLOR_DEFAULT + u", " + Utility::Str::toUTF16(get_indefinite_article(gameItemToName(beedle950Item))) + u" " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle950Item)) + TEXT_COLOR_DEFAULT + u", or " + get_indefinite_article(Utility::Str::toUTF16(gameItemToName(beedle900Item))) + u" " + TEXT_COLOR_RED + Utility::Str::toUTF16(gameItemToName(beedle900Item)) + TEXT_COLOR_DEFAULT + u"? We have them at special bargain prices.";
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

	msbt.writeToFile(path.string());
	return;
}

void update_savage_labyrinth_hint_tablet(const GameItem& floor30, const GameItem& floor50) {
	//https://github.com/LagoLunatic/wwrando/blob/master/tweaks.py#L843
}

//hints

void shorten_zephos_event() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@event_list.dat");

	FileTypes::EventList event_list;
	event_list.loadFromFile(path.string());
	Event& wind_shrine_event = event_list.Events_By_Name["TACT_HT"];


	std::optional<std::reference_wrapper<Actor>> actor = wind_shrine_event.get_actor("Hr");
	if (!actor.has_value()) {
		return;
	}
	Actor& zephos = actor.value(); //set references after error checking instead of using .value() every time a member is accessed

	actor = wind_shrine_event.get_actor("Link");
	if (!actor.has_value()) {
		return;
	}
	Actor& link = actor.value(); //same as zephos

	actor = wind_shrine_event.get_actor("CAMERA");
	if (!actor.has_value()) {
		return;
	}
	Actor& camera = actor.value(); //same as zephos

	zephos.actions.erase(zephos.actions.begin() + 6, zephos.actions.end());
	link.actions.erase(link.actions.begin() + 6, link.actions.end());
	camera.actions.erase(camera.actions.begin() + 4, camera.actions.end());
	wind_shrine_event.ending_flags = {
		zephos.actions.back().flag_id_to_set,
		link.actions.back().flag_id_to_set,
		camera.actions.back().flag_id_to_set,
	};

	event_list.writeToFile(path.string());

	return;
}

void update_korl_dialog() {
	const RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt");

	FileTypes::MSBTFile msbt;
	msbt.loadFromFile(path.string());
	msbt.messages_by_label["03443"].text.message = std::u16string(CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", the sea is all yours.\nMake sure you explore every corner\nin search of items to help you.Remember\nthat your quest is to defeat Ganondorf.\0", 144); //need to use constructor with length because of null characters
	msbt.writeToFile(path.string());

	return;
}

void set_num_starting_triforce_shards(const uint8_t numShards) {
	uint32_t num_shards_address = custom_symbols.at("num_triforce_shards_to_start_with");
	elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, num_shards_address), numShards);
	return;
}

void set_starting_health(const uint16_t heartPieces, const uint16_t heartContainers) {
	uint16_t base_health = 12;

	uint16_t starting_health = base_health + (heartContainers * 4) + heartPieces;

	uint32_t starting_quarter_hearts_address = custom_symbols.at("starting_quarter_hearts");

	elfUtil::write_u16(gRPX, elfUtil::AddressToOffset(gRPX, starting_quarter_hearts_address), starting_health);
	return;
}

void give_double_magic() {
	uint32_t starting_magic_address = custom_symbols.at("starting_magic");
	elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, starting_magic_address), 32);
	return;
}

void set_damage_multiplier(const float multiplier) {
	uint32_t damage_multiplier_address = custom_symbols.at("custom_damage_multiplier");
	elfUtil::write_float(gRPX, elfUtil::AddressToOffset(gRPX, damage_multiplier_address), multiplier);
	return;
}

void set_pig_color(const PigColor color) {
	uint32_t pig_color_address = custom_symbols.at("outset_pig_color");
	elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, pig_color_address), static_cast<std::underlying_type_t<PigColor>>(color));
}

void add_pirate_ship_to_windfall() {
	RandoSession::fspath windfallPath = g_session.openGameFile("content/Common/Pack/szs_permanent1.pack@SARC@sea_Room11.szs@YAZ0@SARC@Room11.bfres@BFRES@room.dzr");
	RandoSession::fspath shipRoomPath = g_session.openGameFile("content/Common/Stage/Asoko_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr");

	FileTypes::DZXFile windfallDzr;
	FileTypes::DZXFile shipDzr;

	windfallDzr.loadFromFile(windfallPath.string());
	shipDzr.loadFromFile(shipRoomPath.string());

	std::vector<ChunkEntry*> wf_layer_2_actors = windfallDzr.entries_by_type_and_layer("ACTR", 2);
	ChunkEntry* layer_2_ship = nullptr;
	for (ChunkEntry* actor : wf_layer_2_actors) {
		if (std::strncmp(&actor->data[0], "Pirates\x00", 8) == 0) layer_2_ship = actor;
	}

	ChunkEntry& default_layer_ship = windfallDzr.add_entity("ACTR", 2);
	default_layer_ship.data = layer_2_ship->data;
	default_layer_ship.data[0x10] = '\x00';

	windfallDzr.writeToFile(windfallPath.string());

	for (const int layer_num : {2, 3}) {
		std::vector<ChunkEntry*> actors = shipDzr.entries_by_type_and_layer("ACTR", layer_num);
		for (ChunkEntry* actor : actors) {
			if (std::strncmp(&actor->data[0], "P2b\x00\x00\x00\x00\x00", 8) == 0) shipDzr.remove_entity(actor);
		}
	}

	ChunkEntry& aryll = shipDzr.add_entity("ACTR");
	aryll.data = std::string("Ls1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x44\x16\x00\x00\xC4\x09\x80\x00\xC3\x48\x00\x00\x00\x00\xC0\x00\x00\x00\xFF\xFF", 0x20);

	RandoSession::fspath msbtPath = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");

	FileTypes::MSBTFile msbt;
	msbt.loadFromFile(msbtPath.string());

	msbt.messages_by_label["03008"].attributes.soundEffect = 106;
	msbt.messages_by_label["03008"].text.message = u"'Hoy! Big Brother!\n";
	msbt.messages_by_label["03008"].text.message += u"Wanna play a game? It's fun, trust me!";
	msbt.messages_by_label["03008"].text.message  = pad_str_4_lines(msbt.messages_by_label["03008"].text.message);
	msbt.messages_by_label["03008"].text.message += word_wrap_string(std::u16string(u"Just " + TEXT_COLOR_RED + u"step on this button" + TEXT_COLOR_DEFAULT + u", and try to swing across the ropes to reach that door over there before time's up!\0", 123), 44);

	uint32_t stage_bgm_info_list_start = 0x1018e428;
	uint32_t second_dynamic_scene_waves_list_start = 0x1018e2ec;
	uint8_t asoko_spot_id = 0xC;
	uint8_t new_second_scene_wave_index = 0xE;
	uint8_t isle_link_0_aw_index = 0x19;

	uint32_t asoko_bgm_info_ptr = stage_bgm_info_list_start + asoko_spot_id * 0x4;
	uint32_t new_second_scene_wave_ptr = second_dynamic_scene_waves_list_start + new_second_scene_wave_index * 2;
	elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, asoko_bgm_info_ptr + 3), new_second_scene_wave_index);
	elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, new_second_scene_wave_ptr), isle_link_0_aw_index);

	shipDzr.writeToFile(shipRoomPath.string());
}

void add_cross_dungeon_warps() {
	std::array<CyclicWarpPotData, 3> loop1{ { {"M_NewD2", 2, 2185.0f, 0.0f, 590.0f, 0xA000, 2}, {"kindan", 1, 986.0f, 3956.43f, 9588.0f, 0xB929, 2}, {"Siren", 6, 277.0f, 229.42f, -6669.0f, 0xC000, 2} } };
	std::array<CyclicWarpPotData, 3> loop2{ { {"ma2room", 2, 1556.0f, 728.46f, -7091.0f, 0xEAA6, 5}, {"M_Dai", 1, -8010.0f, 1010.0f, -1610.0f, 0, 5}, {"kaze", 3, -4333.0f, 1100.0f, 48.0f, 0x4000, 5} } };

	uint8_t warp_index = 0;
	for (CyclicWarpPotData& warp : loop1) {
		RandoSession::fspath stagePath = g_session.openGameFile("content/Common/Stage/" + warp.stage_name + "_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs");
		FileTypes::DZXFile stage;
		stage.loadFromFile(stagePath.string());

		RandoSession::fspath roomPath = g_session.openGameFile("content/Common/Stage/" + warp.stage_name + "_Room" + std::to_string(warp.room_num) + ".szs@YAZ0@SARC@Room" + std::to_string(warp.room_num) + ".bfres@BFRES@room.dzr");
		FileTypes::DZXFile room;
		room.loadFromFile(roomPath.string());

		FileTypes::DZXFile* dzx_for_spawn;
		dzx_for_spawn = &room;

		Utility::Endian::toPlatform_inplace(eType::Big, warp.x);
		Utility::Endian::toPlatform_inplace(eType::Big, warp.y);
		Utility::Endian::toPlatform_inplace(eType::Big, warp.z);
		Utility::Endian::toPlatform_inplace(eType::Big, warp.y_rot);

		ChunkEntry& spawn = dzx_for_spawn->add_entity("PLYR");
		spawn.data = std::string("Link\x00\x00\x00\x00\xFF\xFF\x70", 0xC);
		spawn.data.resize(0x20);
		spawn.data[0xB] = (spawn.data[0xB] & ~0x3F) | (warp.room_num & 0x3F);
		spawn.data.replace(0xC, 1, reinterpret_cast<const char*>(&warp.x), 4);
		spawn.data.replace(0x10, 1, reinterpret_cast<const char*>(&warp.y), 4);
		spawn.data.replace(0x14, 1, reinterpret_cast<const char*>(&warp.z), 4);
		spawn.data.replace(0x18, 1, "\x00\x00", 2);
		spawn.data.replace(0x1A, 1, reinterpret_cast<const char*>(&warp.y_rot), 2);
		spawn.data.replace(0x1C, 1, "\xFF\x45\xFF\xFF", 4);

		std::vector<ChunkEntry*> spawns = dzx_for_spawn->entries_by_type("PLYR");
		std::vector<ChunkEntry*> spawn_id_69;
		for (ChunkEntry* spawn_to_check : spawns) {
			if (std::strncmp(&spawn_to_check->data[0x1D], "\x45", 1) == 0) spawn_id_69.push_back(spawn_to_check);
		}
		if (spawn_id_69.size() != 1) return; //should always be 1

		std::vector<uint8_t> pot_index_to_exit;
		for (const CyclicWarpPotData& other_warp : loop1) {
			ChunkEntry& scls_exit = room.add_entity("SCLS");
			std::string dest_stage = other_warp.stage_name;
			dest_stage.resize(8, '\x00');
			scls_exit.data.resize(0xC);
			scls_exit.data.replace(0, 8, dest_stage);
			scls_exit.data.replace(8, 1, "\x45", 1);
			scls_exit.data.replace(0x9, 1, reinterpret_cast<const char*>(&other_warp.room_num), 1);
			scls_exit.data.replace(0xA, 2, "\x04\xFF", 2);
			pot_index_to_exit.push_back(room.entries_by_type("SCLS").size() - 1);
		}

		uint32_t params = 0x00000000;
		params = (params & ~0xFF000000) | (pot_index_to_exit[0] & 0xFF000000);
		params = (params & ~0x00FF0000) | (pot_index_to_exit[1] & 0x00FF0000);
		params = (params & ~0x0000FF00) | (pot_index_to_exit[2] & 0x0000FF00);
		params = (params & ~0x000000F0) | (warp.event_reg_index & 0x000000F0);
		params = (params & ~0x0000000F) | ((warp_index + 1) & 0x0000000F);

		ChunkEntry& warp_pot = room.add_entity("ACTR");
		warp_pot.data = "Warpts" + std::to_string(warp_index + 1);
		warp_pot.data.resize(0x20);
		warp_pot.data.replace(0x8, 4, reinterpret_cast<const char*>(&params), 4);
		warp_pot.data.replace(0xC, 4, reinterpret_cast<const char*>(&warp.x), 4);
		warp_pot.data.replace(0x10, 4, reinterpret_cast<const char*>(&warp.y), 4);
		warp_pot.data.replace(0x14, 4, reinterpret_cast<const char*>(&warp.z), 4);
		warp_pot.data.replace(0x18, 2, "\xFF\xFF", 4);
		warp_pot.data.replace(0x1A, 2, reinterpret_cast<const char*>(&warp.y_rot), 4);
		warp_pot.data.replace(0x1C, 4, "\xFF\xFF\xFF\xFF", 4);

		room.writeToFile(roomPath.string());
		stage.writeToFile(stagePath.string());

		warp_index++;
	}

	warp_index = 0;
	for (CyclicWarpPotData& warp : loop2) {
		RandoSession::fspath stagePath = g_session.openGameFile("content/Common/Stage/" + warp.stage_name + "_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs");
		FileTypes::DZXFile stage;
		stage.loadFromFile(stagePath.string());

		RandoSession::fspath roomPath = g_session.openGameFile("content/Common/Stage/" + warp.stage_name + "_Room" + std::to_string(warp.room_num) + ".szs@YAZ0@SARC@Room" + std::to_string(warp.room_num) + ".bfres@BFRES@room.dzr");
		FileTypes::DZXFile room;
		room.loadFromFile(roomPath.string());

		FileTypes::DZXFile* dzx_for_spawn;
		if (warp.stage_name == "M_Dai" || warp.stage_name == "kaze") {
			dzx_for_spawn = &stage;
		}
		else {
			dzx_for_spawn = &room;
		}

		Utility::Endian::toPlatform_inplace(eType::Big, warp.x);
		Utility::Endian::toPlatform_inplace(eType::Big, warp.y);
		Utility::Endian::toPlatform_inplace(eType::Big, warp.z);
		Utility::Endian::toPlatform_inplace(eType::Big, warp.y_rot);

		ChunkEntry& spawn = dzx_for_spawn->add_entity("PLYR");
		spawn.data = std::string("Link\x00\x00\x00\x00\xFF\xFF\x70", 0xC);
		spawn.data.resize(0x20);
		spawn.data[0xB] = (spawn.data[0xB] & ~0x3F) | (warp.room_num & 0x3F);
		spawn.data.replace(0xC, 1, reinterpret_cast<const char*>(&warp.x), 4);
		spawn.data.replace(0x10, 1, reinterpret_cast<const char*>(&warp.y), 4);
		spawn.data.replace(0x14, 1, reinterpret_cast<const char*>(&warp.z), 4);
		spawn.data.replace(0x18, 1, "\x00\x00", 2);
		spawn.data.replace(0x1A, 1, reinterpret_cast<const char*>(&warp.y_rot), 2);
		spawn.data.replace(0x1C, 1, "\xFF\x45\xFF\xFF", 4);

		std::vector<ChunkEntry*> spawns = dzx_for_spawn->entries_by_type("PLYR");
		std::vector<ChunkEntry*> spawn_id_69;
		for (ChunkEntry* spawn_to_check : spawns) {
			if (std::strncmp(&spawn_to_check->data[0x1D], "\x45", 1) == 0) spawn_id_69.push_back(spawn_to_check);
		}
		if (spawn_id_69.size() != 1) return; //should always be 1

		std::vector<uint8_t> pot_index_to_exit;
		for (const CyclicWarpPotData& other_warp : loop1) {
			ChunkEntry& scls_exit = room.add_entity("SCLS");
			std::string dest_stage = other_warp.stage_name;
			dest_stage.resize(8, '\x00');
			scls_exit.data.resize(0xC);
			scls_exit.data.replace(0, 8, dest_stage);
			scls_exit.data.replace(8, 1, "\x45", 1);
			scls_exit.data.replace(0x9, 1, reinterpret_cast<const char*>(&other_warp.room_num), 1);
			scls_exit.data.replace(0xA, 2, "\x04\xFF", 2);
			pot_index_to_exit.push_back(room.entries_by_type("SCLS").size() - 1);
		}

		uint32_t params = 0x00000000;
		params = (params & ~0xFF000000) | (pot_index_to_exit[0] & 0xFF000000);
		params = (params & ~0x00FF0000) | (pot_index_to_exit[1] & 0x00FF0000);
		params = (params & ~0x0000FF00) | (pot_index_to_exit[2] & 0x0000FF00);
		params = (params & ~0x000000F0) | (warp.event_reg_index & 0x000000F0);
		params = (params & ~0x0000000F) | ((warp_index + 1) & 0x0000000F);

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

		room.writeToFile(roomPath.string());
		stage.writeToFile(stagePath.string());

		warp_index++;
	}

	FileTypes::JPC drc, totg, ff;
	RandoSession::fspath drcPath = g_session.openGameFile("content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@BFRES@Pscene035.jpc");

	RandoSession::fspath totgPath = g_session.openGameFile("content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@BFRES@Pscene050.jpc");

	RandoSession::fspath ffPath = g_session.openGameFile("content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@BFRES@Pscene043.jpc");

	drc.loadFromFile(drcPath.string());
	totg.loadFromFile(totgPath.string());
	ff.loadFromFile(ffPath.string());

	for (const uint16_t particle_id : {0x8161, 0x8162, 0x8165, 0x8166, 0x8112}) {
		Particle& particle = drc.particles[drc.particle_index_by_id[particle_id]];

		totg.addParticle(particle);
		ff.addParticle(particle);

		for (const std::string& textureFilename : particle.texDatabase.value().texFilenames) {
			if (totg.textures.find(textureFilename) == totg.textures.end()) {
				totg.addTexture(textureFilename);
			}

			if (ff.textures.find(textureFilename) == ff.textures.end()) {
				ff.addTexture(textureFilename);
			}
		}
	}
	
	totg.writeToFile(totgPath.string());
	ff.writeToFile(ffPath.string());

	return;
}

void remove_makar_kidnapping() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/kaze_Room3.szs@YAZ0@SARC@Room3.bfres@BFRES@room.dzr");

	FileTypes::DZXFile dzr;
	dzr.loadFromFile(path.string());
	std::vector<ChunkEntry*> actors = dzr.entries_by_type("ACTR");

	ChunkEntry* switch_actor = nullptr; //initialization is just to make compiler happy
	for (ChunkEntry* actor : actors) {
		if (std::strncmp(&actor->data[0], "AND_SW2\x00", 8) == 0) switch_actor = actor;
	}
	dzr.remove_entity(switch_actor);

	for (ChunkEntry* actor : actors) {
		if (std::strncmp(&actor->data[0], "wiz_r\x00\x00\x00", 8) == 0) {
			actor->data.replace(0xA, 1, "\xFF", 1);
		}
	}

	dzr.writeToFile(path.string());
}

void increase_crawl_speed() {
	//The 3.0 float crawling uses is shared with other things in HD, can't change it directly
	//Redirect both instances to load 6.0 from elsewhere
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0014ec04, 7), 0x000355C4); //update .rela.text entry
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0014ec4c, 7), 0x000355C4); //update .rela.text entry
	return;
}

void add_chart_number_to_item_get_messages() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");

	FileTypes::MSBTFile msbt;
	msbt.loadFromFile(path.string());
	for (uint8_t item_id = 0xCC; item_id < 0xFF; item_id++) {
		if (item_id == 0xDB || item_id == 0xDC) continue; //skip ghost ship chart and tingle's chart
		
		std::u16string itemName = Utility::Str::toUTF16(gameItemToName(idToGameItem(item_id)));
		msbt.messages_by_label["00" + std::to_string(101 + item_id)].text.message.replace(12, 21, itemName);
	}
	msbt.writeToFile(path.string());

	return;
}

void increase_grapple_animation_speed() {
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x02170250), 0x394B000A);
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x00075170, 7), 0x00010ffc); //update .rela.text entry
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x100110c8), 0x41C80000);
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x021711d4), 0x390B0006);
	return;
}

void increase_block_move_animation() {
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x00153b00, 7), 0x00035AAC); //update .rela.text entries
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x00153b48, 7), 0x00035AAC);

	uint32_t offset = 0x101cb424;
	for (int i = 0; i < 13; i++) { //13 types of blocks total
		elfUtil::write_u16(gRPX, elfUtil::AddressToOffset(gRPX, offset + 0x04), 0x000C); // Reduce number frames for pushing to last from 20 to 12
		elfUtil::write_u16(gRPX, elfUtil::AddressToOffset(gRPX, offset + 0x0A), 0x000C); // Reduce number frames for pulling to last from 20 to 12
		offset += 0x9C;
	}
	return;
}

void increase_misc_animations() {
	//Float is shared, redirect it to read another float with the right value
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x00148820, 7), 0x000358D8);
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x001482a4, 7), 0x00035124);
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x00148430, 7), 0x000358D8);
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x00148310, 7), 0x00035AAC);
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x0014e2d4, 7), 0x00035530);

	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x02508b50), 0x3880000A);
	return;
}

void set_casual_clothes() {
	uint32_t starting_clothes_addr = custom_symbols.at("should_start_with_heros_clothes");
	elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, starting_clothes_addr), 0);
}

void hide_ship_sail() {
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x02162B04), 0x4E800020);
	return;
}

void shorten_auction_intro_event() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/Orichh_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat");

	FileTypes::EventList event_list;
	event_list.loadFromFile(path.string());
	Event& auction_start_event = event_list.Events_By_Name["AUCTION_START"];
	std::optional<std::reference_wrapper<Actor>> actor = auction_start_event.get_actor("CAMERA");
	if (!actor.has_value()) {
		return;
	}
	Actor& camera = actor.value(); //to avoid calling .value() every time a member is accessed

	camera.actions.erase(camera.actions.begin() + 3, camera.actions.begin() + 5); //last iterator not inclusive, only erase actions 3-4

	event_list.writeToFile(path.string());
	return;
}

void disable_invisible_walls() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/M_NewD2_Room2.szs@YAZ0@SARC@Room2.bfres@BFRES@room.dzr");

	FileTypes::DZXFile dzr;
	dzr.loadFromFile(path.string());
	std::vector<ChunkEntry*> scobs = dzr.entries_by_type("SCOB");

	for (ChunkEntry* scob : scobs) {
		if (std::strncmp(&scob->data[0], "Akabe\x00\x00\x00", 8) == 0) {
			scob->data[0xB] = '\xFF';
		}
	}

	dzr.writeToFile(path.string());
	return;
}

void update_skip_rematch_bosses_game_variable(const bool skipRefights) {
	uint32_t skip_rematch_bosses_addr = custom_symbols.at("skip_rematch_bosses");
	if (skipRefights) {
		elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, skip_rematch_bosses_addr), 0x01);
	}
	else {
		elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, skip_rematch_bosses_addr), 0x00);
	}
}

void update_sword_mode_game_variable(const SwordMode swordMode) {
	uint32_t sword_mode_addr = custom_symbols.at("sword_mode");

	if (swordMode == SwordMode::StartWithSword) {
		elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, sword_mode_addr), 0x00);
	}
	else if (swordMode == SwordMode::RandomSword) {
		elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, sword_mode_addr), 0x01);
	}
	else if (swordMode == SwordMode::NoSword) {
		elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, sword_mode_addr), 0x02);
	}

	return;
}

void update_starting_gear(const std::vector<GameItem>& startingItems) {
	std::vector<GameItem> startingGear = startingItems; //copy so we can edit without causing problems
	if (auto it = std::find(startingGear.begin(), startingGear.end(), GameItem::MagicMeterUpgrade); it != startingItems.end()) {
		give_double_magic();
		startingGear.erase(it);
	}

	if (startingGear.size() > MAXIMUM_ADDITIONAL_STARTING_ITEMS) {
		return; //error
	}

	uint32_t starting_gear_array_addr = custom_symbols["starting_gear"];
	for (unsigned int i = 0; i < startingGear.size(); i++) {
		uint8_t item_id = static_cast<std::underlying_type_t<GameItem>>(startingGear[i]);
		elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, starting_gear_array_addr + i), item_id);
	}

	elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, starting_gear_array_addr + startingGear.size()), 0xFF);

	return;
}

void update_swordless_text() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");

	FileTypes::MSBTFile msbt;
	msbt.loadFromFile(path.string());
	msbt.messages_by_label["01128"].text.message = std::u16string(CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", you may not have the\nMaster Sword, but do not be afraid!\n\n\nThe hammer of the dead is all you\nneed to crush your foe...\n\n\nEven as his ball of fell magic bears down\non you, you can " + TEXT_COLOR_RED + u"knock it back\nwith an empty bottle" + TEXT_COLOR_DEFAULT + u"!\n\n...I am sure you will have a shot at victory!\0", 287);;
	msbt.messages_by_label["01590"].text.message = std::u16string(CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u"! Do not run! Trust in the\npower of the Skull Hammer!\0", 62);
	msbt.writeToFile(path.string());

	return;
}

void add_hint_signs() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");

	FileTypes::MSBTFile msbt;
	msbt.loadFromFile(path.string());
	std::string new_message_label = "00847";
	Attributes attributes;
	attributes.character = 0xF; //sign
	attributes.boxStyle = 0x2;
	attributes.drawType = 0x1;
	attributes.screenPos = 0x2;
	attributes.lineAlignment = 3;
	TSY1Entry tsy;
	tsy.styleIndex = 0x12B;
	std::u16string message(IMAGE(ImageTags::R_ARROW) + u"\0", 5);
	msbt.addMessage(new_message_label, attributes, tsy, message);
	msbt.writeToFile(path.string());

	path = g_session.openGameFile("content/Common/Stage/M_NewD2_Room2.szs@YAZ0@SARC@Room2.bfres@BFRES@room.dzr");

	FileTypes::DZXFile dzr;
	dzr.loadFromFile(path.string());
	std::vector<ChunkEntry*> actors = dzr.entries_by_type("ACTR");

	std::vector<ChunkEntry*> bomb_flowers;
	for (ChunkEntry* actor : actors) {
		if (std::strncmp(&actor->data[0], "BFlower", 8) == 0) bomb_flowers.push_back(actor);
	}
	bomb_flowers[0]->data = std::string("\x4B\x61\x6E\x62\x61\x6E\x00\x00\x00\x00\x03\x4F\x44\x34\x96\xEB\x42\x47\xFF\xFF\xC2\x40\xB0\x3A\x00\x00\x20\x00\x00\x00\xFF\xFF", 0x20);

	dzr.writeToFile(path.string());

	return;
}

void prevent_door_boulder_softlocks() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/M_NewD2_Room13.szs@YAZ0@SARC@Room13.bfres@BFRES@room.dzr");

	FileTypes::DZXFile room13;
	room13.loadFromFile(path.string());

	ChunkEntry& swc00 = room13.add_entity("SCOB");
	swc00.data = std::string("\x53\x57\x5F\x43\x30\x30\x00\x00\x00\x03\xFF\x05\x45\x24\xB0\x00\x00\x00\x00\x00\x43\x63\x00\x00\x00\x00\xC0\x00\xFF\xFF\xFF\xFF\x20\x10\x10\xFF", 0x24);
	room13.writeToFile(path.string());

	path = g_session.openGameFile("content/Common/Stage/M_NewD2_Room14.szs@YAZ0@SARC@Room14.bfres@BFRES@room.dzr");

	FileTypes::DZXFile room14;
	room14.loadFromFile(path.string());

	swc00 = room14.add_entity("SCOB");
	swc00.data = std::string("\x53\x57\x5F\x43\x30\x30\x00\x00\x00\x03\xFF\x06\xC5\x7A\x20\x00\x44\xF3\xC0\x00\xC5\x06\xC0\x00\x00\x00\xA0\x00\xFF\xFF\xFF\xFF\x20\x10\x10\xFF", 0x24);
	room14.writeToFile(path.string());

	return;
}

void update_tingle_statue_item_get_funcs() {
	const uint32_t item_get_func_ptr = 0x0001da54; //First relevant relocation entry in .rela.data (overwrites .data section when loaded)
	const std::unordered_map<int, std::string> symbol_name_by_item_id = { {0xA3, "dragon_tingle_statue_item_get_func"}, {0xA4, "forbidden_tingle_statue_item_get_func"}, {0xA5, "goddess_tingle_statue_item_get_func"}, {0xA6, "earth_tingle_statue_item_get_func"}, {0xA7, "wind_tingle_statue_item_get_func"} };

	for (const int statue_id : {0xA3, 0xA4, 0xA5, 0xA6, 0xA7}) {
		uint32_t item_func_addr = item_get_func_ptr + (statue_id * 0xC) + 8;
		uint32_t item_func_ptr = custom_symbols.at(symbol_name_by_item_id.at(statue_id));
		elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, item_func_addr, 9), item_func_ptr - 0x02000000);
	}
	return;
}

void make_tingle_statue_reward_rupee_rainbow_colored() {
	uint32_t item_resources_list_start = 0x101e4674;

	uint32_t rainbow_rupee_item_resource_addr = item_resources_list_start + 0xB8 * 0x24;

	elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, rainbow_rupee_item_resource_addr + 0x14), 0x07);
	return;
}

void show_seed_hash_on_title_screen(const std::u16string& hash) { //make sure hash is null terminated
	using namespace NintendoWare::Layout;

	RandoSession::fspath path = g_session.openGameFile("content/Common/Layout/Title_00.szs@YAZ0@SARC@blyt/Title_00.bflyt");

	FileTypes::FLYTFile layout;
	layout.loadFromFile(path.string());

	//add hash
	Pane& newPane = layout.rootPane.children[0].children[1].children[3].duplicateChildPane(1); //unused version number text
	newPane.pane->name = "T_Hash";
	newPane.pane->name.resize(0x18);
	dynamic_cast<txt1*>(newPane.pane.get())->text = u"Seed Hash:\n" + hash;
	dynamic_cast<txt1*>(newPane.pane.get())->fontIndex = 0;
	dynamic_cast<txt1*>(newPane.pane.get())->restrictedLen = 11 + hash.length();
	dynamic_cast<txt1*>(newPane.pane.get())->lineAlignment = txt1::LineAlignment::CENTER;
	dynamic_cast<txt1*>(newPane.pane.get())->translation.X = -491.0f;
	dynamic_cast<txt1*>(newPane.pane.get())->translation.Y = -113.0f;
	dynamic_cast<txt1*>(newPane.pane.get())->width = 205.0f;
	dynamic_cast<txt1*>(newPane.pane.get())->height = 100.0f;

	layout.writeToFile(path.string());
	return;
}

//key bag

//required dungeon map markers

void add_chest_in_place_jabun_cutscene() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/Pjavdou_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr");

	FileTypes::DZXFile dzr;
	dzr.loadFromFile(path.string());
	ChunkEntry& raft = dzr.add_entity("ACTR");
	ChunkEntry& chest = dzr.add_entity("TRES");
	raft.data = std::string("Ikada\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\xFF\xFF", 0x20);
	chest.data = std::string("takara3\x00\xFF\x2F\xF3\x05\x00\x00\x00\x00\x43\x96\x00\x00\xC3\x48\x00\x00\x00\x00\x80\x00\x05\xFF\xFF\xFF", 0x20);
	dzr.writeToFile(path.string());

	return;
}

void add_jabun_obstacles_to_default_layer() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/szs_permanent2.pack@SARC@sea_Room44.szs@YAZ0@SARC@Room44.bfres@BFRES@room.dzr");

	FileTypes::DZXFile dzr;
	dzr.loadFromFile(path.string());

	std::vector<ChunkEntry*> layer_5_actors = dzr.entries_by_type_and_layer("ACTR", 5);
	ChunkEntry* layer_5_door = layer_5_actors[0];
	ChunkEntry* layer_5_whirlpool = layer_5_actors[1];

	ChunkEntry& newDoor = dzr.add_entity("ACTR");
	ChunkEntry& newWhirlpool = dzr.add_entity("ACTR");
	newDoor.data = layer_5_door->data;
	newWhirlpool.data = layer_5_whirlpool->data;

	dzr.remove_entity(layer_5_door);
	dzr.remove_entity(layer_5_whirlpool);

	dzr.writeToFile(path.string());

	return;
}

void remove_jabun_stone_door_event() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@event_list.dat");

	FileTypes::EventList event_list;
	event_list.loadFromFile(path.string());
	Event& unlock_cave_event = event_list.Events_By_Name["ajav_uzu"];
	std::optional<std::reference_wrapper<Actor>> actor = unlock_cave_event.get_actor("DIRECTOR");
	if (!actor.has_value()) {
		return;
	}
	Actor& director = actor.value(); //set references after error checking instead of using .value() every time a member is accessed
	actor = unlock_cave_event.get_actor("CAMERA");
	if (!actor.has_value()) {
		return;
	}
	Actor &camera = actor.value(); //same as director
	actor = unlock_cave_event.get_actor("Ship");
	if (!actor.has_value()) {
		return;
	}
	Actor& ship = actor.value(); //same as camera

	director.actions.erase(director.actions.begin() + 2, director.actions.end());
	camera.actions.erase(camera.actions.begin() + 3, camera.actions.end());
	ship.actions.erase(ship.actions.begin() + 3, ship.actions.end());
	unlock_cave_event.ending_flags = {
		director.actions.back().flag_id_to_set,
		camera.actions.back().flag_id_to_set,
		-1
	};

	event_list.writeToFile(path.string());
	//Untested, references might break

	return;
}

void add_chest_in_place_master_sword() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/kenroom_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr");

	FileTypes::DZXFile dzr;
	dzr.loadFromFile(path.string());

	std::vector<ChunkEntry*> default_layer_actors = dzr.entries_by_type_and_layer("ACTR", DEFAULT_LAYER);
	dzr.remove_entity(default_layer_actors[5]);
	dzr.remove_entity(default_layer_actors[6]);

	std::vector<ChunkEntry*> layer_5_actors = dzr.entries_by_type_and_layer("ACTR", 5);
	std::array<ChunkEntry*, 4> layer_5_to_copy = { layer_5_actors[0], layer_5_actors[2], layer_5_actors[3], layer_5_actors[4] };

	for (ChunkEntry* orig_actor : layer_5_to_copy) {
		ChunkEntry& new_actor = dzr.add_entity("ACTR");
		new_actor.data = orig_actor->data;
		dzr.remove_entity(orig_actor);
	}

	ChunkEntry& chest = dzr.add_entity("TRES");
	chest.data = std::string("takara3\x00\xFF\x20\x50\x04\xc2\xf6\xfd\x71\xc5\x49\x40\x00\xc5\xf4\xe9\x0a\x00\x00\x00\x00\x6a\xff\xff\xff", 0x20);

	dzr.writeToFile(path.string());

	return;
}

void update_spoil_sell_text() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt");

	FileTypes::MSBTFile msbt;
	msbt.loadFromFile(path.string());
	std::vector<std::u16string> lines = Utility::Str::split(msbt.messages_by_label["03957"].text.message, u'\n');
	if (lines.size() != 5) return; //incorrect number of lines
	lines[2] = u"And no Blue Chu Jelly, either!";
	msbt.messages_by_label["03957"].text.message = Utility::Str::merge(lines, u'\n');
	msbt.writeToFile(path.string());

	return;
}

void fix_totg_warp_spawn() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Stage/kenroom_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr");

	FileTypes::DZXFile dzr;
	dzr.loadFromFile(path.string());

	std::vector<ChunkEntry*> spawns = dzr.entries_by_type("PLYR");
	ChunkEntry* spawn = spawns[9];
	spawn->data = std::string("\x4C\x69\x6E\x6B\x00\x00\x00\x00\x32\xFF\x20\x1A\x47\xC3\x4F\x5F\x00\x00\x00\x00\xBF\xBE\xBF\x90\x00\x00\x00\x00\x01\x01\xFF\xFF", 0x20);

	dzr.writeToFile(path.string());
}

void remove_phantom_ganon_req_for_reefs() {
	std::string path;
	for (int room_index : {24, 46, 22, 8, 37, 25}) {
		path = "content/Common/Stage/sea_Room" + std::to_string(room_index) + ".szs@YAZ0@SARC@Room" + std::to_string(room_index) + ".bfres@BFRES@room.dzr";
		RandoSession::fspath filePath = g_session.openGameFile(path);
		FileTypes::DZXFile room_dzr;
		room_dzr.loadFromFile(filePath.string());
		std::vector<ChunkEntry*> actors = room_dzr.entries_by_type("ACTR");
		for (ChunkEntry* actor : actors) {
			if (std::strncmp(&actor->data[0], "Ocanon\x00\x00", 8) == 0) {
				if (std::strncmp(&actor->data[0xA], "\x2A", 1) == 0) {
					actor->data[0xA] = '\xFF'; //check if this is right index
				}
			}
			else if (std::strncmp(&actor->data[0], "Oship\x00\x00\x00", 8) == 0) {
				if (std::strncmp(&actor->data[0x18], "\x2A", 1) == 0) {
					actor->data[0x18] = '\xFF'; //check if this is right index
				}
			}
		}
		room_dzr.writeToFile(filePath.string());
	}
	return;
}

void fix_ff_door() {
	int face_index = 0x1493;
	uint16_t new_prop_index = 0x0011;

	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/szs_permanent1.pack@SARC@sea_Room1.szs@YAZ0@SARC@Room1.bfres@BFRES@room.dzb");
	std::fstream fptr(path, std::ios::binary);

	fptr.seekg(0xC, std::ios::beg);
	uint32_t face_list_offset;
	fptr.read(reinterpret_cast<char*>(&face_list_offset), 4);
	Utility::Endian::toPlatform_inplace(eType::Big, face_list_offset);

	fptr.seekp((face_list_offset + face_index * 0xA) + 6, std::ios::beg);
	fptr.write(reinterpret_cast<const char*>(&new_prop_index), 2);

	return;
}

//add bog warp

//rat hole culling

//not needed until enemy rando is a thing
/*void add_failsafe_id_0_spawns() {
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
		newSpawn.data = std::string("Link\x00\x00\x00\x00");
		newSpawn.data.resize(0x20);

		uint32_t params = 0xFFFFFFFF;
		//https://github.com/LagoLunatic/wwrando/blob/master/tweaks.py#L2160
	}
}*/

void remove_minor_pan_cs() {
	std::array<pan_cs_info, 7> panning_cs{
		{
			{"M_NewD2", "Room2", 4}, {"kindan", "Stage", 2}, {"Siren", "Room18", 2}, {"M_Dai", "Room3", 7}, {"sea", "Room41", 19}, {"sea", "Room41", 22}, {"sea", "Room41", 23}
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
		FileTypes::DZXFile dzx;
		dzx.loadFromFile(filePath.string());
		std::vector<ChunkEntry*> scobs = dzx.entries_by_type("SCOB");
		for (ChunkEntry* scob : scobs) {
			if (std::strncmp(&scob->data[0], "TagEv\x00\x00\x00", 8) == 0) {
				if (scob->data[0xA] == cs_info.evnt_index) {
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
		dzx.writeToFile(filePath.string());
	}

	return;
}

//custom actors?

void fix_stone_head_bugs() {
	uint32_t status_bits = elfUtil::read_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x101ca100));
	Utility::Endian::toPlatform_inplace(eType::Big, status_bits);
	status_bits &= ~0x00000080;
	Utility::Endian::toPlatform_inplace(eType::Big, status_bits);
	elfUtil::write_u32(gRPX, elfUtil::AddressToOffset(gRPX, 0x101ca100), status_bits);

	return;
}

void show_tingle_statues_on_quest_screen() {

	g_session.copyToGameFile("./assets/Tingle.bflim", "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@BtnMapIcon_00.szs@YAZ0@SARC@timg/MapBtn_00^l.bflim");
	g_session.copyToGameFile("./assets/Tingle_Shadow.bflim", "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@BtnMapIcon_00.szs@YAZ0@SARC@timg/MapBtn_07^t.bflim");

	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt");

	FileTypes::MSBTFile msbt;
	msbt.loadFromFile(path.string());
	msbt.messages_by_label["00503"].text.message = std::u16string(u"Tingle Statues\0", 15);
	msbt.messages_by_label["00703"].text.message = std::u16string(u"Golden statues of a mysterious dashing\n figure. They can be traded to " + TEXT_COLOR_RED + u"Ankle" + TEXT_COLOR_DEFAULT + u" on" + TEXT_COLOR_RED + u"Tingle Island" + TEXT_COLOR_DEFAULT + u" for a reward!\0", 137); //need to use constructor with length because of null characters
	msbt.writeToFile(path.string());

	return;
}

void add_shortcut_warps_into_dungeons() {
	RandoSession::fspath path = g_session.openGameFile("content/Common/Pack/szs_permanent2.pack@SARC@sea_Room41.szs@YAZ0@SARC@Room41.bfres@BFRES@room.dzr");

	FileTypes::DZXFile dzr;
	dzr.loadFromFile(path.string());
	ChunkEntry& sw_c00 = dzr.add_entity("SCOB");
	sw_c00.data = std::string("SW_C00\x00\x00\x00\x03\xFF\x7F\x48\x40\x24\xED\x45\x44\x99\xB1\x48\x41\x7B\x63\x00\x00\x00\x00\x00\x00\xFF\xFF\x96\x14\x28\xFF", 0x24);

	ChunkEntry& warp = dzr.add_entity("SCOB");
	warp.data = std::string("Ysdls00\x00\x10\xFF\x06\x7F\x48\x54\x16\x86\x42\x0B\xFF\xF8\x48\x3E\xD3\xED\x00\x00\x00\x00\x00\x00\xFF\xFF", 0x20);

	dzr.writeToFile(path.string());
	return;
}



void apply_necessary_tweaks(const Settings& settings, const std::string& seedHash) {
	Load_Custom_Symbols("./asm/custom_symbols.txt");

	RandoSession::fspath rpxPath = g_session.openGameFile("code/cking.rpx@RPX");
	gRPX.loadFromFile(rpxPath.string());

	std::u16string u16_seedHash = Utility::Str::toUTF16(seedHash);

	Apply_Patch("./asm/patch_diffs/custom_data_diff.json");
	Apply_Patch("./asm/patch_diffs/custom_funcs_diff.json");
	Apply_Patch("./asm/patch_diffs/make_game_nonlinear_diff.json");
	Apply_Patch("./asm/patch_diffs/remove_cutscenes_diff.json");
	Apply_Patch("./asm/patch_diffs/flexible_item_locations_diff.json");
	Apply_Patch("./asm/patch_diffs/fix_vanilla_bugs_diff.json");
	Apply_Patch("./asm/patch_diffs/misc_rando_features_diff.json");
	
	Add_Relocations("./asm/patch_diffs/custom_funcs_reloc.json");
	Add_Relocations("./asm/patch_diffs/make_game_nonlinear_reloc.json");
	Add_Relocations("./asm/patch_diffs/remove_cutscenes_reloc.json");
	Add_Relocations("./asm/patch_diffs/flexible_item_locations_reloc.json");
	Add_Relocations("./asm/patch_diffs/fix_vanilla_bugs_reloc.json");
	Add_Relocations("./asm/patch_diffs/misc_rando_features_reloc.json");

	Remove_Relocation({7, 0x001c0ae8}); //would mess with the custom save_init call

	start_at_outset_dock();
	start_ship_at_outset();
	fix_deku_leaf_model();
	allow_all_items_to_be_field_items();
	remove_shop_item_forced_uniqueness_bit();
	remove_ff2_cutscenes();
	make_items_progressive();
	add_ganons_tower_warp_to_ff2();
	add_chest_in_place_medli_gift();
	add_chest_in_place_queen_fairy_cutscene();
	add_more_magic_jars();
	modify_title_screen();
	update_name_and_icon();
	allow_dungeon_items_to_appear_anywhere();
	fix_shop_item_y_offsets();
	shorten_zephos_event();
	update_korl_dialog();
	set_num_starting_triforce_shards(settings.num_starting_triforce_shards);
	set_starting_health(settings.starting_pohs, settings.starting_hcs);
	set_damage_multiplier(settings.damage_multiplier);
	set_pig_color(settings.pigColor);
	add_pirate_ship_to_windfall(); //doesnt fix getting stuck behind door
	remove_makar_kidnapping();
	increase_crawl_speed();
	add_chart_number_to_item_get_messages();
	increase_grapple_animation_speed();
	increase_block_move_animation();
	increase_misc_animations();
	shorten_auction_intro_event();
	disable_invisible_walls();
	add_hint_signs();
	prevent_door_boulder_softlocks();
	update_tingle_statue_item_get_funcs();
	make_tingle_statue_reward_rupee_rainbow_colored();
	show_seed_hash_on_title_screen(u16_seedHash);
	//key bag
	add_chest_in_place_jabun_cutscene();
	add_chest_in_place_master_sword();
	update_spoil_sell_text();
	fix_totg_warp_spawn();
	remove_phantom_ganon_req_for_reefs();
	fix_ff_door();
	//bog warp
	//rat hole visibility
	//failsafe id 0 spawns
	remove_minor_pan_cs();
	fix_stone_head_bugs();
	show_tingle_statues_on_quest_screen();

	gRPX.writeToFile(rpxPath.string());
	return;
}

void apply_necessary_post_randomization_tweaks(const bool randomizeItems, const std::vector<Location>& itemLocations) {
	RandoSession::fspath rpxPath = g_session.openGameFile("code/cking.rpx@RPX");
	gRPX.loadFromFile(rpxPath.string()); //reload to avoid conflicts written between pre- and post- randomization tweaks

	if (randomizeItems) {
		//placeholders, will change based on logic implementation
		// 
		//update_shop_item_descriptions(itemLocations.at("The Great Sea - Beedle's Shop Ship - 20 Rupee Item").currentItem, itemLocations.at("Rock Spire Isle - Beedle's Special Shop Ship - 500 Rupee Item").currentItem, itemLocations.at("Rock Spire Isle - Beedle's Special Shop Ship - 950 Rupee Item").currentItem, itemLocations.at("Rock Spire Isle - Beedle's Special Shop Ship - 900 Rupee Item").currentItem);
		//update_auction_item_names(itemLocations.at("Windfall Island - 5 Rupee Auction").currentItem, itemLocations.at("Windfall Island - 40 Rupee Auction").currentItem, itemLocations.at("Windfall Island - 60 Rupee Auction").currentItem, itemLocations.at("Windfall Island - 80 Rupee Auction").currentItem, itemLocations.at("Windfall Island - 100 Rupee Auction").currentItem);
		//update_battlesquid_item_names(itemLocations.at("Windfall Island - Battlesquid - First Prize").currentItem, itemLocations.at("Windfall Island - Battlesquid - Second Prize").currentItem);
		//update_item_names_in_letter_advertising_rock_spire_shop(itemLocations.at("Rock Spire Isle - Beedle's Special Shop Ship - 500 Rupee Item").currentItem, itemLocations.at("Rock Spire Isle - Beedle's Special Shop Ship - 950 Rupee Item").currentItem, itemLocations.at("Rock Spire Isle - Beedle's Special Shop Ship - 900 Rupee Item").currentItem);
		//update_savage_labyrinth_hint_tablet(itemLocations.at("Outset Island - Savage Labyrinth - Floor 30").currentItem, itemLocations.at("Outset Island - Savage Labyrinth - Floor 50").currentItem);
	}
	//dungeon sea quest markers
	gRPX.writeToFile(rpxPath.string());
}

#define ENABLE_DEBUG 1
#define FILL_TESTING 1

//this is just for testing 
#include "logic/SpoilerLog.hpp"
#include "logic/Generate.hpp"
#include "logic/Random.hpp"

int main() {

	//timing stuff
	//auto start = std::chrono::high_resolution_clock::now();
	//auto stop = std::chrono::high_resolution_clock::now();
	//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	//auto duration2 = duration.count();

	Load_Custom_Symbols("./asm/custom_symbols.txt");

	RandoSession::fspath rpxPath = g_session.openGameFile("code/cking.rpx@RPX");
	gRPX.loadFromFile(rpxPath.string());

	Apply_Patch("./asm/patch_diffs/custom_funcs_diff.json");
	Apply_Patch("./asm/patch_diffs/make_game_nonlinear_diff.json");
	Apply_Patch("./asm/patch_diffs/remove_cutscenes_diff.json");
	Apply_Patch("./asm/patch_diffs/flexible_item_locations_diff.json");
	Apply_Patch("./asm/patch_diffs/fix_vanilla_bugs_diff.json");
	Apply_Patch("./asm/patch_diffs/misc_rando_features_diff.json");
	
	Add_Relocations("./asm/patch_diffs/custom_funcs_reloc.json");
	Add_Relocations("./asm/patch_diffs/make_game_nonlinear_reloc.json");
	Add_Relocations("./asm/patch_diffs/remove_cutscenes_reloc.json");
	Add_Relocations("./asm/patch_diffs/flexible_item_locations_reloc.json");
	Add_Relocations("./asm/patch_diffs/fix_vanilla_bugs_reloc.json");
	Add_Relocations("./asm/patch_diffs/misc_rando_features_reloc.json");

	show_tingle_statues_on_quest_screen();

	allow_all_items_to_be_field_items();

	elfUtil::write_u32(gRPX, {28, 0x00000058}, gRPX.shdr_table[2].second.data.size());
	Remove_Relocation({ 7, 0x001c0ae8 }); //would mess with the custom save_init call
	start_at_outset_dock();
	//modify_title_screen();
	//show_seed_hash_on_title_screen(u"helmarocKing\njun-Roberto");

	//fix_deku_leaf_model();
	//
	//add_chest_in_place_queen_fairy_cutscene();
	//add_chest_in_place_jabun_cutscene();
	//add_chest_in_place_master_sword();
	//add_chest_in_place_medli_gift();

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

	int seed = Random(0, 100000);

	int worldCount = 1;
	World blankWorld;
	WorldPool worlds(worldCount, blankWorld);
	std::vector<Settings> settingsVector{ settings };

	int retVal = generateWorlds(worlds, settingsVector, seed);
	
	if (retVal == 0)
	{
		std::cout << "Generating Spoiler Log" << std::endl;
		generateSpoilerLog(worlds);
	}

	//get world locations with "worlds[playerNum - 1].locationEntries"
	//assume 1 world for now, modifying multiple copies needs work
	for (const Location& location : worlds[0].locationEntries) {
		if (ModificationError err = location.method->writeLocation(location.currentItem); err != ModificationError::NONE) return 1; //handle err somehow
	}
	saveRPX();

	//get world with "worlds[playerNum - 1]"
	//assume 1 world for now, modifying multiple copies needs work
	//apply_necessary_post_randomization_tweaks(1, worlds[0].locationEntries);

	gRPX.writeToFile(g_session.openGameFile("code/cking.rpx@RPX").string());
	g_session.repackCache();
	return 0;
}
