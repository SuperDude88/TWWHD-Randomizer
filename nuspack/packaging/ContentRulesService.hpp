#pragma once

#include <nuspack/contents/contents.hpp>
#include <nuspack/fst/FSTEntries.hpp>
#include <nuspack/packaging/ContentRules.hpp>



void applyRules(FSTEntry& root, Contents& targetContents, const ContentRules& rules);
