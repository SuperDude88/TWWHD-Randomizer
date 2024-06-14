#pragma once

#include <array>
#include <initializer_list>
#include <string>

using Key = std::array<uint8_t, 16>;

static constexpr Key defaultEncryptionKey = {0x13, 0x37, 0x13, 0x37, 0x13, 0x37, 0x13, 0x37, 0x13, 0x37, 0x13, 0x37, 0x13, 0x37, 0x13, 0x37};
static constexpr Key defaultEncryptWithKey = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
