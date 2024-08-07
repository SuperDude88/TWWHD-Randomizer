#include "appinfo.hpp"

#include <libs/tinyxml2.hpp>
#include <command/Log.hpp>
#include <nuspack/packer.hpp>



bool AppInfo::parseFromXML(const fspath& xmlPath) {
    using namespace tinyxml2;

    XMLDocument app;
    if(XMLError err = LoadXML(app, xmlPath); err != XMLError::XML_SUCCESS) {
        return false;
    }

    const XMLElement* root = app.RootElement();

    OSVersion = std::stoull(root->FirstChildElement("os_version")->GetText(), nullptr, 16);
    titleID = std::stoull(root->FirstChildElement("title_id")->GetText(), nullptr, 16);
    titleVer = std::stoul(root->FirstChildElement("title_version")->GetText(), nullptr, 16);
    sdkVer = std::stoul(root->FirstChildElement("sdk_version")->GetText(), nullptr, 16);
    appType = AppType(std::stoul(root->FirstChildElement("app_type")->GetText(), nullptr, 16));
    groupID = std::stoul(root->FirstChildElement("group_id")->GetText(), nullptr, 16);
    
    const std::string mask = root->FirstChildElement("os_mask")->GetText();
    for(size_t i = 0; i < osMask.size(); i++) {
        const std::string character(1, mask[i]);
        osMask[i] = std::stoul(character, nullptr, 16);
    }

    if(root->FirstChildElement("common_id") != nullptr) {
        common_id = std::stoull(root->FirstChildElement("common_id")->GetText(), nullptr, 16);
    }
    else {
        common_id = 0;
    }

    return true;
}
