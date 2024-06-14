#include "Encryption.hpp"

#include <utility/endian.hpp>
#include <utility/math.hpp>
#include <nuspack/contents/contents.hpp>

#include <gui/update_dialog_header.hpp>

using eType = Utility::Endian::Type;

void Encryption::EncryptFileWithPadding(std::istream& input, const uint32_t& contentID, std::ostream& output, const uint32_t& blockSize) {
    const uint16_t write = Utility::Endian::toPlatform(eType::Big, static_cast<const uint16_t>(contentID));
    IV iv{0};
    std::memcpy(&iv[0], &write, sizeof(uint16_t));

    EncryptSingleFile(input, output, input.seekg(0, std::ios::end).tellg(), iv, blockSize);
}

void Encryption::EncryptSingleFile(std::istream& input, std::ostream& output, const uint64_t& inputLength, const IV& iv_, const uint32_t& blockSize) {
    iv = iv_;
    uint64_t targetSize = roundUp<uint64_t>(inputLength, blockSize);
    input.seekg(0, std::ios::beg);
    
    uint64_t cur_position = 0;
    do
    {
        std::string blockBuffer(blockSize, '\0');
        input.read(&blockBuffer[0], blockSize);
        blockBuffer = Encrypt(blockBuffer).str();

        std::copy(blockBuffer.end() - 16, blockBuffer.end(), iv.begin());

        cur_position += blockSize;
        output.write(&blockBuffer[0], blockBuffer.size());
    } while (cur_position < targetSize && input/* .gcount() == blockSize */);
}

void Encryption::EncryptFileHashed(std::istream& input, std::ostream& output, const uint64_t& len, Content& content, ContentHashes& hashes) {
    constexpr uint32_t hashBlockSize = 0xFC00;

    uint32_t block = 0;
    std::string buffer(hashBlockSize, '\0');
    do
    {
        input.read(&buffer[0], hashBlockSize);

        const std::stringstream& encrypted = EncryptChunkHashed(buffer, block, hashes, content);
        output << encrypted.rdbuf();

        block++;
        // Update progress dialogue
        if (block > 1000)
        {
            UPDATE_DIALOG_VALUE(149 + (int)(((float) (input.gcount() * block) / (float) len) * 50.0f))
        }
    } while (input);
    content.size = output.tellp();
}

std::stringstream Encryption::EncryptChunkHashed(const std::string& buffer, const uint32_t &block, ContentHashes &hashes, const Content &content) {
    const uint16_t& write = Utility::Endian::toPlatform(eType::Big, static_cast<const uint16_t>(content.id));
    iv = IV{0};
    std::memcpy(&iv[0], &write, sizeof(uint16_t));

    std::string decryptedHashes = hashes.GetHashForBlock(block);

    decryptedHashes[1] ^= static_cast<uint8_t>(content.id);

    std::stringstream outputStream(Encrypt(decryptedHashes));
    decryptedHashes[1] ^= static_cast<uint8_t>(content.id);
    const uint32_t iv_start = (block % 16) * 20;

    std::copy(decryptedHashes.begin() + iv_start, decryptedHashes.begin() + iv_start + 16, iv.begin());

    outputStream << Encrypt(buffer).str();

    return outputStream;
}

std::stringstream Encryption::Encrypt(const std::string& input, const uint32_t& size) {
    const uint32_t inputSize = ((size != 0) ? size : input.size());

    const uint8_t* data = aes.EncryptCBC(reinterpret_cast<const uint8_t*>(&input[0]), inputSize, key.data(), iv.data());
    std::stringstream ret;
    ret.write(reinterpret_cast<const char*>(data), inputSize);
    delete[] data;
    return ret;
}
