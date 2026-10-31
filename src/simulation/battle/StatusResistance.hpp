#pragma once

#include "simulation/battle/BattleState.hpp"

namespace soma::simulation
{
    enum class StatusResistance
    {
        sleep,
        confusion,
        debuff,
        decay
    };

    /// Returns the combatant's current resistance percentage for a typed status category.
    [[nodiscard]] inline int resistance(const CombatantState& combatant, StatusResistance category)
    {
        switch (category)
        {
            case StatusResistance::sleep:
                return combatant.secondaryAttributes.sleepResistance;
            case StatusResistance::confusion:
                return combatant.secondaryAttributes.confusionResistance;
            case StatusResistance::debuff:
                return combatant.secondaryAttributes.debuffResistance;
            case StatusResistance::decay:
                return combatant.secondaryAttributes.decayResistance;
        }
        return 0;
    }
} // namespace soma::simulation
