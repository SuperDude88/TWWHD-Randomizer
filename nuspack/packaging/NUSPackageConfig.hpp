#pragma once

#include <utility/path.hpp>
#include <nuspack/crypto/Key.hpp>
#include <nuspack/packaging/ContentRules.hpp>
#include <nuspack/appinfo.hpp>



struct PackageConfig {
    const fspath dir;
    const AppInfo info;
    const Key encryptionKey;
    const Key encryptKeyWith;
    const ContentRules rules;

    PackageConfig(const fspath& dir_, const AppInfo& info_, const Key& key_, const Key& keyWith_, const ContentRules& rules_) :
        dir(dir_),
        info(info_),
        encryptionKey(key_),
        encryptKeyWith(keyWith_),
        rules(rules_)
    {}
};
