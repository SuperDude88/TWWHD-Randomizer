#pragma once

#include <unordered_map>
#include "../types.hpp"
#include <string>
#include <vector>
#include <ostream>

class ContentHashes {
public:
    using Hashes_t = std::unordered_map<int, SHA1_t>;
    Hashes_t h0Hashes;
    Hashes_t h1Hashes;
    Hashes_t h2Hashes;
    Hashes_t h3Hashes;

    SHA1_t TMDHash;
    uint32_t blockCount;

    ContentHashes(std::istream& file, const bool& hashed);
    void CalculateOtherHashes(const uint32_t& hashLevel, const Hashes_t& inHashes, Hashes_t& outHashes);
    void CalculateH0Hashes(std::istream& file);
    std::string GetHashForBlock(uint32_t block);
    std::vector<SHA1_t> GetH3Hashes();
    void SaveH3ToFile(std::ostream& out);
};
