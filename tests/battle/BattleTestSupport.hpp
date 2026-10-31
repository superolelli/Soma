#pragma once

#include "content/battle/BattleContentLoader.hpp"
#include "content/rules/RuleContentLoader.hpp"
#include "gameplay/random/SeededRandom.hpp"
#include "simulation/battle/AbilityTargets.hpp"
#include "simulation/battle/BattleSimulation.hpp"
#include "simulation/battle/EnemyPolicy.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <ranges>
#include <set>
#include <source_location>
#include <stdexcept>

namespace soma::tests
{
    class MinimumRandom final : public gameplay::IRandom
    {
      public:
        [[nodiscard]] std::uint64_t seed() const noexcept override { return 0; }
        int between(int minimumInclusive, int maximumInclusive) override
        {
            checkRange(minimumInclusive, maximumInclusive);
            return minimumInclusive;
        }
        bool chance(double probability) override { return probability > 0.0; }

      private:
        static void checkRange(int minimumInclusive, int maximumInclusive)
        {
            if (minimumInclusive > maximumInclusive)
            {
                throw std::invalid_argument("Random range minimum exceeds maximum");
            }
        }
    };

    class MaximumRandom final : public gameplay::IRandom
    {
      public:
        [[nodiscard]] std::uint64_t seed() const noexcept override { return 0; }
        int between(int minimumInclusive, int maximumInclusive) override
        {
            if (minimumInclusive > maximumInclusive)
            {
                throw std::invalid_argument("Random range minimum exceeds maximum");
            }
            return maximumInclusive;
        }
        bool chance(double probability) override { return probability > 0.0; }
    };

    inline void check(bool condition, const std::source_location location = std::source_location::current())
    {
        if (!condition)
        {
            std::cerr << "Check failed at " << location.file_name() << ':' << location.line() << '\n';
            std::exit(1);
        }
    }

    inline gameplay::EntityId entity(const std::string& definition, int slot)
    {
        return gameplay::EntityId{definition + ".slot" + std::to_string(slot)};
    }

    void runBattleCoreTests(const content::BattleContentLoadResult& loaded, const content::RuleContentLoadResult& rules);
    void runBattleStatusEffectTests(const content::BattleContentLoadResult& loaded, const content::RuleContentLoadResult& rules);
    void runAdvancedBattleMechanicTests(const content::BattleContentLoadResult& loaded);
} // namespace soma::tests
