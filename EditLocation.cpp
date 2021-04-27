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

int EditActor(std::fstream& fptr, ACTR actor, uint8_t item_id, int offset) {

	actor.params = ((actor.params & 0xFFFFFF00) | (item_id));

	fptr.seekp(offset, std::ios::beg);
	fptr.write(actor.name, sizeof actor.name);

	actor.params = Utility::byteswap(actor.params);
	actor.x_pos = Utility::byteswap(actor.x_pos);
	actor.y_pos = Utility::byteswap(actor.y_pos);
	actor.z_pos = Utility::byteswap(actor.z_pos);
	actor.aux_params_1 = Utility::byteswap(actor.aux_params_1);
	actor.y_rot = Utility::byteswap(actor.y_rot);
	actor.aux_params_2 = Utility::byteswap(actor.aux_params_2);
	actor.enemy_number = Utility::byteswap(actor.enemy_number);

	fptr.write((char*)&actor.params, sizeof(actor.params));
	fptr.write((char*)&actor.x_pos, sizeof(actor.x_pos));
	fptr.write((char*)&actor.y_pos, sizeof(actor.y_pos));
	fptr.write((char*)&actor.z_pos, sizeof(actor.z_pos));
	fptr.write((char*)&actor.aux_params_1, sizeof(actor.aux_params_1));
	fptr.write((char*)&actor.y_rot, sizeof(actor.y_rot));
	fptr.write((char*)&actor.aux_params_2, sizeof(actor.aux_params_2));
	fptr.write((char*)&actor.enemy_number, sizeof(actor.enemy_number));
	fptr.close();

	return 1;
}

int EditScob(std::fstream & fptr, SCOB scob, uint8_t item_id, int offset) {

	scob.params = ((scob.params & 0xFFFFFF00) | (item_id));

	fptr.seekp(offset, std::ios::beg);
	fptr.write(scob.name, sizeof scob.name);

	scob.params = Utility::byteswap(scob.params);
	scob.x_pos = Utility::byteswap(scob.x_pos);
	scob.y_pos = Utility::byteswap(scob.y_pos);
	scob.z_pos = Utility::byteswap(scob.z_pos);
	scob.aux_params_1 = Utility::byteswap(scob.aux_params_1);
	scob.y_rot = Utility::byteswap(scob.y_rot);
	scob.aux_params_2 = Utility::byteswap(scob.aux_params_2);
	scob.enemy_number = Utility::byteswap(scob.enemy_number);

	fptr.write((char*)&scob.params, sizeof(scob.params));
	fptr.write((char*)&scob.x_pos, sizeof(scob.x_pos));
	fptr.write((char*)&scob.y_pos, sizeof(scob.y_pos));
	fptr.write((char*)&scob.z_pos, sizeof(scob.z_pos));
	fptr.write((char*)&scob.aux_params_1, sizeof(scob.aux_params_1));
	fptr.write((char*)&scob.y_rot, sizeof(scob.y_rot));
	fptr.write((char*)&scob.aux_params_2, sizeof(scob.aux_params_2));
	fptr.write((char*)&scob.enemy_number, sizeof(scob.enemy_number));
	fptr.write((char*)&scob.scale_x, sizeof(scob.scale_x));
	fptr.write((char*)&scob.scale_y, sizeof(scob.scale_y));
	fptr.write((char*)&scob.scale_z, sizeof(scob.scale_z));
	fptr.write((char*)&scob.padding, sizeof(scob.padding));
	fptr.close();

	return 1;
}
