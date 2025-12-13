#include "yaml.hpp"

#include <utility/file.hpp>
#include <command/Log.hpp>

bool LoadYAML(YAML::Node& out, const fspath& path, const bool& resourceFile /* = false */) {
    std::string file;
    if(Utility::getFileContents(path, file, resourceFile) != 0) {
        ErrorLog::getInstance().log("Unable to get data from " + Utility::toUtf8String(path));
        return false;
    }

    try {
        out = YAML::Load(file);
    }
    catch (const YAML::Exception& ex) {
        ErrorLog::getInstance().log(std::string("Error parsing yaml " + Utility::toUtf8String(path) + ": ") + ex.what());
        return false;
    }

    return true;
}
