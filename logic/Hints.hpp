#pragma once

#include "World.hpp"

#define HINT_PREFIX u"They say that "
#define HINT_SUFFIX u"."

enum struct HintError
{
    NONE = 0,
};

HintError generateHints(WorldPool& worlds);
