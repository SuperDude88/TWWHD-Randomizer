#pragma once

#include <fstream>
#include <sstream>

#include <libs/AES.h>
#include <nuspack/crypto/ContentHashes.hpp>
#include <nuspack/crypto/IV.hpp>
#include <nuspack/crypto/Key.hpp>

class Content; //forward declare to avoid include circle

class Encryption {
    Key key;
    IV iv;
    
    AES aes;

public:
    Encryption(const Key& key_, const IV& iv_) :
        key(key_),
        iv(iv_),
        aes(AESKeyLength::AES_128)
    {}
    
    void EncryptFileWithPadding(std::istream& input, const uint32_t& contentID, std::ostream& output, const uint32_t& blockSize);
    void EncryptSingleFile(std::istream& input, std::ostream& output, const uint64_t& inputLength, const IV& iv_, const uint32_t& blockSize);
    void EncryptFileHashed(std::istream& input, std::ostream& output, const uint64_t& len, Content& content, ContentHashes& hashes);
    std::stringstream EncryptChunkHashed(const std::string& buffer, const uint32_t& block, ContentHashes& hashes, const Content& content);
    std::stringstream Encrypt(const std::string& input, const uint32_t& size = 0);
};