
#pragma once

#include "World.hpp"

enum struct EntranceError
{
    NONE = 0,
};

EntranceError randomizeEntrances(WorldPool& worlds);
const char* errorToName(EntranceError err);
