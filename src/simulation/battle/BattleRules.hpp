#pragma once

#include "gameplay/combat/CombatTypes.hpp"
#include "gameplay/events/GameplayEvent.hpp"
#include "simulation/rules/ModifierPipeline.hpp"

#include <functional>
#include <string>
#include <vector>

namespace soma::simulation
{
    struct ModifierQuery
    {
        std::string name;
        gameplay::EntityId source;
        gameplay::EntityId target;
    };

    struct RuleEffect
    {
        gameplay::RuleId rule;
        gameplay::EntityId source;
        gameplay::EntityId target;
        gameplay::EffectSpec effect;
    };

    struct BattleRuleHooks
    {
        std::function<void(const ModifierQuery&, ModifierPipeline&)> contributeModifiers;
        std::function<std::vector<RuleEffect>(const gameplay::GameplayEvent&)> react;
    };
} // namespace soma::simulation
