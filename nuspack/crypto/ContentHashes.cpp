#include "ContentHashes.hpp"

#include <sstream>

#include <nuspack/contents/contents.hpp>
#include <libs/hashing/sha1.h>
#include <utility/file.hpp>
#include <utility/math.hpp>

static SHA1 sha1;

ContentHashes::ContentHashes(std::istream& file, const bool& hashed) {
    if(hashed) {
        CalculateH0Hashes(file);
        CalculateOtherHashes(1, h0Hashes, h1Hashes);
        CalculateOtherHashes(2, h1Hashes, h2Hashes);
        CalculateOtherHashes(3, h2Hashes, h3Hashes);
        const auto& hashes = GetH3Hashes();
        std::string hash = sha1(hashes.data(), hashes.size() * sizeof(SHA1_t));
        std::copy(hash.begin(), hash.end(), TMDHash.begin());
    }
    else {
        auto ss = std::ostringstream{};
	    ss << file.rdbuf();
        std::string data = ss.str();
        data.resize(roundUp<size_t>(data.size(), Content::PAD_LEN));

        std::string hash = sha1(data);
        std::copy(hash.begin(), hash.begin() + 20, TMDHash.begin());
    }
}

void ContentHashes::CalculateOtherHashes(const uint32_t& hashLevel, const Hashes_t& inHashes, Hashes_t& outHashes) {
    int hash_level_pow = 1 << (4 * hashLevel);

    int hashesCount = (blockCount / hash_level_pow) + 1;
    for (int new_block = 0; new_block < hashesCount; new_block++)
    {
        std::array<SHA1_t, 16> cur_hashes = {{{0}}}; //why do Clang warnings want so many braces?
        for (int i = new_block * 16; i < (new_block * 16) + 16; i++)
        {
            if (inHashes.contains(i)) cur_hashes[(i % 16)] = inHashes.at(i);
        }

        const std::string hash = sha1(reinterpret_cast<const void*>(cur_hashes.data()), sizeof(cur_hashes));
        std::copy(hash.begin(), hash.end(), outHashes[new_block].begin());
    }
}

void ContentHashes::CalculateH0Hashes(std::istream& file) {
    static constexpr uint32_t bufferSize = 0xFC00;

    blockCount = 0;
    std::string buffer(bufferSize, '\0');
    while (file)
    {
        file.read(&buffer[0], bufferSize);

        if(file.gcount() > 0) {
            const std::string hash = sha1(&buffer[0], buffer.size());
            std::copy(hash.begin(), hash.end(), h0Hashes[blockCount].begin());

            blockCount++;
        }
    }
}

std::string ContentHashes::GetHashForBlock(uint32_t block) {
    std::stringstream hashes;

    if(block > blockCount) {
        return hashes.str();
    }

    const uint32_t h0_hash_start = (block / 16) * 16;
    for (uint32_t i = 0; i < 16; i++)
    {
        const uint32_t index = h0_hash_start + i;
        if (h0Hashes.contains(index))
        {
            hashes.write(reinterpret_cast<const char*>(h0Hashes.at(index).data()), h0Hashes.at(index).size());
        }
        else {
            Utility::seek(hashes, 20, std::ios::cur);
        }
    }

    const uint32_t h1_hash_start = (block / 256) * 16;
    for (uint32_t i = 0; i < 16; i++)
    {
        const uint32_t index = h1_hash_start + i;
        if (h1Hashes.contains(index))
        {
            hashes.write(reinterpret_cast<const char*>(h1Hashes.at(index).data()), h1Hashes.at(index).size());
        }
        else {
            Utility::seek(hashes, 20, std::ios::cur);
        }
    }

    const uint32_t h2_hash_start = (block / 4096) * 16;
    for (uint32_t i = 0; i < 16; i++)
    {
        const uint32_t index = h2_hash_start + i;
        if (h2Hashes.contains(index))
        {
            hashes.write(reinterpret_cast<const char*>(h2Hashes.at(index).data()), h1Hashes.at(index).size());
        }
        else {
            Utility::seek(hashes, 20, std::ios::cur);
        }
    }

    std::string ret = hashes.str();
    ret.resize(0x400, '\0');
    return ret;
}

std::vector<SHA1_t> ContentHashes::GetH3Hashes() {
    std::vector<SHA1_t> ret;

    for (int i = 0; i < h3Hashes.size(); i++)
    {
        ret.push_back(h3Hashes[i]);
    }

    return ret;
}

void ContentHashes::SaveH3ToFile(std::ostream& out) {
    if (h3Hashes.size() > 0)
    {
        const auto& hashes = GetH3Hashes();
        out.write(reinterpret_cast<const char*>(hashes.data()), hashes.size() * sizeof(SHA1_t));
    }
}
