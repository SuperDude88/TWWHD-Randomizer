#pragma once

#include <fstream>

#include <utility/file.hpp>



inline void writeCertData(std::ostream& out) {
    out.write("\x00\x01\x00\x03", 4);
    Utility::seek(out, 0x400);
    out.write("\x00\x01\x00\x04", 4);
    Utility::seek(out, 0x700);
    out.write("\x00\x01\x00\x04", 4);

    Utility::seek(out, 0x240);
    out.write("Root\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
    Utility::seek(out, 0x280);
    out.write("\x00\x00\x00\x01""CA00000003\x00\x00", 16);

    Utility::seek(out, 0x540);
    out.write("Root-CA00000003\x00", 16);
    Utility::seek(out, 0x580);
    out.write("\x00\x00\x00\x01""CP0000000b\x00\x00", 16);

    Utility::seek(out, 0x840);
    out.write("Root-CA00000003\x00", 16);
    Utility::seek(out, 0x880);
    out.write("\x00\x00\x00\x01""XS0000000c\x00\x00", 16);
    
    Utility::seek(out, 0xA00, std::ios::beg);

    return;
}
