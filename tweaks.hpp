#pragma once

#include <logic/World.hpp>



enum struct [[nodiscard]] TweakError {
    NONE = 0,
    DATA_FILE_MISSING,
    PATCH_MISSING_KEY,
    RELOCATION_MISSING_KEY,
    FILE_OPEN_FAILED,
    FILE_COPY_FAILED,
    FILE_SAVE_FAILED,
    RPX_OPERATION_FAILED,
    FILETYPE_ERROR,
    MISSING_SYMBOL,
    MISSING_EVENT,
    MISSING_ENTITY,
    UNEXPECTED_VALUE,
    UNKNOWN,
    COUNT
};

std::string get_island_room_dzx_filepath(const uint8_t& islandNum);

TweakError apply_necessary_tweaks(const Settings& settings);

TweakError apply_necessary_post_randomization_tweaks(World& world/* , const bool& randomizeItems */);

