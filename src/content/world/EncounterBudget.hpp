#pragma once

#include "content/world/WorldContent.hpp"

#include <algorithm>
#include <vector>

namespace soma::content
{
    /// Describes which encounter-cost totals can be composed within a level's available non-boss rooms.
    class EncounterBudgetReachability
    {
      public:
        EncounterBudgetReachability(const LevelDefinition& level, const WorldContent& world)
            : reachable_(static_cast<std::size_t>(level.roomCount),
                         std::vector<bool>(static_cast<std::size_t>(level.maximumEncounterBudget + 1), false))
        {
            reachable_.front().front() = true;
            for (int rooms = 1; rooms < level.roomCount; ++rooms)
            {
                auto& totals = reachable_.at(static_cast<std::size_t>(rooms));
                totals.front() = true;
                for (int total = 1; total <= level.maximumEncounterBudget; ++total)
                {
                    totals.at(static_cast<std::size_t>(total)) =
                        std::ranges::any_of(level.encounters, [&world, this, rooms, total](const auto& encounterId) {
                            const auto cost = world.encounters.at(encounterId).budgetCost;
                            return cost <= total && reachable_.at(static_cast<std::size_t>(rooms - 1))
                                                        .at(static_cast<std::size_t>(total - cost));
                        });
                }
            }
        }

        [[nodiscard]] bool canFill(int budget) const
        {
            return budget >= 0 && static_cast<std::size_t>(budget) < reachable_.back().size() &&
                   reachable_.back().at(static_cast<std::size_t>(budget));
        }

        [[nodiscard]] const std::vector<std::vector<bool>>& byRoomCount() const noexcept
        {
            return reachable_;
        }

      private:
        std::vector<std::vector<bool>> reachable_;
    };

    /// Returns all achievable encounter totals inside the level's inclusive configured range.
    [[nodiscard]] inline std::vector<int> reachableEncounterBudgets(const LevelDefinition& level, const WorldContent& world)
    {
        const EncounterBudgetReachability reachability{level, world};
        std::vector<int> budgets;
        for (auto budget = level.minimumEncounterBudget; budget <= level.maximumEncounterBudget; ++budget)
        {
            if (reachability.canFill(budget))
            {
                budgets.push_back(budget);
            }
        }
        return budgets;
    }
} // namespace soma::content
