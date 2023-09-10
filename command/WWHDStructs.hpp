#pragma once

#include <cstdint>
#include <istream>
#include <string>

struct ACTR {
    std::string name;
    uint32_t params;
    float x_pos;
    float y_pos;
    float z_pos;
    uint16_t x_rot;
    uint16_t y_rot;
    uint16_t z_rot;
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

    void writeACTR(std::ostream& out, const ACTR& actr);
    void writeSCOB(std::ostream& out, const SCOB& scob);
}
