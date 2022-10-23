#include "appinfo.hpp"

#include "../../../libs/tinyxml2.h"
#include "../../command/Log.hpp"
#include "./packer.hpp"



PackError AppInfo::parseFromXML(const std::filesystem::path& xmlPath) {
    using namespace tinyxml2;

    XMLDocument app;
    if(XMLError err = app.LoadFile(xmlPath.string().c_str()); err != XMLError::XML_SUCCESS) {
        LOG_ERR_AND_RETURN(PackError::XML_ERROR);
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
        osMask[i] = std::stoi(character, nullptr, 16);
    }

    if(root->FirstChildElement("common_id") != nullptr) {
        common_id = std::stoull(root->FirstChildElement("common_id")->GetText(), nullptr, 16);
    }
    else {
        common_id = 0;
    }

    return PackError::NONE;
}