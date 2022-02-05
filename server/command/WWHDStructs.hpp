#pragma once

#include <cstdint>
#include <istream>

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

struct SCOB
{
    ACTR actr;
    uint8_t scale_x;
    uint8_t scale_y;
    uint8_t scale_z;
    char _unused;
};

namespace WWHDStructs {
    ACTR readACTR(std::istream& in);
    SCOB readSCOB(std::istream& in);

    uint8_t getChestItem(const ACTR& chest);
    void setChestItem(ACTR& chest, uint8_t itemID);
}
