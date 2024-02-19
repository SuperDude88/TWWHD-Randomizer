#pragma once

#include <string>

#include <seedgen/config.hpp>

std::string generate_seed();

std::string generate_seed_hash();

std::string hash_for_seed(const std::string& seed);

std::string hash_for_seed(const std::string& seed, const Config& config);
