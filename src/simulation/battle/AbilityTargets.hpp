#pragma once

#include "content/battle/BattleContent.hpp"
#include "simulation/battle/BattleSimulation.hpp"

#include <vector>

namespace soma::simulation
{
    [[nodiscard]] bool isValidPrimaryTarget(const content::Targeting& targeting,
                                            const CombatantState& actor,
                                            const CombatantState& candidate);

    [[nodiscard]] std::vector<gameplay::EntityId> abilityTargets(const content::Targeting& targeting,
                                                                  const CombatantState& actor,
                                                                  const CombatantState& primary,
                                                                  const std::vector<CombatantState>& combatants);

    [[nodiscard]] bool targetsActor(const content::Targeting& targeting,
                                    const CombatantState& actor,
                                    const CombatantState& primary);
} // namespace soma::simulation
