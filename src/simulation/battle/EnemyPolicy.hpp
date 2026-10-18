#pragma once

#include "gameplay/common/Result.hpp"
#include "gameplay/random/IRandom.hpp"
#include "simulation/battle/BattleSimulation.hpp"

#include <optional>

namespace soma::simulation
{
    /// Selects legal enemy abilities and targets according to each enemy archetype.
    class EnemyPolicy
    {
      public:
        explicit EnemyPolicy(gameplay::IRandom& random);

        [[nodiscard]] gameplay::Result<BattleActionCommand> choose(const BattleSimulation& battle) const;

      private:
        [[nodiscard]] gameplay::AbilityId chooseAbility(const BattleSimulation& battle, const CombatantState& actor) const;
        [[nodiscard]] std::optional<gameplay::AbilityId> chooseOrdinaryEnemyAbility(const BattleSimulation& battle,
                                                                                    const CombatantState& actor) const;
        [[nodiscard]] std::optional<gameplay::AbilityId> chooseBangBossAbility(const BattleSimulation& battle, const CombatantState& actor) const;
        [[nodiscard]] gameplay::AbilityId chooseKutschfahrtAbility(const BattleSimulation& battle, const CombatantState& actor) const;
        [[nodiscard]] std::optional<gameplay::EntityId> chooseTarget(const BattleSimulation& battle,
                                                                     const CombatantState& actor,
                                                                     const gameplay::AbilityId& ability) const;

        gameplay::IRandom& random_;
    };
} // namespace soma::simulation
