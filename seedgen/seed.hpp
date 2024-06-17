#pragma once

#include <seedgen/config.hpp>

std::string generate_seed();

std::string generate_seed_hash();

std::string hash_for_config(const Config& config);
