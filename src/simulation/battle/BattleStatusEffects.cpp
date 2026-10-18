#include "simulation/battle/BattleSimulation.hpp"

#include <algorithm>
#include <cmath>
#include <ranges>

namespace soma::simulation
{
    namespace
    {
        gameplay::StatusId statusId(std::string value)
        {
            return gameplay::StatusId{"status." + std::move(value)};
        }

        int resistance(const CombatantState& target, const std::string& kind)
        {
            if (kind == "sleep")
            {
                return target.secondaryAttributes.sleepResistance;
            }
            if (kind == "confusion")
            {
                return target.secondaryAttributes.confusionResistance;
            }
            if (kind == "decay")
            {
                return target.secondaryAttributes.decayResistance;
            }
            return target.secondaryAttributes.debuffResistance;
        }
    } // namespace

    void BattleSimulation::applyUtilityEffects(CombatantState& actor,
                                               CombatantState& target,
                                               const content::AbilityEffect& effect,
                                               int durationBonus,
                                               std::vector<gameplay::GameplayEvent>& events)
    {
        if (effect.addMiss > 0)
        {
            target.misses += effect.addMiss;
            recordStatusGained(target, statusId("miss"), events);
        }
        if (effect.addMissSelf > 0)
        {
            auto& missTarget = actor.confusedThisTurn ? target : actor;
            missTarget.misses += effect.addMissSelf;
            recordStatusGained(missTarget, statusId("miss"), events);
        }
        if (effect.dynamite)
        {
            target.dynamite = true;
            recordStatusGained(target, statusId("dynamite"), events);
        }
        if (effect.targetFatigue != 0)
        {
            changeFatigue(target, effect.targetFatigue, events);
        }
        if (effect.removeBuffs)
        {
            removeAllBuffs(target, events);
        }
        if (effect.removeDebuffs)
        {
            removeAllDebuffs(target, events);
        }
        if (effect.stealBuffs && !target.buffs.empty())
        {
            for (const auto& buff : target.buffs)
            {
                addAttributes(actor, buff.attributes, 1);
                actor.buffs.push_back(buff);
            }
            removeAllBuffs(target, events);
        }
        const auto applyEffectBuff = [this, &actor, &target, durationBonus, &events](const content::BuffDefinition& buff, bool targetsActor) {
            if (buff.duration <= 0)
            {
                return;
            }
            const auto duration = buff.duration + durationBonus;
            auto& buffTarget = targetsActor && !actor.confusedThisTurn ? actor : target;
            if (!buff.positive && random_.between(1, 100) <= resistance(buffTarget, "debuff"))
            {
                events.emplace_back(gameplay::EffectResisted{buffTarget.id, statusId("debuffed")});
            }
            else
            {
                applyBuff(buffTarget, buff, duration, events);
            }
        };
        applyEffectBuff(effect.targetBuff, false);
        applyEffectBuff(effect.actorBuff, true);
    }

    void BattleSimulation::applyBuff(
        CombatantState& target, const content::BuffDefinition& buff, int duration, std::vector<gameplay::GameplayEvent>& events, bool ignoresHacked)
    {
        duration = std::max(0, duration);
        if (duration == 0)
        {
            return;
        }
        if (buff.positive && target.hackedTurns > 0 && !ignoresHacked)
        {
            events.emplace_back(gameplay::StatusTriggered{target.id, statusId("hacked")});
            return;
        }
        auto& collection = buff.positive ? target.buffs : target.debuffs;
        const auto sameAttributes = [&buff](const ActiveBuff& active) {
            const auto& left = active.attributes;
            const auto& right = buff.attributes;
            return left.armour == right.armour && left.damageMinimum == right.damageMinimum && left.damageMaximum == right.damageMaximum &&
                   left.initiative == right.initiative && left.maximumHealth == right.maximumHealth &&
                   left.criticalHitChance == right.criticalHitChance && left.precision == right.precision && left.dodge == right.dodge &&
                   left.healing == right.healing && left.sleepResistance == right.sleepResistance &&
                   left.confusionResistance == right.confusionResistance && left.debuffResistance == right.debuffResistance &&
                   left.decayResistance == right.decayResistance && active.healingReceived == buff.healingReceived &&
                   active.damageAgainstMarked == buff.damageAgainstMarked && active.damagePerNegativeStatus == buff.damagePerNegativeStatus &&
                   active.damageFromFatigue == buff.damageFromFatigue && active.blocksFatigueStageOne == buff.blocksFatigueStageOne &&
                   active.blocksFatigueStageTwo == buff.blocksFatigueStageTwo;
        };
        const auto existing = std::ranges::find_if(collection, sameAttributes);
        if (existing != collection.end())
        {
            existing->remainingTurns += duration;
            recordStatusGained(target, statusId(buff.positive ? "buffed" : "debuffed"), events);
            return;
        }
        const auto negativeStatusCount = negativeStatusCategoryCount(target);
        addAttributes(target, buff.attributes, 1);
        collection.push_back({buff.attributes,
                              duration,
                              buff.positive,
                              buff.healingReceived,
                              buff.damageAgainstMarked,
                              buff.damagePerNegativeStatus,
                              negativeStatusCount,
                              buff.damageFromFatigue,
                              buff.blocksFatigueStageOne,
                              buff.blocksFatigueStageTwo});
        recordStatusGained(target, statusId(buff.positive ? "buffed" : "debuffed"), events);
    }

    void BattleSimulation::applyHealing(const gameplay::EntityId& source,
                                        CombatantState& target,
                                        int baseHealing,
                                        std::vector<gameplay::GameplayEvent>& events)
    {
        const auto oldHealth = target.health;
        target.health = std::min(target.attributes.maximumHealth, target.health + modifiedHealingReceived(target, baseHealing));
        updateNofaceBuff(target);
        const auto appliedHealing = target.health - oldHealth;
        events.emplace_back(gameplay::HealingApplied{source, target.id, appliedHealing});
        if (appliedHealing > 0 && target.panicStacks > 0)
        {
            changePanic(target, -1, events);
        }
    }

    void BattleSimulation::changePanic(CombatantState& target, int amount, std::vector<gameplay::GameplayEvent>& events)
    {
        const auto previous = target.panicStacks;
        target.panicStacks = std::max(0, target.panicStacks + amount);
        if (target.panicStacks > previous)
        {
            events.emplace_back(gameplay::StatusGained{target.id, statusId("panic")});
        }
        else if (previous > 0 && target.panicStacks == 0)
        {
            events.emplace_back(gameplay::StatusExpired{target.id, statusId("panic")});
        }
    }

    void BattleSimulation::recordStatusGained(CombatantState& target, const gameplay::StatusId& status, std::vector<gameplay::GameplayEvent>& events)
    {
        events.emplace_back(gameplay::StatusGained{target.id, status});
        if (target.panicStacks == 0 || status == statusId("panic"))
        {
            return;
        }
        const auto& value = status.value();
        const auto negative = value == "status.debuffed" || value == "status.sleeping" || value == "status.confused" || value == "status.decay" ||
                              value == "status.bounty" || value == "status.marked" || value == "status.hacked" || value == "status.transfusion";
        const auto positive = value == "status.buffed" || value == "status.riposte" || value == "status.miss";
        if (negative)
        {
            changePanic(target, 1, events);
        }
        else if (positive)
        {
            changePanic(target, -1, events);
        }
    }

    int BattleSimulation::negativeStatusCategoryCount(const CombatantState& target) const
    {
        return static_cast<int>(!target.debuffs.empty()) + static_cast<int>(target.sleeping) + static_cast<int>(target.confusedTurns > 0) +
               static_cast<int>(!target.decay.empty()) + static_cast<int>(target.bountyTurns > 0) + static_cast<int>(target.markedTurns > 0) +
               static_cast<int>(target.hackedTurns > 0) + static_cast<int>(!target.transfusions.empty()) + static_cast<int>(target.panicStacks > 0);
    }

    int BattleSimulation::modifiedHealingReceived(const CombatantState& target, int baseHealing) const
    {
        auto modifier = 1.0;
        for (const auto& buff : target.buffs)
        {
            modifier += buff.healingReceived;
        }
        for (const auto& debuff : target.debuffs)
        {
            modifier += debuff.healingReceived;
        }
        return std::max(0, static_cast<int>(std::lround(std::max(0, baseHealing) * modifier)));
    }

    double BattleSimulation::damageAgainstMarkedBonus(const CombatantState& source) const
    {
        auto result = 0.0;
        for (const auto& buff : source.buffs)
        {
            result += buff.damageAgainstMarked;
        }
        for (const auto& debuff : source.debuffs)
        {
            result += debuff.damageAgainstMarked;
        }
        return result;
    }

    void BattleSimulation::addAttributes(CombatantState& target, const content::AttributeDelta& value, int sign)
    {
        target.attributes.armour += value.armour * sign;
        target.attributes.damageMinimum += value.damageMinimum * sign;
        target.attributes.damageMaximum += value.damageMaximum * sign;
        target.attributes.initiative += value.initiative * sign;
        target.attributes.maximumHealth += value.maximumHealth * sign;
        target.attributes.criticalHitChance += value.criticalHitChance * sign;
        target.attributes.precision += value.precision * sign;
        target.attributes.dodge += value.dodge * sign;
        target.secondaryAttributes.healing += value.healing * sign;
        target.secondaryAttributes.sleepResistance += value.sleepResistance * sign;
        target.secondaryAttributes.confusionResistance += value.confusionResistance * sign;
        target.secondaryAttributes.debuffResistance += value.debuffResistance * sign;
        target.secondaryAttributes.decayResistance += value.decayResistance * sign;
    }

    void BattleSimulation::removeAllBuffs(CombatantState& target, std::vector<gameplay::GameplayEvent>& events)
    {
        const auto removedOrdinaryBuff = !target.buffs.empty();
        const auto removedRiposte = target.riposteTurns > 0;
        for (const auto& buff : target.buffs)
        {
            addAttributes(target, buff.attributes, -1);
        }
        target.buffs.clear();
        if (target.riposteTurns > 0)
        {
            target.riposteTurns = 0;
            events.emplace_back(gameplay::StatusExpired{target.id, statusId("riposte")});
        }
        target.health = std::min(target.health, target.attributes.maximumHealth);
        if (removedOrdinaryBuff || removedRiposte)
        {
            events.emplace_back(gameplay::BuffsRemoved{target.id});
        }
    }

    void BattleSimulation::removeAllDebuffs(CombatantState& target, std::vector<gameplay::GameplayEvent>& events)
    {
        const auto removedDebuff = !target.debuffs.empty();
        for (const auto& debuff : target.debuffs)
        {
            addAttributes(target, debuff.attributes, -1);
        }
        target.debuffs.clear();
        if (removedDebuff)
        {
            events.emplace_back(gameplay::DebuffsRemoved{target.id});
        }
    }

} // namespace soma::simulation
