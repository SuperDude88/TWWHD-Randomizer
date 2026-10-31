#pragma once

#include <logic/WorldPool.hpp>

#include <unordered_map>
#include <string>

#define HINT_PREFIX_ENGLISH u"They say that "
#define HINT_PREFIX_SPANISH u"Dicen que "
#define HINT_PREFIX_FRENCH  u"J'ai entendu dire que "
#define HINT_PREFIX_GERMAN  u"Man sagt, "
#define HINT_PREFIX_ITALIAN u"Si dice che "
#define HINT_PREFIX_JAPANESE u"うわさによると、"

enum struct HintError
{
    NONE = 0,
};

enum struct HintType
{
    NONE = 0,
    PATH,
    BARREN,
    ITEM,
    LOCATION,
};

class Location;
struct Hint
{
    // Message for this location (one for each language)
    std::unordered_map<std::string, std::u16string> text = {};
    Location* location;
    HintType type = HintType::NONE;
};

HintError generateHints(WorldPool& worlds);
