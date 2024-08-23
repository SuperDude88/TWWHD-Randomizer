#pragma once

#include <utility/path.hpp>
#include <nuspack/crypto/Key.hpp>

enum struct [[nodiscard]] PackError {
    NONE = 0,
    XML_ERROR,
    UNKNOWN,
    COUNT,
};

std::string packErrorGetName(PackError err);

PackError createPackage(const fspath& dirPath, const fspath& out, const Key& encryptionKey = defaultEncryptionKey, const Key& encryptKeyWith = defaultEncryptWithKey);
