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

TweakError apply_necessary_tweaks(const Settings& settings);

TweakError apply_necessary_post_randomization_tweaks(World& world/* , const bool& randomizeItems */);

std::string errorGetName(TweakError err);
