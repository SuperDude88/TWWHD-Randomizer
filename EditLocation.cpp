#include <Structs.h>
#include <Import.h>
#include <fstream>
#include <Utility.h>

int EditChest(std::fstream& fptr, ACTR chest, uint8_t item_id, int offset) {

	chest.aux_params_2 = ((chest.aux_params_2 & 0x00FF) | (item_id << 8));

	fptr.seekp(offset, std::ios::beg);
	fptr.write(chest.name, sizeof chest.name);

	chest.params = Utility::byteswap(chest.params);
	chest.x_pos = Utility::byteswap(chest.x_pos);
	chest.y_pos = Utility::byteswap(chest.y_pos);
	chest.z_pos = Utility::byteswap(chest.z_pos);
	chest.aux_params_1 = Utility::byteswap(chest.aux_params_1);
	chest.y_rot = Utility::byteswap(chest.y_rot);
	chest.aux_params_2 = Utility::byteswap(chest.aux_params_2);
	chest.enemy_number = Utility::byteswap(chest.enemy_number);

	fptr.write((char*)&chest.params, sizeof(chest.params));
	fptr.write((char*)&chest.x_pos, sizeof(chest.x_pos));
	fptr.write((char*)&chest.y_pos, sizeof(chest.y_pos));
	fptr.write((char*)&chest.z_pos, sizeof(chest.z_pos));
	fptr.write((char*)&chest.aux_params_1, sizeof(chest.aux_params_1));
	fptr.write((char*)&chest.y_rot, sizeof(chest.y_rot));
	fptr.write((char*)&chest.aux_params_2, sizeof(chest.aux_params_2));
	fptr.write((char*)&chest.enemy_number, sizeof(chest.enemy_number));
	fptr.close();

	return 1;
}