#include "simulation/battle/BattleSimulation.hpp"
#include "simulation/battle/BattleRuleSupport.hpp"
#include "simulation/rules/ModifierPipeline.hpp"

#include <algorithm>
#include <cmath>

namespace soma::simulation
{
    gameplay::CombatantAttributes BattleSimulation::effectiveAttributes(const CombatantState& combatant) const
    {
        auto result = combatant.attributes;
        result.damageMinimum = standingModifiedValue(content::ModifierKind::damageDealt, combatant, result.damageMinimum);
        result.damageMaximum = standingModifiedValue(content::ModifierKind::damageDealt, combatant, result.damageMaximum);
        result.armour = standingModifiedValue(content::ModifierKind::armour, combatant, result.armour);
        result.dodge = standingModifiedValue(content::ModifierKind::dodge, combatant, result.dodge);
        result.criticalHitChance = standingModifiedValue(content::ModifierKind::criticalHitChance, combatant, result.criticalHitChance);
        result.precision = standingModifiedValue(content::ModifierKind::precision, combatant, result.precision) - combatant.panicStacks * 5;
        result.initiative = standingModifiedValue(content::ModifierKind::initiative, combatant, result.initiative);
        return result;
    }

    int BattleSimulation::modifiedDamage(const CombatantState& source, const CombatantState& target, int baseDamage) const
    {
        auto dealtMultiplier = 1.0;
        for (const auto& buff : source.buffs)
        {
            dealtMultiplier += buff.damagePerNegativeStatus * buff.negativeStatusCountAtApplication;
        }
        for (const auto& debuff : source.debuffs)
        {
            dealtMultiplier += debuff.damagePerNegativeStatus * debuff.negativeStatusCountAtApplication;
        }
        const auto standingDamage = standingModifiedValue(content::ModifierKind::damageDealt, source, baseDamage);
        const auto dealtDamage = static_cast<int>(std::round(standingDamage * std::max(0.0, dealtMultiplier)));
        const auto receivedDamage = standingModifiedValue(content::ModifierKind::damageReceived, target, dealtDamage);
        const auto rageMultiplier = (source.rageTurns > 0 ? 1.5 : 1.0) * (target.rageTurns > 0 ? 1.5 : 1.0);
        return static_cast<int>(std::round(receivedDamage * rageMultiplier));
    }

    bool BattleSimulation::standingSubjectMatches(content::SubjectKind subject, const CombatantState& target) const
    {
        switch (subject)
        {
        case content::SubjectKind::allMarked:
            return target.markedTurns > 0;
        case content::SubjectKind::allConfused:
            return target.confusedTurns > 0;
        case content::SubjectKind::allSleeping:
            return target.sleeping;
        case content::SubjectKind::allDecaying:
            return !target.decay.empty();
        case content::SubjectKind::allFullHealth:
            return target.health == target.attributes.maximumHealth;
        case content::SubjectKind::allBuffed:
            return !target.buffs.empty();
        case content::SubjectKind::allDebuffed:
            return !target.debuffs.empty();
        case content::SubjectKind::allBelowNineFatigue:
            return target.fatigue < 9;
        case content::SubjectKind::allAboveEightFatigue:
            return target.fatigue >= 9;
        case content::SubjectKind::allOthers:
        case content::SubjectKind::allFriends:
        case content::SubjectKind::allOpponents:
        case content::SubjectKind::thisCombatant:
        case content::SubjectKind::randomCombatant:
        case content::SubjectKind::randomOpponent:
        case content::SubjectKind::randomFriend:
            return false;
        }
        return false;
    }

    int BattleSimulation::standingModifiedValue(content::ModifierKind kind, const CombatantState& target, int baseValue) const
    {
        if (rules_ == nullptr || activeRules_ == nullptr)
        {
            return baseValue;
        }
        ModifierPipeline pipeline;
        for (std::size_t slot = 0; slot < activeRules_->size(); ++slot)
        {
            const auto& rule = activeRules_->at(slot);
            if (!detail::isCompleteStandingRule(rule))
            {
                continue;
            }
            const auto& subject = std::get<content::SubjectFragment>(rules_->fragments.at(*rule.subject).payload);
            const auto& modifier = std::get<content::ModifierFragment>(rules_->fragments.at(*rule.modifier).payload);
            if (!standingSubjectMatches(subject.kind, target))
            {
                continue;
            }

            auto value = modifier.kind == kind ? modifier.value : 0;
            if (modifier.kind == content::ModifierKind::nofaceBuff)
            {
                const auto level = std::max(0, (target.attributes.maximumHealth - target.health) / 5) * modifier.value;
                if (kind == content::ModifierKind::damageDealt || kind == content::ModifierKind::precision)
                {
                    value = level * 2;
                }
                else if (kind == content::ModifierKind::criticalHitChance || kind == content::ModifierKind::initiative)
                {
                    value = level;
                }
            }
            if (value != 0)
            {
                pipeline.add({detail::activeRuleId(slot), ModifierOperation::additive, static_cast<double>(value), static_cast<int>(slot)});
            }
        }
        return static_cast<int>(pipeline.evaluate(baseValue).value);
    }
} // namespace soma::simulation
