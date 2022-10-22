#pragma once

#include <cstdint>
#include <array>

using SHA1_t = std::array<uint8_t, 20>;
using SHA256_t = std::array<uint8_t, 0x20>;

using Issuer_t = std::array<uint8_t, 0x40>;
