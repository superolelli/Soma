#include "simulation/rewards/LevelRewardCalculator.hpp"

#include <numeric>

namespace soma::simulation
{
    int LevelRewards::totalCards() const noexcept
    {
        return std::accumulate(achievements.begin(), achievements.end(), 0, [](int total, const auto& achievement) {
            return total + achievement.cards;
        });
    }

    int LevelRewards::totalDice() const noexcept
    {
        return std::accumulate(achievements.begin(), achievements.end(), 0, [](int total, const auto& achievement) {
            return total + achievement.dice;
        });
    }

    LevelRewards LevelRewardCalculator::calculate(const LevelPerformance& performance) const
    {
        LevelRewards result;
        if (performance.completed)
        {
            result.achievements.push_back({10 * performance.difficulty, performance.difficulty, "Level geschafft"});
        }
        if (performance.playerDeaths == 0)
        {
            result.achievements.push_back({20, 0, "Niemanden zurückgelassen"});
        }
        if (performance.playerDeaths == 3)
        {
            result.achievements.push_back({30, 0, "Knappe Kiste"});
        }
        if (performance.enemiesDefeated > 0)
        {
            result.achievements.push_back(
                {2 * performance.enemiesDefeated, 0, std::to_string(performance.enemiesDefeated) + " besiegte Gegner"});
        }
        if (performance.allAbilitiesUsed)
        {
            result.achievements.push_back({25, 0, "Jede Fähigkeit ist nützlich"});
        }
        if (performance.fatigue >= 18)
        {
            result.achievements.push_back({30, 0, "Der Müdigkeit zum Trotz"});
        }
        return result;
    }
} // namespace soma::simulation
