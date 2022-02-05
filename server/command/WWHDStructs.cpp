#include "WWHDStructs.hpp"

#include "../utility/byteswap.hpp"



namespace WWHDStructs {
    ACTR readACTR(std::istream& in)
    {
        ACTR actr{};
        // name
        in.read(actr.name, sizeof(actr.name));
        // params
        in.read(reinterpret_cast<char*>(&actr.params), sizeof(actr.params));
        Utility::byteswap_inplace(actr.params);
        // x_pos
        in.read(reinterpret_cast<char*>(&actr.x_pos), sizeof(actr.x_pos));
        Utility::byteswap_inplace(actr.x_pos);
        // y_pos
        in.read(reinterpret_cast<char*>(&actr.y_pos), sizeof(actr.y_pos));
        Utility::byteswap_inplace(actr.y_pos);
        // z_pos
        in.read(reinterpret_cast<char*>(&actr.z_pos), sizeof(actr.z_pos));
        Utility::byteswap_inplace(actr.z_pos);
        // aux_params_1
        in.read(reinterpret_cast<char*>(&actr.aux_params_1), sizeof(actr.aux_params_1));
        Utility::byteswap_inplace(actr.aux_params_1);
        // y_rot
        in.read(reinterpret_cast<char*>(&actr.y_rot), sizeof(actr.y_rot));
        Utility::byteswap_inplace(actr.y_rot);
        // aux_params_2
        in.read(reinterpret_cast<char*>(&actr.aux_params_2), sizeof(actr.aux_params_2));
        Utility::byteswap_inplace(actr.aux_params_2);
        // enemy_number
        in.read(reinterpret_cast<char*>(&actr.enemy_number), sizeof(actr.enemy_number));
        Utility::byteswap_inplace(actr.enemy_number);

        return actr;
    }

    SCOB readSCOB(std::istream& in)
    {
        SCOB scob{};
        scob.actr = readACTR(in);
        // scale_x
        in.read(reinterpret_cast<char*>(&scob.scale_x), sizeof(scob.scale_x));
        // scale_y
        in.read(reinterpret_cast<char*>(&scob.scale_y), sizeof(scob.scale_y));
        // scaleZ
        in.read(reinterpret_cast<char*>(&scob.scale_z), sizeof(scob.scale_z));
        // _unused
        scob._unused = in.get();
        return scob;
    }

    uint8_t getChestItem(const ACTR& chest)
    {
        return (chest.aux_params_2 & 0xFF00) >> 8;
    }

    void setChestItem(ACTR& chest, uint8_t itemID)
    {
        chest.aux_params_2 &= 0x00FF;
        chest.aux_params_2 |= static_cast<uint16_t>(itemID) << 8;
    }
}
