//Metadata for Wii U titles
//Contains info on the title and contents (including hashes), necessary for building a NUS package

#pragma once

#include <array>

#include <utility/path.hpp>
#include <nuspack/packaging/ticket.hpp>
#include <nuspack/appinfo.hpp>
#include <nuspack/crypto/Encryption.hpp>
#include <nuspack/contents/contentInfo.hpp>
#include <nuspack/contents/contents.hpp>

enum struct [[nodiscard]] TMDError {
    NONE = 0,
    COULD_NOT_OPEN,
    REACHED_EOF,
    UNEXPECTED_VALUE,
    UNKNOWN,
    COUNT
};

enum struct SigType : uint32_t {
    RSA_2048_SHA256 = 0x00010004,
    RSA_4096_SHA256 = 0x00010003,
    RSA_2048_SHA1   = 0x00010001,
    RSA_4096_SHA1   = 0x00010000
};

struct TMDHeader {
    SigType signatureType = SigType::RSA_2048_SHA256;
    std::array<uint8_t, 0x100> signature = {0};
    std::array<uint8_t, 0x3C> padding_0x00 = {0};
    Issuer_t issuer = {"Root-CA00000003-CP0000000b\0\0\0\0\0\0\0\0\0\0\0\0\0\0"};
    uint8_t version = 1;
    uint8_t caCrlVersion = 0;
    uint8_t signerCrlVersion = 0;
    uint8_t pad_0x00 = 0;
    uint64_t systemVersion = 0x000500101000400A;
    //uint64_t titleID = 0;
    uint32_t titleType = 0x00000100;
    uint16_t groupID = 0;
    AppType appType = AppType::GAME;
    uint32_t unk1 = 0;
    uint32_t unk2 = 0;
    std::array<uint8_t, 0x3A> reserved = {0};
    uint32_t accessRights = 0;
    uint16_t titleVersion = 0;
    uint16_t contentCount = 0;
    uint16_t bootIndex = 0;
    std::array<uint8_t, 2> padding2_0x00 = {0};
    SHA256_t SHA2 = {0};
};

namespace FileTypes {

    const char* TMDErrorGetName(TMDError err);

    class TMDFile {
    public:
        TMDFile(Ticket& ticket_, Contents& contents_) : 
            ticket(ticket_),
            contents(contents_)
        {}

        TMDError loadFromBinary(std::istream& in);
        TMDError loadFromFile(const fspath& filePath);
        void update(const AppInfo& info);
        TMDError writeToStream(std::ostream& out);
        TMDError writeToFile(const fspath& filePath);
        Encryption getEncryption();
        void UpdateContentInfoHash();
        
        ContentInfo contentInfo;
    private:
        TMDHeader header;
        Ticket& ticket;
        Contents& contents;
    };
}
