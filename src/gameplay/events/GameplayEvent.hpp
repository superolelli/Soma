#pragma once

#include "gameplay/combat/CombatTypes.hpp"

#include <variant>

namespace soma::gameplay
{
    enum class DamageKind
    {
        direct,
        decay
    };

    struct TurnStarted
    {
        EntityId combatant;
    };

    struct AbilityUsed
    {
        EntityId combatant;
        AbilityId ability;
    };

    struct TargetRedirected
    {
        EntityId actor;
        EntityId originalTarget;
        EntityId resolvedTarget;
    };

    struct AttackDodged
    {
        EntityId source;
        EntityId target;
    };

    struct DamageApplied
    {
        EntityId source;
        EntityId target;
        int amount{};
        DamageKind kind{DamageKind::direct};
    };

    struct HealingApplied
    {
        EntityId source;
        EntityId target;
        int amount{};
    };

    struct FatigueChanged
    {
        EntityId combatant;
        int amount{};
        int previous{};
        int current{};
    };

    struct StatusGained
    {
        EntityId combatant;
        StatusId status;
    };

    struct StatusExpired
    {
        EntityId combatant;
        StatusId status;
    };

    struct StatusTriggered
    {
        EntityId combatant;
        StatusId status;
    };

    struct CombatantDefeated
    {
        EntityId combatant;
    };

    struct CriticalHit
    {
        EntityId combatant;
    };

    struct EffectResisted
    {
        EntityId combatant;
        StatusId status;
    };

    struct BuffsRemoved
    {
        EntityId combatant;
    };

    struct DebuffsRemoved
    {
        EntityId combatant;
    };

    struct DecayTransferred
    {
        EntityId source;
        EntityId target;
    };

    struct DecayDetonated
    {
        EntityId combatant;
    };

    struct NegativeEffectsProlonged
    {
        EntityId combatant;
        int rounds{};
    };

    struct CombatantSummoned
    {
        EntityId combatant;
        ContentDefinitionId definition;
        int slot{};
    };

    struct EncounterStarted
    {
        ContentDefinitionId encounter;
    };

    struct LootGranted
    {
        ContentDefinitionId reward;
        int quantity{};
    };

    struct RuleActivated
    {
        RuleId rule;
        EntityId trigger;
    };

    struct ItemActivated
    {
        ItemId item;
        EntityId carrier;
    };

    struct BattleFinished
    {
        Team winningTeam{};
    };

    using GameplayEvent = std::variant<TurnStarted,
                                       AbilityUsed,
                                       TargetRedirected,
                                       AttackDodged,
                                       DamageApplied,
                                       HealingApplied,
                                       FatigueChanged,
                                       StatusGained,
                                       StatusExpired,
                                       StatusTriggered,
                                       CombatantDefeated,
                                       CriticalHit,
                                       EffectResisted,
                                       BuffsRemoved,
                                       DebuffsRemoved,
                                       DecayTransferred,
                                       DecayDetonated,
                                       NegativeEffectsProlonged,
                                       CombatantSummoned,
                                       EncounterStarted,
                                       LootGranted,
                                       RuleActivated,
                                       ItemActivated,
                                       BattleFinished>;
} // namespace soma::gameplay
