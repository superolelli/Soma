#pragma once

#include "gameplay/common/StrongId.hpp"

#include <string>
#include <variant>
#include <vector>

namespace soma::gameplay
{
    enum class Team
    {
        players,
        enemies
    };

    struct CombatantAttributes
    {
        int maximumHealth{};
        int damageMinimum{};
        int damageMaximum{};
        int armour{};
        int criticalHitChance{};
        int dodge{};
        int initiative{};
        int precision{};
    };

    struct StatusInstance
    {
        StatusId id;
        int remainingTurns{};
    };

    struct CombatantState
    {
        EntityId id;
        std::string name;
        Team team{};
        CombatantAttributes attributes;
        int health{};
        std::vector<StatusInstance> statuses;

        [[nodiscard]] bool alive() const noexcept
        {
            return health > 0;
        }
    };

    enum class TargetSide
    {
        self,
        allies,
        enemies
    };

    struct TargetingRules
    {
        TargetSide side{TargetSide::enemies};
        int minimumTargets{1};
        int maximumTargets{1};
    };

    struct DamageEffect
    {
        double factor{1.0};
    };

    struct HealEffect
    {
        int amount{};
    };

    struct AddStatusEffect
    {
        StatusId status;
        int duration{};
    };

    using EffectSpec = std::variant<DamageEffect, HealEffect, AddStatusEffect>;

    struct AbilityDefinition
    {
        AbilityId id;
        std::string name;
        TargetingRules targeting;
        std::vector<EffectSpec> effects;
    };

    struct CombatantDefinition
    {
        ContentDefinitionId id;
        std::string name;
        Team team{};
        CombatantAttributes attributes;
        std::vector<AbilityId> abilities;
    };

    struct ConsumableDefinition
    {
        ItemId id;
        std::string name;
        TargetingRules targeting;
        std::vector<EffectSpec> effects;
    };
} // namespace soma::gameplay
