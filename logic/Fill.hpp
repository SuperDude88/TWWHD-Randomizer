
#pragma once

#include "World.hpp"

enum struct FillError
{
    NONE = 0,
    RAN_OUT_OF_RETRIES,
    MORE_ITEMS_THAN_LOCATIONS,
};

FillError fill(std::vector<World>& worlds);
const char* errorToName(FillError err);
