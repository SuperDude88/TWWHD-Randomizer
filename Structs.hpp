#ifndef Structs_H
#define Structs_H

#include <cstdint>

struct ACTR {
	char name[8];
	uint32_t params;
	float x_pos;
	float y_pos;
	float z_pos;
	uint16_t aux_params_1;
	uint16_t y_rot;
	uint16_t aux_params_2;
	uint16_t enemy_number;
};

struct SCOB {
	char name[8];
	uint32_t params;
	float x_pos;
	float y_pos;
	float z_pos;
	uint16_t aux_params_1;
	uint16_t y_rot;
	uint16_t aux_params_2;
	uint16_t enemy_number;
	uint8_t scale_x;
	uint8_t scale_y;
	uint8_t scale_z;
	uint8_t padding;
};

#endif
