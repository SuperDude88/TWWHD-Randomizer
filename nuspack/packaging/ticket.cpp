#include "ticket.hpp"

#include <random>
#include <algorithm>

#include <nuspack/crypto/IV.hpp>
#include <nuspack/crypto/Encryption.hpp>
#include <utility/file.hpp>
#include <utility/endian.hpp>

using eType = Utility::Endian::Type;



Key Ticket::GetEncryptedKey() {
    using namespace Utility::Endian;
    
    const uint64_t tid = toPlatform(Type::Big, titleID);
    IV iv{0};
    std::memcpy(iv.data(), &tid, sizeof(tid));
    Encryption encrypt(encryptWith, iv);

    std::string keyData = encrypt.Encrypt(std::string(reinterpret_cast<const char*>(decryptedKey.data()), decryptedKey.size())).str();
    Key ret;
    std::memcpy(ret.data(), keyData.data(), ret.size());
    return ret;
}

void Ticket::writeToStream(std::ostream& out) {
    using bytes_randomizer = std::independent_bits_engine<std::default_random_engine, 32, uint32_t>;
    std::default_random_engine rd;
    bytes_randomizer bytes(rd);

    std::array<uint8_t, 0x100> randomData;
    std::generate(std::begin(randomData), std::end(randomData), std::ref(bytes));

    out.write("\x00\x01\x00\x04", 4);
    out.write(reinterpret_cast<const char*>(randomData.data()), randomData.size());
    Utility::seek(out, 0x3C, std::ios::cur);
    out.write("Root-CA00000003-XS0000000c\x00\x00\x00\x00\x00\x00", 0x20);
    Utility::seek(out, 0x5C, std::ios::cur);
    out.write("\x01\x00\x00", 3);
    const auto key = GetEncryptedKey();
    out.write(reinterpret_cast<const char*>(key.data()), key.size());
    out.write("\x00\x00\x05", 3);
    
    std::generate(std::begin(randomData), std::end(randomData), std::ref(bytes));
    out.write(reinterpret_cast<const char*>(randomData.data()), 6);
    Utility::seek(out, 0x04, std::ios::cur);

    const uint64_t tid = Utility::Endian::toPlatform(eType::Big, titleID);
    out.write(reinterpret_cast<const char*>(&tid), sizeof(tid));
    out.write("\x00\x00\x00\x11\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05", 0x10);
    Utility::seek(out, 0xB0, std::ios::cur);
    out.write("\x00\x01\x00\x14\x00\x00\x00\xAC\x00\x00\x00\x14\x00\x01\x00\x14\x00\x00\x00\x00\x00\x00\x00\x28\x00\x00\x00\x01\x00\x00\x00\x84\x00\x00\x00\x84\x00\x03\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\x01", 0x30);
    Utility::seek(out, 0x7C, std::ios::cur);
}
