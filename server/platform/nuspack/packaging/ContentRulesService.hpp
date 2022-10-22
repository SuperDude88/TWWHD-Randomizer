#pragma once

#include <string>
#include <vector>

#include "../contents/contents.hpp"
#include "../fst/FSTEntries.hpp"
#include "ContentRules.hpp"



void applyRules(FSTEntry& root, Contents& targetContents, const ContentRules& rules);
