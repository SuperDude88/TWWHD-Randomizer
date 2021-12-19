#pragma once

#include <iostream>

namespace FileTypes {
    uint32_t yaz0Encode(std::istream& in, std::ostream& out, int compressionLevel = 9);
    uint32_t yaz0Decode(std::istream& in, std::ostream& out);
}
