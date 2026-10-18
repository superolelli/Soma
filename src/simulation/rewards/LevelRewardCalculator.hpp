#pragma once

#include <string>
#include <vector>

namespace soma::simulation
{
    struct LevelPerformance
    {
        int difficulty{};
        bool completed{};
        int playerDeaths{};
        int enemiesDefeated{};
        bool allAbilitiesUsed{};
        int fatigue{};
    };

    struct RewardAchievement
    {
        int cards{};
        int dice{};
        std::string description;

        auto operator<=>(const RewardAchievement&) const = default;
    };

    struct LevelRewards
    {
        std::vector<RewardAchievement> achievements;

        [[nodiscard]] int totalCards() const noexcept;
        [[nodiscard]] int totalDice() const noexcept;
    };

    /// Converts end-of-level performance into the displayed card and dice rewards.
    class LevelRewardCalculator
    {
      public:
        [[nodiscard]] LevelRewards calculate(const LevelPerformance& performance) const;
    };
} // namespace soma::simulation
