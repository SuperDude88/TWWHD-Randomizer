#pragma once

#include "World.hpp"

#define HINT_PREFIX_ENGLISH u"They say that "
#define HINT_PREFIX_SPANISH u"Dicen que "
#define HINT_PREFIX_FRENCH  u"J'ai entendu dire que "

enum struct HintError
{
    NONE = 0,
};

HintError generateHints(WorldPool& worlds);
