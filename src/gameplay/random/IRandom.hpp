#pragma once
#include <cstdint>

namespace soma::gameplay
{
    /// Abstracts random choices so simulations can be deterministic in tools and tests.
    class IRandom
    {
      public:
        virtual ~IRandom() = default;
        [[nodiscard]] virtual std::uint64_t seed() const noexcept = 0;
        virtual int between(int minimumInclusive, int maximumInclusive) = 0;
        virtual bool chance(double probability) = 0;
    };
} // namespace soma::gameplay
