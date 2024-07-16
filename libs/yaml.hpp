#pragma once

#include "./yaml-cpp/include/yaml-cpp/yaml.h"

#include <utility/path.hpp>
#include <utility/file.hpp>

// this wrapper is here to avoid path encoding issues
// removes any possible path -> string oddities or the need to open the file manually
inline YAML::Node LoadYAML(const fspath& path, const bool& resourceFile = false) {
    std::string file;
    if(Utility::getFileContents(path, file, resourceFile) != 0) {
        throw YAML::BadFile(path.string()); // exception is bad (unhandled) but it matches the old behavior
    }

    return YAML::Load(file);
}
