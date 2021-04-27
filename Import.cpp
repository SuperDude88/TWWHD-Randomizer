#include <fstream>
#include <cstdint>
#include <Structs.h>
#include <Import.h>
#include <EditLocation.h>
#include <Utility.h>

ACTR ReadChest(std::fstream& fptr, int offset) {

	ACTR chest;

	fptr.seekg(offset, std::ios::beg);
	fptr.read(reinterpret_cast<char*>(&chest), sizeof chest);
	chest.params = Utility::byteswap(chest.params);
	chest.x_pos = Utility::byteswap(chest.x_pos);
	chest.y_pos = Utility::byteswap(chest.y_pos);
	chest.z_pos = Utility::byteswap(chest.z_pos);
	chest.aux_params_1 = Utility::byteswap(chest.aux_params_1);
	chest.y_rot = Utility::byteswap(chest.y_rot);
	chest.aux_params_2 = Utility::byteswap(chest.aux_params_2);
	chest.enemy_number = Utility::byteswap(chest.enemy_number);

	return chest;
}

ACTR ReadActor(std::fstream& fptr, int offset) {

	ACTR actor;

	fptr.seekg(offset, std::ios::beg);
	fptr.read(reinterpret_cast<char*>(&actor), sizeof actor);
	actor.params = Utility::byteswap(actor.params);
	actor.x_pos = Utility::byteswap(actor.x_pos);
	actor.y_pos = Utility::byteswap(actor.y_pos);
	actor.z_pos = Utility::byteswap(actor.z_pos);
	actor.aux_params_1 = Utility::byteswap(actor.aux_params_1);
	actor.y_rot = Utility::byteswap(actor.y_rot);
	actor.aux_params_2 = Utility::byteswap(actor.aux_params_2);
	actor.enemy_number = Utility::byteswap(actor.enemy_number);

	return actor;
}

SCOB ReadScob(std::fstream& fptr, int offset) {

	SCOB scob;

	fptr.seekg(offset, std::ios::beg);
	fptr.read(reinterpret_cast<char*>(&scob), sizeof scob);
	scob.params = Utility::byteswap(scob.params);
	scob.x_pos = Utility::byteswap(scob.x_pos);
	scob.y_pos = Utility::byteswap(scob.y_pos);
	scob.z_pos = Utility::byteswap(scob.z_pos);
	scob.aux_params_1 = Utility::byteswap(scob.aux_params_1);
	scob.y_rot = Utility::byteswap(scob.y_rot);
	scob.aux_params_2 = Utility::byteswap(scob.aux_params_2);
	scob.enemy_number = Utility::byteswap(scob.enemy_number);

	return scob;
}