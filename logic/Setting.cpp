
#include "Setting.hpp"
#include <unordered_map>

Setting nameToSetting(const std::string& name)
{
    static std::unordered_map<std::string, Setting> settingNameMap = {
        {"Swordless", Setting::Swordless},
        {"SkipTrials", Setting::SkipTrials}
    };
    if (settingNameMap.count(name) == 0) return Setting::INVALID;
    return settingNameMap.at(name);
}
