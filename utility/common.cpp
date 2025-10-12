#include "common.hpp"

size_t padToLen(std::ostream& out, const unsigned int& len, const char pad) {
    if (len == 0) return 0; //don't pad to no alignment (also cant % by 0)

    size_t padLen = len - (static_cast<size_t>(out.tellp()) % len);
    if (padLen == len) return 0; //doesnt write any padding, return length 0

    for (size_t i = 0; i < padLen; i++) {
        out.write(&pad, 1);
    }

    return padLen; //return number of bytes written
}
