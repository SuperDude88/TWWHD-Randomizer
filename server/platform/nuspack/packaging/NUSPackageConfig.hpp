#pragma once

#include "../crypto/Key.hpp"
#include "../packaging/ContentRules.hpp"
#include "../appinfo.hpp"

#include <filesystem>



struct PackageConfig {
    const std::filesystem::path dir;
    const AppInfo info;
    const Key encryptionKey;
    const Key encryptKeyWith;
    const ContentRules rules;

    PackageConfig(const std::filesystem::path& dir_, const AppInfo& info_, const Key& key_, const Key& keyWith_, const ContentRules& rules_) :
        dir(dir_),
        info(info_),
        encryptionKey(key_),
        encryptKeyWith(keyWith_),
        rules(rules_)
    {}
};
