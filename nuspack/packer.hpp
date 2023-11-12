#pragma once

#include <filesystem>

#include <nuspack/crypto/Key.hpp>

enum struct [[nodiscard]] PackError {
    NONE = 0,
    XML_ERROR,
    UNKNOWN,
    COUNT,
};

const char* packErrorGetName(PackError err);

PackError createPackage(const std::filesystem::path& dirPath, const std::filesystem::path& out, const Key& encryptionKey = defaultEncryptionKey, const Key& encryptKeyWith = defaultEncryptWithKey);
