#include "random.hpp"

static bool init = false;
static Generator_t generator;

Seed_t seedFromString(const std::string& str) {
    // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash (64-bit)
    Seed_t hash = 14695981039346656037U;
    for(const char& c : str) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 1099511628211;
    }

    return hash;
}

// Initialize with seed specified
void Random_Init(Seed_t seed)
{
    init = true;
    generator = Generator_t{seed};
}

// Returns a random integer in range [min, max-1]
uint32_t Random(int min, int max)
{
    if (!init)
    {
        // No seed given, get a random number from device to seed
        const Seed_t& seed = static_cast<Seed_t>(std::random_device{}());
        Random_Init(seed);
    }

    const auto& number = generator();
    return min + (number % (max - min));
}

// Returns a random floating point number in [0.0, 1.0]
double RandomDouble()
{
    if (!init)
    {
        // No seed given, get a random number from device to seed
        const Seed_t& seed = static_cast<Seed_t>(std::random_device{}());
        Random_Init(seed);
    }

    const auto& number = generator();
    return (double) number / (double) generator.max(); 
}

Generator_t& GetGenerator()
{
    return generator;
}
