#pragma once

#include "./yaml-cpp/include/yaml-cpp/yaml.h"

#include <utility/path.hpp>

// this wrapper is here to avoid path encoding issues
// removes any possible path -> string oddities or the need to open the file manually
bool LoadYAML(YAML::Node& out, const fspath& path, const bool& resourceFile = false);
