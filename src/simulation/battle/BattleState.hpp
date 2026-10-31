#pragma once

#include "content/battle/BattleContent.hpp"

#include <optional>
#include <vector>

namespace soma::simulation
{
    /// Runtime representation of an ordinary attribute buff or debuff on one combatant.
    struct ActiveBuff
    {
        content::AttributeDelta attributes;
        int remainingTurns{};
        bool positive{};
        double healingReceived{};
        double damageAgainstMarked{};
        double damagePerNegativeStatus{};
        int negativeStatusCountAtApplication{};
        bool damageFromFatigue{};
        bool blocksFatigueStageOne{};
        bool blocksFatigueStageTwo{};
    };

    /// Describes one combatant when constructing a battle, including state carried in from exploration.
    struct BattleSpawn
    {
        gameplay::ContentDefinitionId definition;
        int slot{};
        std::optional<int> initialHealth;
        std::vector<ActiveBuff> initialBuffs;
        int initialFatigue{};
        double healthAndDamageMultiplier{1.0};
    };

    /// Tracks one independently expiring application of decay.
    struct ActiveDecay
    {
        int remainingTurns{};
        int damage{};
    };

    /// Tracks one transfusion application while its owner still has decay.
    struct ActiveTransfusion
    {
        int healing{};
    };

    /// Contains all authoritative mutable state for one combatant in the current battle.
    struct CombatantState
    {
        gameplay::EntityId id;
        gameplay::ContentDefinitionId definition;
        std::string name;
        gameplay::Team team{};
        int slot{};
        gameplay::CombatantAttributes attributes;
        content::AttributeDelta secondaryAttributes;
        int health{};
        std::vector<ActiveBuff> buffs;
        std::vector<ActiveBuff> debuffs;
        std::vector<ActiveDecay> decay;
        std::vector<ActiveTransfusion> transfusions;
        int confusedTurns{};
        int markedTurns{};
        int bountyTurns{};
        int misses{};
        bool sleeping{};
        bool dynamite{};
        bool fistOfRevengeTarget{};
        bool confusedThisTurn{};
        int nofaceBuffLevel{-1};
        int fatigue{};
        int riposteTurns{};
        int rageTurns{};
        int hackedTurns{};
        int panicStacks{};
        int appliedFatiguePenaltyStages{};
        gameplay::CombatantAttributes baseAttributes;

        [[nodiscard]] bool alive() const noexcept;
    };
} // namespace soma::simulation
