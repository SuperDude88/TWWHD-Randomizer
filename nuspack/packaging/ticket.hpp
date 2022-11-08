#pragma once

#include <cstdint>
#include <fstream>

#include <nuspack/crypto/Key.hpp>

class Ticket {
public:
    uint64_t titleID = 0;
    Key decryptedKey = defaultEncryptionKey;
    Key encryptWith = defaultEncryptWithKey;

    Ticket() = default;
    Ticket(const uint64_t& titleID_, const Key& decrypted_, const Key& with_) :
        titleID(titleID_),
        decryptedKey(decrypted_),
        encryptWith(with_)
    {}
    
    Key GetEncryptedKey();
    void writeToStream(std::ostream& out);
};
