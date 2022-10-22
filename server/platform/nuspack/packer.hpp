#pragma once

#include <filesystem>

#include "./crypto/Key.hpp"

enum class [[nodiscard]] PackError {
    NONE = 0,
    XML_ERROR,
    UNKNOWN,
    COUNT,
};

const char* packErrorGetName(PackError err);

PackError createPackage(const std::filesystem::path& dirPath, const std::filesystem::path& out, const Key& encryptKeyWith = defaultEncryptWithKey);
