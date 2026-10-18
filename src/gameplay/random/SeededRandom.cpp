#include "gameplay/random/SeededRandom.hpp"

#include <stdexcept>

namespace soma::gameplay
{
    SeededRandom::SeededRandom(const std::uint64_t seed)
        : seed_(seed)
        , engine_(seed)
    {
    }

    std::uint64_t SeededRandom::seed() const noexcept
    {
        return seed_;
    }

    int SeededRandom::between(const int minimumInclusive, const int maximumInclusive)
    {
        if (minimumInclusive > maximumInclusive)
        {
            throw std::invalid_argument("Random range minimum exceeds maximum");
        }
        return std::uniform_int_distribution<int>{minimumInclusive, maximumInclusive}(engine_);
    }

    bool SeededRandom::chance(const double probability)
    {
        if (probability < 0.0 || probability > 1.0)
        {
            throw std::invalid_argument("Probability must be between zero and one");
        }
        return std::bernoulli_distribution{probability}(engine_);
    }
} // namespace soma::gameplay
