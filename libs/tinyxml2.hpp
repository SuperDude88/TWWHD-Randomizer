#pragma once

#include "./tinyxml2/tinyxml2.h"

#include <utility/file.hpp>

// this wrapper is here to avoid path encoding issues
// because the included overloads take a const char* or a FILE*, you either need
// to remember to use _wfopen_s or risk broken paths on Windows
inline tinyxml2::XMLError LoadXML(tinyxml2::XMLDocument& out, const fspath& path, const bool& resourceFile = false) {
    std::string file;
    if(Utility::getFileContents(path, file, resourceFile) != 0) {
        return tinyxml2::XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED;
    }

    return out.Parse(file.c_str(), file.size());
}
