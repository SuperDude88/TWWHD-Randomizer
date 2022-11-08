#include "WWHDStructs.hpp"

#include <utility/endian.hpp>

using eType = Utility::Endian::Type;

namespace WWHDStructs {
    ACTR readACTR(std::istream& in)
    {
        ACTR actr{};

        actr.name.resize(8);
        in.read(&actr.name[0], 8);
        in.read(reinterpret_cast<char*>(&actr.params), sizeof(actr.params));
        in.read(reinterpret_cast<char*>(&actr.x_pos), sizeof(actr.x_pos));
        in.read(reinterpret_cast<char*>(&actr.y_pos), sizeof(actr.y_pos));
        in.read(reinterpret_cast<char*>(&actr.z_pos), sizeof(actr.z_pos));
        in.read(reinterpret_cast<char*>(&actr.aux_params_1), sizeof(actr.aux_params_1));
        in.read(reinterpret_cast<char*>(&actr.y_rot), sizeof(actr.y_rot));
        in.read(reinterpret_cast<char*>(&actr.aux_params_2), sizeof(actr.aux_params_2));
        in.read(reinterpret_cast<char*>(&actr.enemy_number), sizeof(actr.enemy_number));

        Utility::Endian::toPlatform_inplace(eType::Big, actr.params);
        Utility::Endian::toPlatform_inplace(eType::Big, actr.x_pos);
        Utility::Endian::toPlatform_inplace(eType::Big, actr.y_pos);
        Utility::Endian::toPlatform_inplace(eType::Big, actr.z_pos);
        Utility::Endian::toPlatform_inplace(eType::Big, actr.aux_params_1);
        Utility::Endian::toPlatform_inplace(eType::Big, actr.y_rot);
        Utility::Endian::toPlatform_inplace(eType::Big, actr.aux_params_2);
        Utility::Endian::toPlatform_inplace(eType::Big, actr.enemy_number);
        return actr;
    }

    SCOB readSCOB(std::istream& in)
    {
        SCOB scob{};
        scob.actr = readACTR(in);
        in.read(reinterpret_cast<char*>(&scob.scale_x), sizeof(scob.scale_x));
        in.read(reinterpret_cast<char*>(&scob.scale_y), sizeof(scob.scale_y));
        in.read(reinterpret_cast<char*>(&scob.scale_z), sizeof(scob.scale_z));
        scob._unused = in.get();
        return scob;
    }


    void writeACTR(std::ostream& out, const ACTR& actr) {
        auto params_BE = Utility::Endian::toPlatform(eType::Big, actr.params);
        auto x_pos_BE = Utility::Endian::toPlatform(eType::Big, actr.x_pos);
        auto y_pos_BE = Utility::Endian::toPlatform(eType::Big, actr.y_pos);
        auto z_pos_BE = Utility::Endian::toPlatform(eType::Big, actr.z_pos);
        auto aux_params_1_BE = Utility::Endian::toPlatform(eType::Big, actr.aux_params_1);
        auto y_rot_BE = Utility::Endian::toPlatform(eType::Big, actr.y_rot);
        auto aux_params_2_BE = Utility::Endian::toPlatform(eType::Big, actr.aux_params_2);
        auto enemy_number_BE = Utility::Endian::toPlatform(eType::Big, actr.enemy_number);

        out.write(&actr.name[0], 8);
        out.write(reinterpret_cast<const char*>(&params_BE), sizeof(params_BE));
        out.write(reinterpret_cast<const char*>(&x_pos_BE), sizeof(x_pos_BE));
        out.write(reinterpret_cast<const char*>(&y_pos_BE), sizeof(y_pos_BE));
        out.write(reinterpret_cast<const char*>(&z_pos_BE), sizeof(z_pos_BE));
        out.write(reinterpret_cast<const char*>(&aux_params_1_BE), sizeof(aux_params_1_BE));
        out.write(reinterpret_cast<const char*>(&y_rot_BE), sizeof(y_rot_BE));
        out.write(reinterpret_cast<const char*>(&aux_params_2_BE), sizeof(aux_params_2_BE));
        out.write(reinterpret_cast<const char*>(&enemy_number_BE), sizeof(enemy_number_BE));

        return;
    }

    void writeSCOB(std::ostream& out, const SCOB& scob) {
        writeACTR(out, scob.actr);
        out.write(reinterpret_cast<const char*>(&scob.scale_x), sizeof(scob.scale_x));
        out.write(reinterpret_cast<const char*>(&scob.scale_y), sizeof(scob.scale_y));
        out.write(reinterpret_cast<const char*>(&scob.scale_z), sizeof(scob.scale_z));
        out.write(&scob._unused, sizeof(scob._unused));

        return;
    }
}
