#include "random.hpp"

static bool init = false;
static std::mt19937_64 generator;

//Initialize with seed specified
void Random_Init(size_t seed)
{
    init = true;
    generator = std::mt19937_64{seed};
}

//Returns a random integer in range [min, max-1]
uint32_t Random(int min, int max)
{
    if (!init)
    {
        //No seed given, get a random number from device to seed
        const auto seed = static_cast<uint32_t>(std::random_device{}());
        Random_Init(seed);
    }

    auto number = generator();
    return min + (number % (max - min));
}

//Returns a random floating point number in [0.0, 1.0]
double RandomDouble()
{
    auto number = generator();
    return (double) number / (double) generator.max(); 
}

std::mt19937_64& GetGenerator()
{
    return generator;
}
