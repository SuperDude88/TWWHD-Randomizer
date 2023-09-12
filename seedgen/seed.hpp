#pragma once

#include <string>

#include <seedgen/config.hpp>

#define SEED_KEY "SEED KEY TEST"

std::string generate_seed();

std::string generate_seed_hash();

std::string hash_for_seed(const std::string& seed);

std::string hash_for_seed(const std::string& seed, const Config& config);