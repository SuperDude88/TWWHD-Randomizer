#include "tweaks.hpp"

FileTypes::ELF outRPX;

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
	uint8_t spawn_id_to_copy = 0;
};

struct pan_cs_info {
	std::string stage_name;
	std::string szs_suffix;
	int evnt_index;
};

bool containsAddress(int address, int memAddress, int sectionLen) {
	if (memAddress <= address && address < memAddress + sectionLen) {
		return true;
	}
	else {
		return false;
	}
}

std::pair<int, int> AddressToOffset(int address) { //calculates offset into section, returns first value as section index and second as offset
	for (int index : {2, 3, 4}) { //only check a few sections that might be written to
		if (containsAddress(address, outRPX.shdr_table[index].second.sh_addr, outRPX.shdr_table[index].second.sh_size)) {
			return {index, address - outRPX.shdr_table[index].second.sh_addr};
		}
	}
	return {0, 0};
}

std::pair<int, int> AddressToOffset(int address, int sectionIndex) { //.rela sections all have same address ranges, need to specify index to make it work
	if (!containsAddress(address, outRPX.shdr_table[sectionIndex].second.sh_addr, outRPX.shdr_table[sectionIndex].second.sh_size)) {
		return {0, 0};
	}
	return {sectionIndex, address - outRPX.shdr_table[sectionIndex].second.sh_addr};
}

void write_u8_to_rpx(std::pair<int, int> offset, uint8_t data) { //assume sections are sorted by index, always should be 
	outRPX.shdr_table[offset.first].second.data[offset.second] = (char)data;
	return;
}

void write_u16_to_rpx(std::pair<int, int> offset, uint16_t data) { //assume sections are sorted by index, always should be 
	data = Utility::byteswap(data);
	outRPX.shdr_table[offset.first].second.data.replace(offset.second, 2, (char*)&data, 2);
	return;
}

void write_u32_to_rpx(std::pair<int, int> offset, uint32_t data) { //assume sections are sorted by index, always should be 
	data = Utility::byteswap(data);
	outRPX.shdr_table[offset.first].second.data.replace(offset.second, 4, (char*)&data, 4);
	return;
}

void write_float_to_rpx(std::pair<int, int> offset, float data) { //assume sections are sorted by index, always should be 
	data = Utility::byteswap(data);
	outRPX.shdr_table[offset.first].second.data.replace(offset.second, 4, (char*)&data, 4);
	return;
}

void write_bytes_to_rpx(std::pair<int, int> offset, std::vector<uint8_t> Bytes) { //assume sections are sorted by index, always should be 
	for (unsigned int i = 0; i < Bytes.size(); i++) {
		outRPX.shdr_table[offset.first].second.data[offset.second + i] = (char)Bytes[i];
	}
	return;
}

uint8_t read_rpx_u8(std::pair<int, int> offset) { //assume sections are sorted by index, always should be 
	return *(uint8_t*)&outRPX.shdr_table[offset.first].second.data[offset.second];
}

uint32_t read_rpx_u32(std::pair<int, int> offset) { //assume sections are sorted by index, always should be 
	return *(uint32_t*)&outRPX.shdr_table[offset.first].second.data[offset.second];
}

float read_rpx_float(std::pair<int, int> offset) { //assume sections are sorted by index, always should be 
	return *(float*)&outRPX.shdr_table[offset.first].second.data[offset.second];
}

std::vector<uint8_t> read_rpx_bytes(std::pair<int, int> offset, int NumBytes) {
	uint8_t buffer = 0x0;
	std::vector<uint8_t> Bytes;
	Bytes.reserve(NumBytes); //avoid extra reallocations
	for (int i = 0; i < NumBytes; i++) {
		buffer = *(uint8_t*)&outRPX.shdr_table[offset.first].second.data[offset.second + i];
		Bytes.push_back(buffer);
	}
	return Bytes;
}

nlohmann::json Load_Patches(std::string file_path) {
	std::ifstream fptr;
	fptr.open(file_path, std::ios::in);

	nlohmann::json patches = nlohmann::json::parse(fptr);

	return patches;
}

void Apply_Patch_OLD(nlohmann::json patches, std::string name) { //original format, keeping as reference for now
	for (auto& data : patches[name]) {
		for (auto& offset_pair : data.items()) {
			int offset = std::stoi(offset_pair.key(), nullptr, 16);
			for (std::string byte : offset_pair.value()) {
				uint8_t toWrite = std::stoi(byte, nullptr, 16);
				write_u8_to_rpx(AddressToOffset(offset), toWrite);
				offset++; //This is because we cycle through the bytes individually, so for this we need to increase the offset by one each time to make it work in the file
			}
		}
	}
	return;
}

void Apply_Patch(std::string file_path) {
	std::ifstream fptr;
	fptr.open(file_path, std::ios::in);

	nlohmann::json patches = nlohmann::json::parse(fptr);

	for (auto& patch : patches.items()) {
		int offset = std::stoi(patch.key(), nullptr, 16);
		for (std::string byte : patch.value()) {
			uint8_t toWrite = std::stoi(byte, nullptr, 16);
			write_u8_to_rpx(AddressToOffset(offset), toWrite);
			offset++; //Cycles through the bytes individually, need to increase the offset by one each time
		}
	}

	return;
}

nlohmann::json Load_Relocations(std::string file_path) { //untested
	std::ifstream fptr;
	fptr.open(file_path, std::ios::in);

	nlohmann::json relocations = nlohmann::json::parse(fptr);

	return relocations;
}

void Add_Relocations(nlohmann::json in) { //untested
	std::string entry;
	entry.resize(12);
	for (auto& relocation : in.items()) {
		auto& data = relocation.value();
		int section_index = std::stoi(relocation.key(), nullptr, 16);
		Elf32_Rela reloc;
		reloc.r_offset = std::stoi((std::string)data.at("r_offset"), nullptr, 16);
		reloc.r_info = std::stoi((std::string)data.at("r_info"), nullptr, 16);
		reloc.r_addend = std::stoi((std::string)data.at("r_addend"), nullptr, 16);

		entry.replace(0, 4, (char*)&reloc.r_offset, 4);
		entry.replace(4, 4, (char*)&reloc.r_info, 4);
		entry.replace(8, 4, (char*)&reloc.r_addend, 4);
		outRPX.extend_section(section_index, entry);
	}
	return;
}

//End of helper functions (might get moved into a separate file later)

void set_new_game_starting_location(uint8_t spawn_id, uint8_t room_index) {
	write_u8_to_rpx(AddressToOffset(0x025b508F), room_index);
	write_u8_to_rpx(AddressToOffset(0x025b50CB), room_index);
	write_u8_to_rpx(AddressToOffset(0x025B5093), spawn_id);
	write_u8_to_rpx(AddressToOffset(0x025B50CF), spawn_id);
	return;
}

void change_ship_starting_island(int room_index) {
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
	RandoSession::fspath filePath = path;
	std::ifstream in = g_session.openGameFile(sepPath(path, '@'), filePath);
	FileTypes::DZXFile room_dzr;
	room_dzr.loadFromBinary(in);
	std::vector<ChunkEntry*> ship_spawns = room_dzr.entries_by_type("SHIP");
	ChunkEntry* ship_spawn_0 = (ChunkEntry*)0; //initialization is just to make compiler happy
	for (ChunkEntry* spawn : ship_spawns) { //Find spawn with ID 0
		if (reinterpret_cast<uint8_t*>(&spawn->data[0xE]) == 0) ship_spawn_0 = spawn;
	}

	FileTypes::DZXFile stage_dzs;
	RandoSession::fspath stage_filePath = "content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs";
	std::string stage_path = "content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs";
	std::ifstream stage_in = g_session.openGameFile(sepPath(stage_path, '@'), stage_filePath);
	stage_dzs.loadFromBinary(stage_in);
	std::vector<ChunkEntry*> actors = stage_dzs.entries_by_type("ACTR");
	for (ChunkEntry* actor : actors) {
		if (strncmp(&actor->data[0], "SHIP", 4) && ship_spawn_0 != nullptr) {
			actor->data.replace(0xC, 0xC, ship_spawn_0->data, 0x0, 0xC);
			actor->data.replace(0x1A, 0x2, ship_spawn_0->data, 0xC, 0x2);
			actor->data.replace(0x10, 0x4, "\xC8\xF4\x24\x00", 0x0, 0x4); //prevent softlock on fire mountain (may be wrong offset)
		}
	}
	room_dzr.writeToFile(g_session.relToExtract(path, '@'));
	g_session.repackGameFile(sepPath(path, '@'));

	stage_dzs.writeToFile(g_session.relToExtract(stage_path, '@'));
	g_session.repackGameFile(sepPath(stage_path, '@'));
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
	RandoSession::fspath paths[4] = { "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt", "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt", "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message3_msbt.szs@YAZ0@SARC@message3.msbt" , "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message4_msbt.szs@YAZ0@SARC@message4.msbt" };
	RandoSession::fspath repackPaths[4] = { "content/Common/Pack/permanent_2d_UsEnglish.pack.unpack/message_msbt.szs@YAZ0@SARC@message.msbt", "content/Common/Pack/permanent_2d_UsEnglish.pack.unpack/message2_msbt.szs@YAZ0@SARC@message2.msbt" , "content/Common/Pack/permanent_2d_UsEnglish.pack.unpack/message3_msbt.szs@YAZ0@SARC@message3.msbt" , "content/Common/Pack/permanent_2d_UsEnglish.pack.unpack/message4_msbt.szs@YAZ0@SARC@message4.msbt" };
	for (int i = 0; i < 4; i++) {
		std::string filePath = paths[i].string();
		std::ifstream in = g_session.openGameFile(sepPath(filePath, '@'), paths[i]);
		FileTypes::MSBTFile msbt;
		msbt.loadFromBinary(in);
		for (std::pair<const std::string, Message>& message : msbt.messages_by_label) {
			std::u16string& String = message.second.text.message;
			std::string::size_type Wait = String.find(std::u16string(u"\x0e\x01\x06\x02", 4)); //dont use macro because duration shouldnt matter
			while (Wait != std::u16string::npos) {
				String.erase(Wait, 5);
				Wait = String.find(std::u16string(u"\x0e\x01\x06\x02", 4));
			}
			std::string::size_type Wait_dismiss = String.find(std::u16string(u"\x0e\x01\x03\x02", 4)); //dont use macro because duration shouldnt matter
			while (Wait_dismiss != std::u16string::npos) {
				if (message.first != "07726" && message.first != "02488") { //exclude messages that are broken by removing the command
					String.erase(Wait_dismiss, 5);
				}
				Wait_dismiss = String.find(std::u16string(u"\x0e\x01\x03\x02", 4));
			}
			std::string::size_type Wait_dismiss_prompt = String.find(std::u16string(u"\x0e\x01\x02\x02", 4)); //dont use macro because duration shouldnt matter
			while (Wait_dismiss_prompt != std::u16string::npos) {
				String.erase(Wait_dismiss_prompt, 5);
				Wait_dismiss_prompt = String.find(std::u16string(u"\x0e\x01\x02\x02", 4));
			}
		}
		msbt.writeToFile(g_session.relToExtract(filePath, '@'));
		g_session.repackGameFile(sepPath(filePath, '@'), false);
	}
	std::string packPath = "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC";
	g_session.repackGameFile(sepPath(packPath, '@'));
	return;
}

void fix_deku_leaf_model() {
	uint32_t actor_offsets[2] = { 0x33b704, 0x33b9c4 };
	RandoSession::fspath path = "content/Common/Stage/Omori_Room0.szs@YAZ0";
	std::string pathString = path.string();
	g_session.openGameFile(sepPath(pathString, '@'), path);
	std::ofstream file(g_session.relToExtract(pathString, '@'), std::ios::binary);
	std::string data = "item\x00\x00\x00\x00\x01\xFF\x02\x34\xc4\x08\x7d\x81\x45\x9d\x59\xec\xc3\xf5\x8e\xd9\x00\x00\x00\x00\x00\xff\xff\xff";

	for (const uint32_t& offset : actor_offsets) {
		file.seekp(offset, std::ios::beg);
		file.write(&data[0], 0x20);
	}
	g_session.repackGameFile(sepPath(pathString, '@'));
	return;
}

void allow_all_items_to_be_field_items() {

	uint32_t item_resources_list_start = 0x101e4674;
	int field_item_resources_list_start = 0x101e6a74;

	const std::array<uint8_t, 171> Item_Ids_Without_Field_Model = {
	0x1a, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x35, 0x36, 0x39, 0x3a, 0x3c, 0x3e, 0x3f, 0x42, 0x43, 0x4c, 0x4d, 0x4e, 0x50, 0x51, 0x52,
	0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x98,
	0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd,
	0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xde, 0xdd, 0xdf, 0xe0, 0xe1, 0xe2,
	0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe
	};

	const std::unordered_map<int, int> szs_name_pointers{
		{0x1a, 0x1004e5d8}, {0x20, 0x1004e414}, {0x21, 0x1004ec54}, {0x22, 0x1004e578}, {0x23, 0x1004e4a8}, {0x24, 0x1004e4d0}, {0x25, 0x1004e548}, {0x26, 0x1004e658}, {0x27, 0x1004e730}, {0x28, 0x1004e4f0}, {0x29, 0x1004e498}, {0x2a, 0x1004e550}, {0x2b, 0x1004e4a0}, {0x2c, 0x1004e4d8}, {0x2d, 0x1004e6b0}, {0x2e, 0x1004e5c0}, {0x2f, 0x1004e4e8}, {0x30, 0x1004e4c8}, {0x31, 0x1004e41c}, {0x32, 0x1004e5c0}, {0x33, 0x1004e510}, {0x35, 0x1004e580}, {0x36, 0x1004e590}, {0x36, 0x1004e558}, {0x3c, 0x1004e560}, {0x3f, 0x1004e440}, {0x42, 0x1004e518}, {0x43, 0x1004e520}, {0x4c, 0x1004e4b8}, {0x4d, 0x1004e4b0}, {0x4e, 0x1004e698}, {0x50, 0x1004e430}, {0x51, 0x1004e538}, {0x52, 0x1004e530},
		{0x53, 0x1004e528}, {0x54, 0x1004e5b0}, {0x55, 0x1004e5b0}, {0x56, 0x1004e5b8}, {0x57, 0x1004e5a0}, {0x58, 0x1004e5a8}, {0x59, 0x1004e598}, {0x61, 0x1004e570}, {0x62, 0x1004e600}, {0x63, 0x1004e608}, {0x64, 0x1004e610}, {0x65, 0x1004e618}, {0x66, 0x1004e620}, {0x67, 0x1004e628}, {0x68, 0x1004e630}, {0x69, 0x1004ec24}, {0x6a, 0x1004ec3c}, {0x6b, 0x1004ec48}, {0x6c, 0x1004e518}, {0x6d, 0x1004e518}, {0x6e, 0x1004e518}, {0x6f, 0x1004e518}, {0x70, 0x1004e518}, {0x71, 0x1004e518}, {0x72, 0x1004e518}, {0x77, 0x1004e434}, {0x78, 0x1004e434}, {0x79, 0x1004e638}, {0x7a, 0x1004e638}, {0x7b, 0x1004e638}, {0x7c, 0x1004e638}, {0x7d, 0x1004e638}, {0x7e, 0x1004e638}, {0x7f, 0x1004e638}, {0x80, 0x1004e638}, {0x98, 0x1004e5e0},
		{0x99, 0x1004e5e8}, {0x9a, 0x1004e5f0}, {0x9b, 0x1004e5f8}, {0x9c, 0x1004e688}, {0x9d, 0x1004e500}, {0x9e, 0x1004e4f8}, {0x9f, 0x1004e658}, {0xa0, 0x1004e518}, {0xa1, 0x1004e518}, {0xa2, 0x1004e518}, {0xa3, 0x1004e660}, {0xa4, 0x1004e668}, {0xa5, 0x1004e670}, {0xa6, 0x1004e678}, {0xa7, 0x1004e680}, {0xaa, 0x028f87f4}, {0xab, 0x1004e470}, {0xac, 0x1004e478}, {0xad, 0x1004e490}, {0xae, 0x1004e4a0}, {0xaf, 0x1004e480}, {0xb0, 0x1004e488}, {0xb3, 0x1004e5d8}, {0xb4, 0x1004e5d8}, {0xb5, 0x1004e5d8}, {0xb6, 0x1004e5d8}, {0xb7, 0x1004e5d8}, {0xb8, 0x1004e5d8}, {0xb9, 0x1004e5d8}, {0xba, 0x1004e5d8}, {0xbb, 0x1004e5d8}, {0xbc, 0x1004e5d8}, {0xbd, 0x1004e5d8},
		{0xbe, 0x1004e5d8}, {0xbf, 0x1004e5d8}, {0xc0, 0x1004e5d8}, {0xc1, 0x1004e5d8}, {0xc2, 0x1004e588}, {0xc3, 0x1004e588}, {0xc4, 0x1004e588}, {0xc5, 0x1004e588}, {0xc6, 0x1004e588}, {0xc7, 0x1004e588}, {0xc8, 0x1004e588}, {0xc9, 0x1004e588}, {0xca, 0x1004e588}, {0xcb, 0x1004e468}, {0xcc, 0x1004e640}, {0xcd, 0x1004e640}, {0xce, 0x1004e640}, {0xcf, 0x1004e640}, {0xd0, 0x1004e640}, {0xd1, 0x1004e640}, {0xd2, 0x1004e640}, {0xd3, 0x1004e640}, {0xd4, 0x1004e640}, {0xd5, 0x1004e640}, {0xd6, 0x1004e640}, {0xd7, 0x1004e640}, {0xd8, 0x1004e640}, {0xd9, 0x1004e640}, {0xda, 0x1004e640}, {0xdb, 0x1004e650}, {0xdc, 0x1004e468}, {0xdd, 0x1004e640}, {0xde, 0x1004e640}, {0xdf, 0x1004e640}, {0xe0, 0x1004e640}, {0xe1, 0x1004e640}, {0xe2, 0x1004e640},
		{0xe3, 0x1004e640}, {0xe4, 0x1004e640}, {0xe5, 0x1004e640}, {0xe6, 0x1004e640}, {0xe7, 0x1004e640}, {0xe8, 0x1004e640}, {0xe9, 0x1004e640}, {0xea, 0x1004e640}, {0xeb, 0x1004e640}, {0xec, 0x1004e640}, {0xed, 0x1004e648}, {0xee, 0x1004e648}, {0xef, 0x1004e648}, {0xf0, 0x1004e648}, {0xf1, 0x1004e648}, {0xf2, 0x1004e648}, {0xf3, 0x1004e648}, {0xf4, 0x1004e648}, {0xf5, 0x1004e648}, {0xf6, 0x1004e648}, {0xf7, 0x1004e648}, {0xf8, 0x1004e648}, {0xf9, 0x1004e648}, {0xfa, 0x1004e638}, {0xfb, 0x1004e648}, {0xfc, 0x1004e638}, {0xfd, 0x1004e648}, {0xfe, 0x1004e638}
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
		uint32_t szs_name_pointer = szs_name_pointers.at(item_id_to_copy_from);
		int section_start = 0x10000000;

		if (item_id == 0xAA) {
			szs_name_pointer = 0x028f87f4; //issues with custom .szs currently, may need to use sword model instead
			item_resources_addr_to_fix = item_resources_list_start + item_id * 0x24;
			section_start = 0x02000000; //custom stuff only gets put in .text
		}

		write_u32_to_rpx(AddressToOffset(field_item_resources_addr), szs_name_pointer);
		if (item_resources_addr_to_fix) {
			write_u32_to_rpx(AddressToOffset(item_resources_addr_to_fix), szs_name_pointer);
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
		entry.replace(0, 4, (char*)&relocation.r_offset, 4);
		entry.replace(4, 4, (char*)&relocation.r_info, 4);
		entry.replace(8, 4, (char*)&relocation.r_addend, 4);
		outRPX.extend_section(9, entry);

		if (item_resources_addr_to_fix) {
			Elf32_Rela relocation2;
			relocation2.r_offset = item_resources_addr_to_fix;
			relocation2.r_info = relocation.r_info; //same as first entry
			relocation2.r_addend = relocation.r_addend; //same as first entry

			std::string entry2;
			entry2.resize(12);
			entry2.replace(0, 4, (char*)&relocation2.r_offset, 4);
			entry2.replace(4, 4, (char*)&relocation2.r_info, 4);
			entry2.replace(8, 4, (char*)&relocation2.r_addend, 4);
			outRPX.extend_section(9, entry2);
		}

		std::vector<uint8_t> data1 = read_rpx_bytes(AddressToOffset(item_resources_addr_to_copy_from + 8), 0xD);
		std::vector<uint8_t> data2 = read_rpx_bytes(AddressToOffset(item_resources_addr_to_copy_from + 0x1C), 4);
		write_bytes_to_rpx(AddressToOffset(field_item_resources_addr + 4), data1);
		write_bytes_to_rpx(AddressToOffset(field_item_resources_addr + 0x14), data2);
		if (item_resources_addr_to_fix) {
			write_bytes_to_rpx(AddressToOffset(item_resources_addr_to_fix + 8), data1);
			write_bytes_to_rpx(AddressToOffset(item_resources_addr_to_fix + 0x1C) , data2);
		}
	}

	for (const uint32_t& address : { 0x0255220CU, 0x02552214U, 0x0255221CU, 0x02552224U, 0x0255222CU, 0x02552234U, 0x02552450U }) { //unsigned to make compiler happy
		write_u32_to_rpx(AddressToOffset(address), 0x60000000);
	}

	nlohmann::json patches = Load_Patches("../asm/patches/FieldItems.json"); //update paths
	//Apply_Patch(patches, "Fix Ground Item Exec");

	write_u32_to_rpx(AddressToOffset(0x0007a2d0, 7), 0x00011ed8); //Update the Y offset that is being read (.rela.text edit)

	uint32_t extra_item_data_list_start = 0x101e8674;
	for (int item_id = 0x00; item_id < 0xFF + 1; item_id++) {
		uint32_t item_extra_data_entry_addr = extra_item_data_list_start + 4 * item_id;
		uint8_t original_y_offset = read_rpx_u8(AddressToOffset(item_extra_data_entry_addr + 1));
		if (original_y_offset == 0) {
			write_u8_to_rpx(AddressToOffset(item_extra_data_entry_addr + 1), 0x28);
		}
		uint8_t original_radius = read_rpx_u8(AddressToOffset(item_extra_data_entry_addr + 2));
		if (original_radius == 0) {
			write_u8_to_rpx(AddressToOffset(item_extra_data_entry_addr + 2), 0x28);
		}
	}

	write_u32_to_rpx(AddressToOffset(0x02182d84), 0x4BFFE865);
	write_u32_to_rpx(AddressToOffset(0x02182da8), 0x41800030);
	write_u32_to_rpx(AddressToOffset(0x02182db8), 0x4bffe831);
	//02182e58 (item ids 16-34, no 1e,1f), custom branch to check if <= 0x20, use itemactionforarrow, else use rupee, branch back
	//Add vscroll.szs

	return;
}

void remove_shop_item_forced_uniqueness_bit() {
	int shop_item_data_list_start = 0x101eaea4;

	for (const int& shop_item_index : { 0x0, 0xB, 0xC, 0xD }) {
		int shop_item_data_addr = shop_item_data_list_start + shop_item_index * 0x10;
		uint8_t buy_requirements_bitfield = read_rpx_u8(AddressToOffset(shop_item_data_addr + 0xC));
		buy_requirements_bitfield = (buy_requirements_bitfield & (~2));
		write_u8_to_rpx(AddressToOffset(shop_item_data_addr + 0xC), buy_requirements_bitfield);
	}
}

void remove_ff2_cutscenes() { //could be done with dzx code instead, hardcoded offsets are faster and simple enough for this instance
	RandoSession::fspath path = "content/Common/Stage/M2tower_Room0.szs@YAZ0";
	std::string pathString = path.string();
	g_session.openGameFile(sepPath(pathString, '@'), path);
	std::ofstream fptr(g_session.relToExtract(pathString, '@'), std::ios::binary);

	fptr.seekp(0xf0a77c, std::ios::beg);
	fptr.write((char*)0xFF, 1); //Remove the cutscene with rescuing Aryll

	fptr.seekp(0xf098d0, std::ios::beg);
	fptr.write("sea\x00\x00\x00\x00\x00", 8);
	fptr.write((char*)(uint16_t)0x0001, 2);
	g_session.repackGameFile(sepPath(pathString, '@'));
	return;
}

void make_items_progressive() {
	nlohmann::json patches = Load_Patches("../asm/patches/ProgressiveItems.json"); //Create this file once the offsets are finalized
	//Apply_Patch(patches, "Make Items Progressive");

	int item_get_func_pointer = 0x0001da54; //First relevant relocation entry in .rela.data (overwrites .data section when loaded)

	for (const int sword_id : {0x38, 0x39, 0x3A, 0x3D, 0x3E}) {
		int item_get_func_addr = item_get_func_pointer + sword_id * 0xC + 8;
		write_u32_to_rpx(AddressToOffset(item_get_func_addr, 7), 0x11111111); //replace value with offset in .text section to custom func (REPLACES DATA IN .rela)
	}
	for (const int shield_id : {0x3B, 0x3C}) {
		int item_get_func_addr = item_get_func_pointer + (shield_id * 0xC) + 8;
		write_u32_to_rpx(AddressToOffset(item_get_func_addr, 7), 0x11111111); //replace value with offset in .text section to custom func (REPLACES DATA IN .rela)
	}
	for (const int bow_id : {0x27, 0x35, 0x36}) {
		int item_get_func_addr = item_get_func_pointer + bow_id * 0xC + 8;
		write_u32_to_rpx(AddressToOffset(item_get_func_addr, 7), 0x11111111); //replace value with offset in .text section to custom func (REPLACES DATA IN .rela)
	}
	for (const int wallet_id : {0xAB, 0xAC}) {
		int item_get_func_addr = item_get_func_pointer + wallet_id * 0xC + 8;
		write_u32_to_rpx(AddressToOffset(item_get_func_addr, 7), 0x11111111); //replace value with offset in .text section to custom func (REPLACES DATA IN .rela)
	}
	for (const int bomb_bag_id : {0xAD, 0xAE}) {
		int item_get_func_addr = item_get_func_pointer + bomb_bag_id * 0xC + 8;
		write_u32_to_rpx(AddressToOffset(item_get_func_addr, 7), 0x11111111); //replace value with offset in .text section to custom func (REPLACES DATA IN .rela)
	}
	for (const int quiver_id : {0xAF, 0xB0}) {
		int item_get_func_addr = item_get_func_pointer + quiver_id * 0xC + 8;
		write_u32_to_rpx(AddressToOffset(item_get_func_addr, 7), 0x11111111); //replace value with offset in .text section to custom func (REPLACES DATA IN .rela)
	}
	for (const int picto_id : {0x23, 0x26}) {
		int item_get_func_addr = item_get_func_pointer + picto_id * 0xC + 8;
		write_u32_to_rpx(AddressToOffset(item_get_func_addr, 7), 0x11111111); //replace value with offset in .text section to custom func (REPLACES DATA IN .rela)
	}

	write_u32_to_rpx(AddressToOffset(0x0254e8c4), 0x60000000);
	write_u32_to_rpx(AddressToOffset(0x0254e8cc), 0x60000000);
	write_u32_to_rpx(AddressToOffset(0x0254e66c), 0x60000000);
	write_u32_to_rpx(AddressToOffset(0x0254e674), 0x60000000);

	write_u32_to_rpx(AddressToOffset(0x0254e96c), 0x60000000);
	write_u32_to_rpx(AddressToOffset(0x0254e97c), 0x60000000);

	return;
}

void add_ganons_tower_warp_to_ff2() {
	RandoSession::fspath path = "content/Common/Pack/szs_permanent1.pack@SARC@sea_Room1.szs@YAZ0";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::DZXFile dzr;
	dzr.loadFromBinary(fptr);
	ChunkEntry& warp = dzr.add_entity("ACTR", 1);
	warp.data = "Warpmj\x00\x00\x00\x00\x00\x11\xc8\x93\x0f\xd9\x00\x00\x00\x00\xc8\x91\xf7\xfa\x00\x00\x00\x00\x00\x00\xff\xff";
	dzr.writeToFile(g_session.relToExtract(pathString, '@'));

	g_session.repackGameFile(sepPath(pathString, '@'));
	return;
}

void add_chest_in_place_medli_gift() {
	RandoSession::fspath path = "content/Common/Stage/M_Dra09_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::DZXFile dzs;
	dzs.loadFromBinary(fptr);
	ChunkEntry& chest = dzs.add_entity("TRES");
	chest.data = "takara3\x00\xFF\x20\x08\x80\xc4\xca\x99\xec\x46\x54\x80\x00\x43\x83\x84\x5a\x00\x09\xcc\x16\x0f\xff\xff\xff";
	dzs.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	RandoSession::fspath path2 = "content/Common/Stage/M_NewD2_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs";
	std::string pathString2 = path.string();
	std::ifstream fptr2 = g_session.openGameFile(sepPath(pathString2, '@'), path2);

	FileTypes::DZXFile dzs2;
	dzs2.loadFromBinary(fptr2);
	ChunkEntry& dummyChest = dzs2.add_entity("TRES");
	dummyChest.data = "takara3\x00\xFF\x20\x08\x80\xc4\xca\x99\xec\x46\x54\x80\x00\x43\x83\x84\x5a\x00\x09\xcc\x16\x0f\xff\xff\xff";
	dzs2.writeToFile(g_session.relToExtract(pathString2, '@'));
	g_session.repackGameFile(sepPath(pathString2, '@'));
	return;
}

void add_chest_in_place_queen_fairy_cutscene() {
	RandoSession::fspath path = "content/Common/Pack/szs_permanent2.pack@SARC@sea_Room9.szs@YAZ0@SARC@Room9.bfres@BFRES@room.dzr";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::DZXFile dzr;
	dzr.loadFromBinary(fptr);
	ChunkEntry& chest = dzr.add_entity("TRES");
	chest.data = "takara3\x00\xFF\x20\x0e\x00\xc8\x2f\xcf\xc0\x44\x34\xc0\x00\xc8\x43\x4e\xc0\x00\x09\x10\x00\xa5\xff\xff\xff";
	dzr.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	return;
}

void add_more_magic_jars() {
	{
		RandoSession::fspath path = "content/Common/Stage/M_NewD2_Room2.szs@YAZ0@SARC@Room2.bfres@BFRES@room.dzr";
		std::string pathString = path.string();
		std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

		FileTypes::DZXFile drc_hub;
		drc_hub.loadFromBinary(fptr);
		std::vector<ChunkEntry*> actors = drc_hub.entries_by_type("ACTR");
		std::vector<ChunkEntry*> skulls;
		for (ChunkEntry* actor : actors) {
			if (strncmp(&actor->data[0], "Odokuro\x00", 8)) skulls.push_back(actor);
		}
		skulls[2]->data.replace(0x8, 0x4, "\x75\x7f\xff\x09", 0, 4);
		skulls[5]->data.replace(0x8, 0x4, "\x75\x7f\xff\x0A", 0, 4);
		drc_hub.writeToFile(g_session.relToExtract(pathString, '@'));
		g_session.repackGameFile(sepPath(pathString, '@'));
		fptr.close();
	}

	{
		RandoSession::fspath path = "content/Common/Stage/M_NewD2_Room10.szs@YAZ0@SARC@Room10.bfres@BFRES@room.dzr";
		std::string pathString = path.string();
		std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

		FileTypes::DZXFile drc_before_boss;
		drc_before_boss.loadFromBinary(fptr);
		std::vector<ChunkEntry*> actors = drc_before_boss.entries_by_type("ACTR");
		std::vector<ChunkEntry*> skulls;
		for (ChunkEntry* actor : actors) {
			if (strncmp(&actor->data[0], "Odokuro\x00", 8)) skulls.push_back(actor);
		}
		skulls[0]->data.replace(0x8, 0x4, "\x75\x7f\xff\x0A", 0, 4);
		skulls[9]->data.replace(0x8, 0x4, "\x75\x7f\xff\x0A", 0, 4);
		drc_before_boss.writeToFile(g_session.relToExtract(pathString, '@'));
		g_session.repackGameFile(sepPath(pathString, '@'));
		fptr.close();
	}

	{
		RandoSession::fspath path = "content/Common/Pack/szs_permanent1.pack@SARC@sea_Room13.szs@YAZ0@SARC@Room13.bfres@BFRES@room.dzr";
		std::string pathString = path.string();
		std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

		FileTypes::DZXFile dri;
		dri.loadFromBinary(fptr);
		ChunkEntry& grass1 = dri.add_entity("ACTR");
		grass1.data = "\x6B\x75\x73\x61\x78\x31\x00\x00\x00\x00\x0E\x00\x48\x4C\xC7\x80\x44\xED\x80\x00\xC8\x45\xB7\xC0\x00\x00\x00\x00\x00\x00\xFF\xFF";
		ChunkEntry& grass2 = dri.add_entity("ACTR");
		grass2.data = "\x6B\x75\x73\x61\x78\x31\x00\x00\x00\x00\x0E\x00\x48\x4C\x6D\x40\x44\xA2\x80\x00\xC8\x4D\x38\x40\x00\x00\x00\x00\x00\x00\xFF\xFF";
		dri.writeToFile(g_session.relToExtract(pathString, '@'));
		g_session.repackGameFile(sepPath(pathString, '@'));
		fptr.close();
	}

	{
		RandoSession::fspath path = "content/Common/Stage/Siren_Room14.szs@YAZ0@SARC@Room14.bfres@BFRES@room.dzr";
		std::string pathString = path.string();
		std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

		FileTypes::DZXFile totg;
		totg.loadFromBinary(fptr);
		std::vector<ChunkEntry*> actors = totg.entries_by_type("ACTR");
		std::vector<ChunkEntry*> pots;
		for (ChunkEntry* actor : actors) {
			if (strncmp(&actor->data[0], "kotubo\x00\x00", 8)) pots.push_back(actor);
		}
		pots[1]->data = "\x6B\x6F\x74\x75\x62\x6F\x00\x00\x70\x7F\xFF\x0A\xC5\x6E\x20\x00\x43\x66\x00\x05\xC5\xDF\xC0\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF";
		totg.writeToFile(g_session.relToExtract(pathString, '@'));
		g_session.repackGameFile(sepPath(pathString, '@'));
		fptr.close();
	}
}

void modify_title_screen() {
	RandoSession::fspath path = "content/Common/Layout/Title_00.szs@YAZ0@SARC@blyt/Title_00.bflyt";
	std::string pathString = path.string();
	g_session.openGameFile(sepPath(pathString, '@'), path);
	std::string outPath = g_session.relToExtract(pathString, '@');
	std::ofstream outFile(outPath, std::ios::binary);
	outFile.seekp(0x8A4, std::ios::beg);
	outFile.write("\xC3\x25\x00\x00\x41\x40\x00\x00", 8);
	outFile.seekp(0x98C, std::ios::beg);
	outFile.write("\xc1\xF0\x00\x00", 4);
	outFile.seekp(0x9AC, std::ios::beg);
	outFile.write("\x42\xF0\x00\x00", 4);

	RandoSession::fspath subtitlePath = "content/Common/Layout/Title_00.szs@YAZ0@SARC@timg/TitleLogoWindwaker_00^l.bflim";
	std::string subPathString = subtitlePath.string();
	g_session.openGameFile(sepPath(subPathString, '@'), subtitlePath);
	std::string subOutPath = g_session.relToExtract(subPathString, '@');
	std::ofstream subOut(subOutPath, std::ios::binary);

	std::ifstream newSubtitle("../assets/TitleLogoWindwaker_00^l.bflim", std::ios::binary);
	std::string subtitleData;
	subtitleData.resize(180264);
	newSubtitle.read(&subtitleData[0], 180264);
	subOut.write(&subtitleData[0], 180624);

	RandoSession::fspath maskPath = "content/Common/Layout/Title_00.szs@YAZ0@SARC@timg/TitleLogoWindwakerMask_00^s.bflim";
	std::string maskPathString = maskPath.string();
	g_session.openGameFile(sepPath(maskPathString, '@'), maskPath);
	std::string maskOutPath = g_session.relToExtract(maskPathString, '@');
	std::ofstream maskOut(maskOutPath, std::ios::binary);

	std::ifstream newMask("../assets/TitleLogoWindwakerMask_00^s.bflim", std::ios::binary);
	std::string maskData;
	maskData.resize(24616);
	newMask.read(&maskData[0], 24616);
	maskOut.write(&maskData[0], 24616);

	g_session.repackGameFile(sepPath(pathString, '@')); //repack the sarc

	//update sparkle size/position
	write_u32_to_rpx(AddressToOffset(0x101f7048), 0x3fb3333); //scale
	write_u32_to_rpx(AddressToOffset(0x101f7044), 0x40100000); //possibly particle size, JP changes it for its larger title text
	write_u32_to_rpx(AddressToOffset(0x10108280), 0xc2180000); //vertical position

	//add version number if reasonable, bflyt support
	return;
}

void update_name_and_icon() {
	std::string path = g_session.relToExtract("meta/iconTex.tga", '@'); //doesnt do any extraction, just used to convert to absolute path
	std::ofstream icon(path, std::ios::binary);
	std::ifstream newIcon("../assets/iconTex.tga", std::ios::binary);

	std::string data;
	data.resize(65584);
	newIcon.read(&data[0], 65584);
	icon.write(&data[0], 65584);

	std::string metaPath = g_session.relToExtract("meta/meta.xml", '@'); //doesnt do any extraction, just used to convert to absolute path
	std::fstream xml(metaPath, std::ios::binary); //Binary to avoid any line ending issues
	std::string xmlData;
	xmlData.resize(0x256F);
	xml.read(&xmlData[0], 0x256F); //read xml into string so we can insert data
	xmlData.replace(0x12A7, 0x25, "THE LEGEND OF ZELDA\x0AThe Wind Waker HD Randomizer"); //Replace name for all languages
	xmlData.replace(0x1310, 0x25, "THE LEGEND OF ZELDA\x0AThe Wind Waker HD Randomizer");
	xmlData.replace(0x13EB, 0x25, "THE LEGEND OF ZELDA\x0AThe Wind Waker HD Randomizer");
	xmlData.replace(0x1501, 0x25, "THE LEGEND OF ZELDA\x0AThe Wind Waker HD Randomizer");

	xmlData.replace(0x161A, 0x11, "The Wind Waker HD Randomizer"); //Replace short name for all languages
	xmlData.replace(0x1671, 0x11, "The Wind Waker HD Randomizer");
	xmlData.replace(0x173E, 0x11, "The Wind Waker HD Randomizer");
	xmlData.replace(0x1848, 0x11, "The Wind Waker HD Randomizer");

	xml.seekp(0, std::ios::beg);
	xml.write(&xmlData[0], xmlData.size());
	return;
}

void allow_dungeon_items_to_appear_anywhere() {
	int item_get_func_pointer = 0x0001da54; //First relevant relocation entry in .rela.data (overwrites .data section when loaded)
	int item_resources_list_start = 0x101e4674;
	int field_item_resources_list_start = 0x101e6a74;

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

	const std::unordered_map<std::u16string, int> item_func_addresses{ { //update pointers when tested
		{u"DRC Small Key", 0x1},
		{u"FW Small Key", 0x1},
		{u"TotG Small Key", 0x1},
		{u"ET Small Key", 0x1},
		{u"WT Small Key", 0x1},
		{u"DRC Big Key", 0x1},
		{u"FW Big Key", 0x1},
		{u"TotG Big Key", 0x1},
		{u"ET Big Key", 0x1},
		{u"WT Big Key", 0x1},
		{u"DRC Dungeon Map", 0x1},
		{u"FW Dungeon Map", 0x1},
		{u"TotG Dungeon Map", 0x1},
		{u"FF Dungeon Map", 0x1},
		{u"ET Dungeon Map", 0x1},
		{u"WT Dungeon Map", 0x1},
		{u"DRC Compass", 0x1},
		{u"FW Compass", 0x1},
		{u"TotG Compass", 0x1},
		{u"FF Compass", 0x1},
		{u"ET Compass", 0x1},
		{u"WT Compass", 0x1}
	} };

	const std::unordered_map<int, int> szs_name_pointers{
		{0x15, 0x1004e448},
		{0x4C, 0x1004e4b8},
		{0x4D, 0x1004e4b0},
		{0x4E, 0x1004e698}
	};

	RandoSession::fspath path = "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::MSBTFile msbt;
	msbt.loadFromBinary(fptr);
	for (const dungeon_item_info& item_data : dungeon_items) {
		std::u16string item_name = item_data.short_name + u" " + item_data.base_item_name;
		uint8_t base_item_id = item_name_to_id.at(item_data.base_item_name);
		std::u16string dungeon_name = dungeon_names.at(item_data.short_name);

		write_u32_to_rpx(AddressToOffset(item_get_func_pointer + 0x8 + (0xC * item_data.item_id)), item_func_addresses.at(item_name) - 0x02000000); //write to the relocation entries
		
		int message_id = 101 + item_data.item_id;
		Message& to_copy = msbt.messages_by_label["00" + std::to_string(101 + base_item_id)];
		if (item_data.base_item_name == u"Small Key") {
			std::u16string article = get_indefinite_article(dungeon_name); //this is to avoid a duplicate function call
			std::u16string message(DRAW_INSTANT u"You got " + article + u" " TEXT_COLOR_RED + dungeon_name + u" small key" TEXT_COLOR_WHITE u"!", 39 + article.size() + dungeon_name.size()); //calculate string length for initalizer to handle null characters inside string
			//word_wrap_string
			msbt.addMessage("00" + std::to_string(message_id), to_copy.attributes, to_copy.style, message);
		} 
		else if (item_data.base_item_name == u"Big Key") {
			std::u16string message(DRAW_INSTANT u"You got the " TEXT_COLOR_RED + dungeon_name + u" Big Key" TEXT_COLOR_WHITE u"!", 40 + dungeon_name.size()); //calculate string length for initalizer to handle null characters inside string
			//word_wrap_string
			msbt.addMessage("00" + std::to_string(message_id), to_copy.attributes, to_copy.style, message);
		}
		else if (item_data.base_item_name == u"Dungeon Map") {
			std::u16string message(DRAW_INSTANT u"You got the " TEXT_COLOR_RED + dungeon_name + u" Dungeon Map" TEXT_COLOR_WHITE u"!", 44 + dungeon_name.size()); //calculate string length for initalizer to handle null characters inside string
			//word_wrap_string
			msbt.addMessage("00" + std::to_string(message_id), to_copy.attributes, to_copy.style, message);
		}
		else if (item_data.base_item_name == u"Compass") {
			std::u16string message(DRAW_INSTANT u"You got the " TEXT_COLOR_RED + dungeon_name + u" Compass" TEXT_COLOR_WHITE u"!", 40 + dungeon_name.size()); //calculate string length for initalizer to handle null characters inside string
			//word_wrap_string
			msbt.addMessage("00" + std::to_string(message_id), to_copy.attributes, to_copy.style, message);
		}

		uint32_t item_resources_addr_to_copy_from = item_resources_list_start + base_item_id * 0x24;
		uint32_t field_item_resources_addr_to_copy_from = field_item_resources_list_start + base_item_id * 0x1c;

		uint32_t item_resources_addr = item_resources_list_start + item_data.item_id * 0x24;
		uint32_t field_item_resources_addr = item_resources_list_start + item_data.item_id * 0x24;

		uint32_t szs_name_pointer = szs_name_pointers.at(base_item_id);

		write_u32_to_rpx(AddressToOffset(field_item_resources_addr), szs_name_pointer);
		write_u32_to_rpx(AddressToOffset(item_resources_addr), szs_name_pointer);

		//need relocation entries so pointers work on console
		Elf32_Rela relocation;
		relocation.r_offset = field_item_resources_addr;
		relocation.r_info = 0x00000201;
		relocation.r_addend = szs_name_pointer - 0x10000000; //needs offset into .rodata section, subtract start address from data location

		std::string entry;
		entry.resize(12);
		entry.replace(0, 4, (char*)&relocation.r_offset, 4);
		entry.replace(4, 4, (char*)&relocation.r_info, 4);
		entry.replace(8, 4, (char*)&relocation.r_addend, 4);
		outRPX.extend_section(9, entry);

		Elf32_Rela relocation2;
		relocation2.r_offset = item_resources_addr;
		relocation2.r_info = relocation.r_info; //same as first entry
		relocation2.r_addend = relocation.r_addend; //same as first entry

		std::string entry2;
		entry2.resize(12);
		entry2.replace(0, 4, (char*)&relocation2.r_offset, 4);
		entry2.replace(4, 4, (char*)&relocation2.r_info, 4);
		entry2.replace(8, 4, (char*)&relocation2.r_addend, 4);
		outRPX.extend_section(9, entry2);

		std::vector<uint8_t> data1 = read_rpx_bytes(AddressToOffset(item_resources_addr_to_copy_from + 8), 0xD);
		write_bytes_to_rpx(AddressToOffset(item_resources_addr + 8), data1);
		write_bytes_to_rpx(AddressToOffset(field_item_resources_addr + 4), data1);
		std::vector<uint8_t> data2 = read_rpx_bytes(AddressToOffset(item_resources_addr_to_copy_from + 0x1C), 4);
		write_bytes_to_rpx(AddressToOffset(item_resources_addr + 0x1C), data2);
		write_bytes_to_rpx(AddressToOffset(field_item_resources_addr + 0x14), data2);

		std::vector<uint8_t> data3 = read_rpx_bytes(AddressToOffset(item_resources_addr_to_copy_from + 0x15), 0x7);
		write_bytes_to_rpx(AddressToOffset(item_resources_addr + 0x15), data3);
		std::vector<uint8_t> data4 = read_rpx_bytes(AddressToOffset(item_resources_addr_to_copy_from + 0x20), 0x4);
		write_bytes_to_rpx(AddressToOffset(item_resources_addr + 0x20), data4);

		std::vector<uint8_t> data5 = read_rpx_bytes(AddressToOffset(field_item_resources_addr_to_copy_from + 0x11), 0x3);
		write_bytes_to_rpx(AddressToOffset(field_item_resources_addr + 0x11), data5);
		std::vector<uint8_t> data6 = read_rpx_bytes(AddressToOffset(field_item_resources_addr_to_copy_from + 0x18), 0x4);
		write_bytes_to_rpx(AddressToOffset(field_item_resources_addr + 0x18), data6);
		
	}
	msbt.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	return;
}

std::u16string word_wrap_string(std::u16string string, int max_line_len) {
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

std::string get_indefinite_article(std::string string) {
	char first_letter = std::tolower(string[0]);
	if (first_letter == 'a' || first_letter == 'e' || first_letter == 'i' || first_letter == 'o' || first_letter == 'u') {
		return "an";
	}
	else {
		return "a";
	}
}

std::u16string get_indefinite_article(std::u16string string) {
	char16_t first_letter = std::tolower(string[0]);
	if (first_letter == u'a' || first_letter == u'e' || first_letter == u'i' || first_letter == u'o' || first_letter == u'u') {
		return u"an";
	}
	else {
		return u"a";
	}
}

std::string upper_first_letter(std::string string) {
	string[0] = std::toupper(string[0]);
	return string;
}

std::u16string upper_first_letter(std::u16string string) {
	string[0] = std::toupper(string[0]);
	return string;
}

std::string pad_str_4_lines(std::string string) {
	std::vector<std::string> lines;
	unsigned int index = 0;
	while (index = string.find_first_of('\n'), index != std::string::npos) {
		lines.push_back(string.substr(0, index));
		string = string.substr(index + 1);
	}

	int padding_lines_needed = (4 - lines.size() % 4) % 4;
	for (int i = 0; i < padding_lines_needed; i++) {
		lines.push_back("");
	}

	std::string ret;
	for (const std::string& segment : lines) {
		ret = ret + segment + '\n';
	}

	return ret;
}

std::u16string pad_str_4_lines(std::u16string string) {
	std::vector<std::u16string> lines = split_lines(string);

	int padding_lines_needed = (4 - lines.size() % 4) % 4;
	for (int i = 0; i < padding_lines_needed; i++) {
		lines.push_back(u"");
	}

	return merge_lines(lines);
}

std::vector<std::string> split_lines(std::string string) {
	std::vector<std::string> lines;
	unsigned int index = 0;
	while (index = string.find_first_of('\n'), index != std::string::npos) {
		lines.push_back(string.substr(0, index));
		string = string.substr(index + 1);
	}

	return lines;
}

std::vector<std::u16string> split_lines(std::u16string string) {
	std::vector<std::u16string> lines;
	unsigned int index = 0;
	while (index = string.find_first_of('\n'), index != std::u16string::npos) {
		lines.push_back(string.substr(0, index));
		string = string.substr(index + 1);
	}

	return lines;
}

std::string merge_lines(std::vector<std::string> lines) {
	std::string ret;
	for (const std::string& segment : lines) {
		ret += segment + '\n';
	}

	return ret;
}

std::u16string merge_lines(std::vector<std::u16string> lines) {
	std::u16string ret;
	for (const std::u16string& segment : lines) {
		ret += segment + u'\n';
	}

	return ret;
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
		RandoSession::fspath filePath = path;
		std::ifstream in = g_session.openGameFile(sepPath(path, '@'), filePath);
		FileTypes::DZXFile room_dzr;
		room_dzr.loadFromBinary(in);
		for (ChunkEntry* spawn : room_dzr.entries_by_type("PLYR")) {
			uint8_t spawn_type = ((*(uint8_t*)&spawn->data[0xB]) & 0xF0) >> 4;
			if (spawn_type == 0x09) {
				spawn->data[0xB] = (spawn->data[0xB] & 0x0F) | 0x20;
			}
		}
		room_dzr.writeToFile(g_session.relToExtract(path, '@'));
		g_session.repackGameFile(sepPath(path, '@'));
	}
}

void fix_shop_item_y_offsets() {
	int shop_item_display_data_list_start = 0x1003a930;

	for (int id = 0; id < 0xFF; id++) {
		int display_data_addr = shop_item_display_data_list_start + id * 0x20;
		float y_offset = read_rpx_float(AddressToOffset(display_data_addr + 0x10));
		int ArrowID[] = { 0x10, 0x11, 0x12 };

		if (y_offset == 0 && std::find(std::begin(ArrowID), std::end(ArrowID), id) == std::end(ArrowID)) {
			//If the item didn't originally have a Y offset we need to give it one so it's not sunken into the pedestal.
			// Only exception are for items 10 11 and 12 - arrow refill pickups.Those have no Y offset but look fine already.
			float new_y_offset = 20.0f;
			write_float_to_rpx(AddressToOffset(display_data_addr + 0x10), new_y_offset);
		}
	}
}

void update_shop_item_descriptions(std::string beedle20Item, std::string beedle500Item, std::string beedle950Item, std::string beedle900Item) {
	RandoSession::fspath path = "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::MSBTFile msbt;
	msbt.loadFromBinary(fptr);
	
	//msbt.messages_by_label["03906"].text.message = u"\x0E\x00\x03\x02\x01" + item_name; //not finished

	msbt.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	return;
}

//hints, text updates

void shorten_zephos_event() {
	RandoSession::fspath path = "content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@event_list.dat";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::EventList event_list;
	event_list.loadFromBinary(fptr);
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

	event_list.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));
	//Untested, references might break

	return;
}

void update_korl_dialog() {
	RandoSession::fspath path = "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::MSBTFile msbt;
	msbt.loadFromBinary(fptr);
	msbt.messages_by_label["03443"].text.message = std::u16string(PLAYER_NAME u", the sea is all yours.\nMake sure you explore every corner\nin search of items to help you.Remember\nthat your quest is to defeat Ganondorf.\0", 144); //need to use constructor with length because of null characters
	msbt.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	return;
}

//starting shards, health, magic

void add_pirate_ship_to_windfall() {
	//https://github.com/LagoLunatic/wwrando/blob/master/tweaks.py#L1060
}

void add_cross_dungeon_warps() {
	std::array<CyclicWarpPotData, 3> loop1{ { {"M_NewD2", 2, 2185.0f, 0.0f, 590.0f, 0xA000, 2}, {"kindan", 1, 986.0f, 3956.43f, 9588.0f, 0xB929, 2}, {"Siren", 6, 277.0f, 229.42f, -6669.0f, 0xC000, 2} } };
	std::array<CyclicWarpPotData, 3> loop2{ { {"ma2room", 2, 1556.0f, 728.46f, -7091.0f, 0xEAA6, 5}, {"M_Dai", 1, -8010.0f, 1010.0f, -1610.0f, 0, 5}, {"kaze", 3, -4333.0f, 1100.0f, 48.0f, 0x4000, 5} } };

	uint8_t warp_index = 0;
	for (CyclicWarpPotData& warp : loop1) {
		RandoSession::fspath stagePath = "content/Common/Stage/" + warp.stage_name + "_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs";
		std::string pathString = stagePath.string();
		std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), stagePath);
		FileTypes::DZXFile stage;
		stage.loadFromBinary(fptr);

		RandoSession::fspath roomPath = "content/Common/Stage/" + warp.stage_name + "_Room" + std::to_string(warp.room_num) + ".szs@YAZ0@SARC@Room" + std::to_string(warp.room_num) + ".bfres@BFRES@room.dzr";
		std::string pathString2 = roomPath.string();
		std::ifstream fptr2 = g_session.openGameFile(sepPath(pathString2, '@'), roomPath);
		FileTypes::DZXFile room;
		room.loadFromBinary(fptr2);

		FileTypes::DZXFile* dzx_for_spawn;
		dzx_for_spawn = &room;

		Utility::byteswap_inplace(warp.x);
		Utility::byteswap_inplace(warp.y);
		Utility::byteswap_inplace(warp.z);
		Utility::byteswap_inplace(warp.y_rot);

		ChunkEntry& spawn = dzx_for_spawn->add_entity("PLYR");
		spawn.data = "Link\x00\x00\x00\x00\xFF\xFF\x70";
		spawn.data.resize(0x20);
		spawn.data[0xB] = (spawn.data[0xB] & ~0x3F) | (warp.room_num & 0x3F);
		spawn.data.replace(0xB, 1, (char*)&warp.x, 4);
		spawn.data.replace(0xB, 1, (char*)&warp.y, 4);
		spawn.data.replace(0xB, 1, (char*)&warp.z, 4);
		spawn.data.replace(0xB, 1, "\x00\x00", 2);
		spawn.data.replace(0xB, 1, (char*)&warp.y_rot, 2);
		spawn.data.replace(0xB, 1, "\xFF\x45\xFF\xFF", 4);

		std::vector<ChunkEntry*> spawns = dzx_for_spawn->entries_by_type("PLYR");
		std::vector<ChunkEntry*> spawn_id_69;
		for (ChunkEntry* spawn_to_check : spawns) {
			if (strncmp(&spawn_to_check->data[0x1D], "\x45", 1)) spawn_id_69.push_back(spawn_to_check);
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
			scls_exit.data.replace(0x9, 1, (char*)&other_warp.room_num, 1);
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
		warp_pot.data.replace(0x8, 4, (char*)&params, 4);
		warp_pot.data.replace(0xC, 4, (char*)&warp.x, 4);
		warp_pot.data.replace(0x10, 4, (char*)&warp.y, 4);
		warp_pot.data.replace(0x14, 4, (char*)&warp.z, 4);
		warp_pot.data.replace(0x18, 2, "\xFF\xFF", 4);
		warp_pot.data.replace(0x1A, 2, (char*)&warp.y_rot, 4);
		warp_pot.data.replace(0x1C, 4, "\xFF\xFF\xFF\xFF", 4);

		room.writeToFile(g_session.relToExtract(pathString, '@'));
		stage.writeToFile(g_session.relToExtract(pathString2, '@'));
		g_session.repackGameFile(sepPath(pathString, '@'));
		g_session.repackGameFile(sepPath(pathString2, '@'));

		warp_index++;
	}

	warp_index = 0;
	for (CyclicWarpPotData& warp : loop2) {
		RandoSession::fspath stagePath = "content/Common/Stage/" + warp.stage_name + "_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs";
		std::string pathString = stagePath.string();
		std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), stagePath);
		FileTypes::DZXFile stage;
		stage.loadFromBinary(fptr);

		RandoSession::fspath roomPath = "content/Common/Stage/" + warp.stage_name + "_Room" + std::to_string(warp.room_num) + ".szs@YAZ0@SARC@Room" + std::to_string(warp.room_num) + ".bfres@BFRES@room.dzr";
		std::string pathString2 = roomPath.string();
		std::ifstream fptr2 = g_session.openGameFile(sepPath(pathString2, '@'), roomPath);
		FileTypes::DZXFile room;
		room.loadFromBinary(fptr2);

		FileTypes::DZXFile* dzx_for_spawn;
		if (warp.stage_name == "M_Dai" || warp.stage_name == "kaze") {
			dzx_for_spawn = &stage;
		}
		else {
			dzx_for_spawn = &room;
		}

		Utility::byteswap_inplace(warp.x);
		Utility::byteswap_inplace(warp.y);
		Utility::byteswap_inplace(warp.z);
		Utility::byteswap_inplace(warp.y_rot);

		ChunkEntry& spawn = dzx_for_spawn->add_entity("PLYR");
		spawn.data = "Link\x00\x00\x00\x00\xFF\xFF\x70";
		spawn.data.resize(0x20);
		spawn.data[0xB] = (spawn.data[0xB] & ~0x3F) | (warp.room_num & 0x3F);
		spawn.data.replace(0xB, 1, (char*)&warp.x, 4);
		spawn.data.replace(0xB, 1, (char*)&warp.y, 4);
		spawn.data.replace(0xB, 1, (char*)&warp.z, 4);
		spawn.data.replace(0xB, 1, "\x00\x00", 2);
		spawn.data.replace(0xB, 1, (char*)&warp.y_rot, 2);
		spawn.data.replace(0xB, 1, "\xFF\x45\xFF\xFF", 4);

		std::vector<ChunkEntry*> spawns = dzx_for_spawn->entries_by_type("PLYR");
		std::vector<ChunkEntry*> spawn_id_69;
		for (ChunkEntry* spawn_to_check : spawns) {
			if (strncmp(&spawn_to_check->data[0x1D], "\x45", 1)) spawn_id_69.push_back(spawn_to_check);
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
			scls_exit.data.replace(0x9, 1, (char*)&other_warp.room_num, 1);
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
		warp_pot.data.replace(0x8, 4, (char*)&params, 4);
		warp_pot.data.replace(0xC, 4, (char*)&warp.x, 4);
		warp_pot.data.replace(0x10, 4, (char*)&warp.y, 4);
		warp_pot.data.replace(0x14, 4, (char*)&warp.z, 4);
		warp_pot.data.replace(0x18, 2, "\xFF\xFF", 2);
		warp_pot.data.replace(0x1A, 2, (char*)&warp.y_rot, 2);
		warp_pot.data.replace(0x1C, 4, "\xFF\xFF\xFF\xFF", 4);

		room.writeToFile(g_session.relToExtract(pathString, '@'));
		stage.writeToFile(g_session.relToExtract(pathString2, '@'));
		g_session.repackGameFile(sepPath(pathString, '@'));
		g_session.repackGameFile(sepPath(pathString2, '@'));

		warp_index++;
	}

	FileTypes::JPC drc, totg, ff;
	RandoSession::fspath path = "content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@BFRES@Pscene035.jpc";
	std::string pathString = path.string();
	std::ifstream drcIn = g_session.openGameFile(sepPath(pathString, '@'), path);

	path = "content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@BFRES@Pscene050.jpc";
	pathString = path.string();
	std::ifstream totgIn = g_session.openGameFile(sepPath(pathString, '@'), path);

	path = "content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@BFRES@Pscene043.jpc";
	pathString = path.string();
	std::ifstream ffIn = g_session.openGameFile(sepPath(pathString, '@'), path);

	drc.loadFromBinary(drcIn);
	totg.loadFromBinary(totgIn);
	ff.loadFromBinary(ffIn);

	for (const uint16_t particle_id : {0x8161, 0x8162, 0x8165, 0x8166, 0x8112}) {
		Particle& particle = drc.particles_by_id[particle_id];

		std::array<FileTypes::JPC*, 2> jpcs = { &totg, &ff };
		for (FileTypes::JPC* dest_jpc : jpcs) {
			dest_jpc->addParticle(particle);

			for (std::string texture_filename : particle.tdb1.texture_filenames) {
				if (dest_jpc->textures_by_filename.find(texture_filename) == dest_jpc->textures_by_filename.end()) {
					chunk tex = drc.textures_by_filename[texture_filename];

					TEX1 texClass;
					chunks::TEX1_read(texClass, tex);
					dest_jpc->addTexture(texClass);
				}
			}
		}
	}

	drc.writeToFile(g_session.relToExtract("content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@Pscene035.jpc", '@'));
	totg.writeToFile(g_session.relToExtract("content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@Pscene050.jpc", '@'));
	ff.writeToFile(g_session.relToExtract("content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@Pscene043.jpc", '@'));

	g_session.repackGameFile(sepPath(pathString, '@')); //repacks full bfres

	return;
}

void remove_makar_kidnapping() {
	RandoSession::fspath path = "content/Common/Stage/kaze_Room3.szs@YAZ0@SARC@Room3.bfres@BFRES@room.dzr";
	std::string pathString = path.string();
	std::ifstream dzrIn = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::DZXFile dzr;
	dzr.loadFromBinary(dzrIn);
	std::vector<ChunkEntry*> actors = dzr.entries_by_type("ACTR");

	ChunkEntry* switch_actor = (ChunkEntry*)0; //initialization is just to make compiler happy
	for (ChunkEntry* actor : actors) {
		if (strncmp(&actor->data[0], "AND_SW2\x00", 8)) switch_actor = actor;
	}
	dzr.remove_entity(switch_actor);

	for (ChunkEntry* actor : actors) {
		if (strncmp(&actor->data[0], "wiz_r\x00\x00\x00", 8)) {
			actor->data.replace(0xA, 1, "\xFF", 1);
		}
	}

	dzr.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));
}

void increase_crawl_speed() {
	//The 3.0 float crawling uses is shared with other things in HD, can't change it directly
	//Redirect both instances to load 6.0 from elsewhere
	write_u32_to_rpx(AddressToOffset(0x0014ec04, 7), 0x000355C4); //update .rela.text entry
	write_u32_to_rpx(AddressToOffset(0x0014ec4c, 7), 0x000355C4); //update .rela.text entry
	return;
}

//chart numbers

void increase_grapple_animation_speed() {
	write_u32_to_rpx(AddressToOffset(0x02170250), 0x394B000A);
	write_u32_to_rpx(AddressToOffset(0x00075170, 7), 0x00010ffc); //update .rela.text entry
	write_u32_to_rpx(AddressToOffset(0x100110c8), 0x41C80000);
	write_u32_to_rpx(AddressToOffset(0x021711d4), 0x390B0006);
	return;
}

void increase_block_move_animation() {
	write_u32_to_rpx(AddressToOffset(0x00153b00, 7), 0x00035AAC); //update .rela.text entries
	write_u32_to_rpx(AddressToOffset(0x00153b48, 7), 0x00035AAC);

	int offset = 0x101cb424;
	for (int i = 0; i < 13; i++) { //13 types of blocks total
		write_u16_to_rpx(AddressToOffset(offset + 0x04), 0x000C); // Reduce number frames for pushing to last from 20 to 12
		write_u16_to_rpx(AddressToOffset(offset + 0x0A), 0x000C); // Reduce number frames for pulling to last from 20 to 12
		offset += 0x9C;
	}
	return;
}

void increase_misc_animations() {
	//Float is shared, redirect it to read another float with the right value
	write_u32_to_rpx(AddressToOffset(0x00148820, 7), 0x000358D8);
	write_u32_to_rpx(AddressToOffset(0x001482a4, 7), 0x00035124);
	write_u32_to_rpx(AddressToOffset(0x00148430, 7), 0x000358D8);
	write_u32_to_rpx(AddressToOffset(0x00148310, 7), 0x00035AAC);
	write_u32_to_rpx(AddressToOffset(0x0014e2d4, 7), 0x00035530);

	write_u32_to_rpx(AddressToOffset(0x02508b50), 0x3880000A);
	return;
}

//starting clothes

void hide_ship_sail() {
	write_u32_to_rpx(AddressToOffset(0x02162B04), 0x4E800020);
	return;
}

void shorten_auction_intro_event() {
	RandoSession::fspath path = "content/Common/Stage/Orichh_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::EventList event_list;
	event_list.loadFromBinary(fptr);
	Event& auction_start_event = event_list.Events_By_Name["AUCTION_START"];
	std::optional<std::reference_wrapper<Actor>> actor = auction_start_event.get_actor("CAMERA");
	if (!actor.has_value()) {
		return;
	}
	Actor& camera = actor.value(); //to avoid calling .value() every time a member is accessed

	camera.actions.erase(camera.actions.begin() + 3, camera.actions.begin() + 5); //last iterator not inclusive, only erase actions 3-4

	event_list.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));
	//Untested, references might break

	return;
}

void disable_invisible_walls() {
	RandoSession::fspath path = "content/Common/Stage/M_NewD2_Room2.szs@YAZ0@SARC@Room2.bfres@BFRES@room.dzr";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::DZXFile dzr;
	dzr.loadFromBinary(fptr);
	std::vector<ChunkEntry*> scobs = dzr.entries_by_type("SCOB");

	for (ChunkEntry* scob : scobs) {
		if (strncmp(&scob->data[0], "Akabe\x00\x00\x00", 8)) {
			scob->data[0xB] = '\xFF';
		}
	}


	dzr.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));
	return;
}

//trials variable, sword mode, starting gear

void update_swordless_text() {
	RandoSession::fspath path = "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::MSBTFile msbt;
	msbt.loadFromBinary(fptr);
	msbt.messages_by_label["01128"].text.message = std::u16string(CAPITAL PLAYER_NAME u", you may not have the\nMaster Sword, but do not be afraid!\n\n\nThe hammer of the dead is all you\nneed to crush your foe...\n\n\nEven as his ball of fell magic bears down\non you, you can " TEXT_COLOR_RED u"knock it back\nwith an empty bottle" TEXT_COLOR_WHITE u"!\n\n...I am sure you will have a shot at victory!\0", 287);;
	msbt.messages_by_label["01590"].text.message = std::u16string(CAPITAL PLAYER_NAME u"! Do not run! Trust in the\npower of the Skull Hammer!\0", 62);
	msbt.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	return;
}

void add_hint_signs() {
	RandoSession::fspath path = "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::MSBTFile msbt;
	msbt.loadFromBinary(fptr);
	std::string new_message_label = "00847";
	Attributes attributes;
	attributes.character = 0xF; //sign
	attributes.boxStyle = 0x2;
	attributes.drawType = 0x1;
	attributes.screenPos = 0x2;
	attributes.lineAlignment = 3;
	TSY1Entry tsy;
	tsy.styleIndex = 0x12B;
	std::u16string message(IMAGE(u"\x0A") u"\0", 5); //right arrow
	msbt.addMessage(new_message_label, attributes, tsy, message);
	msbt.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	path = "content/Common/Stage/M_NewD2_Room2.szs@YAZ0@SARC@Room2.bfres@BFRES@room.dzr";
	pathString = path.string();
	fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::DZXFile dzr;
	dzr.loadFromBinary(fptr);
	std::vector<ChunkEntry*> actors = dzr.entries_by_type("ACTR");

	std::vector<ChunkEntry*> bomb_flowers;
	for (ChunkEntry* actor : actors) {
		if (strncmp(&actor->data[0], "BFlower", 8)) bomb_flowers.push_back(actor);
	}
	bomb_flowers[0]->data = "\x4B\x61\x6E\x62\x61\x6E\x00\x00\x00\x00\x03\x4F\x44\x34\x96\xEB\x42\x47\xFF\xFF\xC2\x40\xB0\x3A\x00\x00\x20\x00\x00\x00\xFF\xFF";

	dzr.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	return;
}

void prevent_door_boulder_softlocks() {
	RandoSession::fspath path = "content/Common/Stage/M_NewD2_Room13.szs@YAZ0@SARC@Room13.bfres@BFRES@room.dzr";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::DZXFile room13;
	room13.loadFromBinary(fptr);

	ChunkEntry& swc00 = room13.add_entity("SCOB");
	swc00.data = "\x53\x57\x5F\x43\x30\x30\x00\x00\x00\x03\xFF\x05\x45\x24\xB0\x00\x00\x00\x00\x00\x43\x63\x00\x00\x00\x00\xC0\x00\xFF\xFF\xFF\xFF\x20\x10\x10\xFF";
	room13.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	path = "content/Common/Stage/M_NewD2_Room14.szs@YAZ0@SARC@Room14.bfres@BFRES@room.dzr";
	pathString = path.string();
	fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::DZXFile room14;
	room14.loadFromBinary(fptr);

	swc00 = room14.add_entity("SCOB");
	swc00.data = "\x53\x57\x5F\x43\x30\x30\x00\x00\x00\x03\xFF\x06\xC5\x7A\x20\x00\x44\xF3\xC0\x00\xC5\x06\xC0\x00\x00\x00\xA0\x00\xFF\xFF\xFF\xFF\x20\x10\x10\xFF";
	room14.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	return;
}

void update_tingle_statue_item_get_funcs() {
	const uint32_t item_get_func_ptr = 0x0001da54; //First relevant relocation entry in .rela.data (overwrites .data section when loaded)
	const std::vector<uint32_t> item_func_ptr_list = { 0x0, 0x0, 0x0, 0x0, 0x0 }; //fill with item get func addresses IN ORDER

	for (const int statue_id : {0xA3, 0xA4, 0xA5, 0xA6, 0xA7}) {
		uint32_t item_func_addr = item_get_func_ptr + (statue_id * 0xC) + 8;
		uint32_t item_func_ptr = item_func_ptr_list[statue_id - 0xA3]; //convert statue id to index in list
		write_u32_to_rpx(AddressToOffset(item_func_addr, 9), item_func_ptr - 0x02000000);
	}
	return;
}

//rainbow rupee

//seed hash

//key bag

//required dungeon map markers

void add_chest_in_place_jabun_cutscene() {
	RandoSession::fspath path = "content/Common/Stage/Pjavdou_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::DZXFile dzr;
	dzr.loadFromBinary(fptr);
	ChunkEntry& raft = dzr.add_entity("ACTR");
	ChunkEntry& chest = dzr.add_entity("TRES");
	raft.data = "Ikada\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\xFF\xFF";
	chest.data = "takara3\x00\xFF\x2F\xF3\x05\x00\x00\x00\x00\x43\x96\x00\x00\xC3\x48\x00\x00\x00\x00\x80\x00\x05\xFF\xFF\xFF";
	dzr.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	return;
}

void add_jabun_obstacles_to_default_layer() {
	RandoSession::fspath path = "content/Common/Pack/szs_permanent2.pack@SARC@sea_Room44.szs@YAZ0@SARC@Room44.bfres@BFRES@room.dzr";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::DZXFile dzr;
	dzr.loadFromBinary(fptr);

	std::vector<ChunkEntry*> layer_5_actors = dzr.entries_by_type_and_layer("ACTR", 5);
	ChunkEntry* layer_5_door = layer_5_actors[0];
	ChunkEntry* layer_5_whirlpool = layer_5_actors[1];

	ChunkEntry& newDoor = dzr.add_entity("ACTR");
	ChunkEntry& newWhirlpool = dzr.add_entity("ACTR");
	newDoor.data = layer_5_door->data;
	newWhirlpool.data = layer_5_whirlpool->data;

	dzr.remove_entity(layer_5_door);
	dzr.remove_entity(layer_5_whirlpool);

	dzr.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	return;
}

void remove_jabun_stone_door_event() {
	RandoSession::fspath path = "content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@event_list.dat";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::EventList event_list;
	event_list.loadFromBinary(fptr);
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

	event_list.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));
	//Untested, references might break

	return;
}

void add_chest_in_place_master_sword() {
	RandoSession::fspath path = "content/Common/Stage/kenroom_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::DZXFile dzr;
	dzr.loadFromBinary(fptr);

	std::vector<ChunkEntry*> default_layer_actors = dzr.entries_by_type_and_layer("ACTR", default_layer);
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
	chest.data = "takara3\x00\xFF\x20\x50\x04\xc2\xf6\xfd\x71\xc5\x49\x40\x00\xc5\xf4\xe9\x0a\x00\x00\x00\x00\x6a\xff\xff\xff";

	dzr.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	return;
}

void update_spoil_sell_text() {
	RandoSession::fspath path = "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::MSBTFile msbt;
	msbt.loadFromBinary(fptr);
	std::vector<std::u16string> lines = split_lines(msbt.messages_by_label["03957"].text.message);
	lines[2] = u"And no Blue Chu Jelly, either!";
	msbt.messages_by_label["03957"].text.message = merge_lines(lines);
	msbt.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	return;
}

void fix_totg_warp_spawn() {
	RandoSession::fspath path = "content/Common/Stage/kenroom_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::DZXFile dzr;
	dzr.loadFromBinary(fptr);

	ChunkEntry* spawn = dzr.entries_by_type("PLYR")[9];
	spawn->data = "\x4C\x69\x6E\x6B\x00\x00\x00\x00\x32\xFF\x20\x1A\x47\xC3\x4F\x5F\x00\x00\x00\x00\xBF\xBE\xBF\x90\x00\x00\x00\x00\x01\x01\xFF\xFF";

	dzr.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));
}

void remove_phantom_ganon_req_for_reefs() {
	std::string path;
	for (int room_index : {24, 46, 22, 8, 37, 25}) {
		path = "content/Common/Stage/sea_Room" + std::to_string(room_index) + ".szs@YAZ0@SARC@Room" + std::to_string(room_index) + ".bfres@BFRES@room.dzr";
		RandoSession::fspath filePath = path;

		std::ifstream in = g_session.openGameFile(sepPath(path, '@'), filePath);
		FileTypes::DZXFile room_dzr;
		room_dzr.loadFromBinary(in);
		std::vector<ChunkEntry*> actors = room_dzr.entries_by_type("ACTR");
		for (ChunkEntry* actor : actors) {
			if (strncmp(&actor->data[0], "Ocanon\x00\x00", 8)) {
				if (strncmp(&actor->data[0xA], "\x2A", 1)) {
					actor->data[0xA] = '\xFF'; //check if this is right index
				}
			}
			else if (strncmp(&actor->data[0], "Oship\x00\x00\x00", 8)) {
				if (strncmp(&actor->data[0x18], "\x2A", 1)) {
					actor->data[0x18] = '\xFF'; //check if this is right index
				}
			}
		}
		room_dzr.writeToFile(g_session.relToExtract(path, '@'));
		g_session.repackGameFile(sepPath(path, '@'));
	}
	return;
}

void fix_ff_door() {
	int face_index = 0x1493;
	uint16_t new_prop_index = 0x0011;

	RandoSession::fspath path = "content/Common/Pack/szs_permanent1.pack@SARC@sea_Room1.szs@YAZ0@SARC@Room1.bfres@BFRES@room.dzb";
	std::string pathString = path.string();
	std::fstream fptr;
	g_session.openGameFile(sepPath(pathString, '@'), path);

	fptr.open(g_session.relToExtract(pathString, '@'), std::ios::binary);

	fptr.seekg(0xC, std::ios::beg);
	uint32_t face_list_offset;
	fptr.read((char*)&face_list_offset, 4);
	Utility::byteswap_inplace(face_list_offset);

	fptr.seekp((face_list_offset + face_index * 0xA) + 6, std::ios::beg);
	fptr.write((char*)&new_prop_index, 2);

	g_session.repackGameFile(sepPath(pathString, '@'));
	return;
}

void add_failsafe_id_0_spawns() {
	std::array<spawn_data, 32> spawns_to_copy{
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

	for (const spawn_data& spawn_info : spawns_to_copy) {
		std::string path = "content/Common/Stage/" + spawn_info.stage_name + "_Room" + std::to_string(spawn_info.room_num) + ".szs@YAZ0@SARC@Room" + std::to_string(spawn_info.room_num) + ".bfres@BFRES@room.dzr";
		RandoSession::fspath filePath = path;

		std::ifstream in = g_session.openGameFile(sepPath(path, '@'), filePath);
		FileTypes::DZXFile room_dzr;
		room_dzr.loadFromBinary(in);

		std::vector<ChunkEntry*> spawns = room_dzr.entries_by_type("PLYR");

		for (ChunkEntry* spawn : spawns) {
			if (spawn->data[0x1D] == (char)spawn_info.spawn_id_to_copy) {
				ChunkEntry& new_spawn = room_dzr.add_entity("PLYR");
				new_spawn.data = spawn->data;
				new_spawn.data[0x1D] = '\x00';
			}
		}

		room_dzr.writeToFile(g_session.relToExtract(path, '@'));
		g_session.repackGameFile(sepPath(path, '@'));
	}

	//https://github.com/LagoLunatic/wwrando/blob/master/tweaks.py#L2123 //complex, requires more effort

}

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

		RandoSession::fspath filePath = path;
		std::ifstream in = g_session.openGameFile(sepPath(path, '@'), filePath);
		FileTypes::DZXFile dzx;
		dzx.loadFromBinary(in);
		std::vector<ChunkEntry*> scobs = dzx.entries_by_type("SCOB");
		for (ChunkEntry* scob : scobs) {
			if (strncmp(&scob->data[0], "TagEv\x00\x00\x00", 8)) {
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
		dzx.writeToFile(g_session.relToExtract(path, '@'));
		g_session.repackGameFile(sepPath(path, '@'));
	}

	return;
}

void fix_stone_head_bugs() {
	uint32_t status_bits = read_rpx_u32(AddressToOffset(0x101ca100));
	Utility::byteswap_inplace(status_bits);
	status_bits &= ~0x00000080;
	Utility::byteswap_inplace(status_bits);
	write_u32_to_rpx(AddressToOffset(0x101ca100), status_bits);

	return;
}

void show_tingle_statues_on_quest_screen() {

	//change icon eventually

	RandoSession::fspath path = "content/Common/Pack/permanent_2d_UsEnglish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt";
	std::string pathString = path.string();
	std::ifstream fptr = g_session.openGameFile(sepPath(pathString, '@'), path);

	FileTypes::MSBTFile msbt;
	msbt.loadFromBinary(fptr);
	msbt.messages_by_label["00503"].text.message = u"Tingle Statues\0";
	msbt.messages_by_label["00703"].text.message = std::u16string(u"Golden statues of a mysterious dashing\n figure. They can be traded to " TEXT_COLOR_RED u"Ankle" TEXT_COLOR_WHITE u" on" TEXT_COLOR_RED u"Tingle Island" TEXT_COLOR_WHITE u" for a reward!\0", 137); //need to use constructor with length because of null characters
	msbt.writeToFile(g_session.relToExtract(pathString, '@'));
	g_session.repackGameFile(sepPath(pathString, '@'));

	return;
}



//this is just for testing 
int main() {

	//timing stuff
	//auto start = std::chrono::high_resolution_clock::now();
	//auto stop = std::chrono::high_resolution_clock::now();
	//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	//auto duration2 = duration.count();

	std::ifstream fptr;
	fptr.open("./asm/patch_diffs/Swordless_diff.txt", std::ios::in);

	//throws error with a relative file path
	nlohmann::json patches = nlohmann::json::parse(fptr);

	return 0;
}
