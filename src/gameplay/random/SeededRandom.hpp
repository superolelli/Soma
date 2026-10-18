#pragma once
#include "gameplay/random/IRandom.hpp"

#include <random>

namespace soma::gameplay
{
    /// Implements reproducible pseudo-random choices from a supplied seed.
    class SeededRandom final : public IRandom
    {
      public:
        explicit SeededRandom(std::uint64_t seed);
        [[nodiscard]] std::uint64_t seed() const noexcept override;
        int between(int minimumInclusive, int maximumInclusive) override;
        bool chance(double probability) override;

      private:
        std::uint64_t seed_;
        std::mt19937_64 engine_;
    };
} // namespace soma::gameplay
