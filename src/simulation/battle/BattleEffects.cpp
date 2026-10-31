#include "simulation/battle/BattleSimulation.hpp"
#include "simulation/battle/StatusResistance.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <ranges>
#include <unordered_set>

namespace soma::simulation
{
    namespace
    {
        gameplay::StatusId statusId(std::string value)
        {
            return gameplay::StatusId{"status." + std::move(value)};
        }

    } // namespace

    void BattleSimulation::applyEffect(CombatantState& actor,
                                       CombatantState& target,
                                       const content::AbilityEffect& effect,
                                       double additionalDamage,
                                       double damageFactorMultiplier,
                                       std::vector<gameplay::GameplayEvent>& events)
    {
        if (effect.removeDecayAndHealParty)
        {
            removeDecayAndHealLivingPlayers(actor, target, events);
        }
        if (effect.prolongNegativeEffects > 0)
        {
            prolongNegativeEffects(target, effect.prolongNegativeEffects, events);
        }
        const auto criticalChance = effectiveAttributes(actor).criticalHitChance + effect.criticalHitModifier;
        const auto critical = random_.chance(std::clamp(criticalChance, 0, 100) / 100.0);
        const auto effectFactor = critical ? 2 : 1;
        const auto durationBonus = critical ? 2 : 0;
        if (critical)
        {
            events.emplace_back(gameplay::CriticalHit{target.id});
        }

        applyDamageEffect(actor, target, effect, additionalDamage, damageFactorMultiplier, effectFactor, events);
        applyHealingEffects(actor, target, effect, effectFactor, events);
        applyStatusEffects(actor, target, effect, durationBonus, events);
        applyUtilityEffects(actor, target, effect, durationBonus, events);
    }

    void BattleSimulation::applyDamageEffect(CombatantState& actor,
                                             CombatantState& target,
                                             const content::AbilityEffect& effect,
                                             double additionalDamage,
                                             double damageFactorMultiplier,
                                             int effectFactor,
                                             std::vector<gameplay::GameplayEvent>& events)
    {
        if (effect.damageFactor != 0.0)
        {
            const auto baseDamage = random_.between(actor.attributes.damageMinimum, actor.attributes.damageMaximum);
            const auto markedBonus = target.markedTurns > 0 ? effect.damageAgainstMarked + damageAgainstMarkedBonus(actor) : 0.0;
            const auto revengeBonus = target.fistOfRevengeTarget ? effect.damageAgainstRevengeMarked : 0.0;
            auto fatigueBonus = 0.0;
            for (const auto& buff : actor.buffs)
            {
                if (buff.damageFromFatigue)
                {
                    fatigueBonus += static_cast<double>(actor.fatigue) / gameplay::maximumFatigue;
                }
            }
            const auto abilityDamage =
                static_cast<int>(baseDamage * (effect.damageFactor + additionalDamage + markedBonus + revengeBonus + fatigueBonus) *
                                 damageFactorMultiplier * effectFactor);
            const auto rawDamage = modifiedDamage(actor, target, abilityDamage);
            const auto armour = effect.ignoreArmour ? 0 : effectiveAttributes(target).armour;
            const auto armourReduction = static_cast<int>(std::round(armour / 100.0 * rawDamage));
            const auto damage = std::max(0, rawDamage - armourReduction);
            target.health = std::max(0, target.health - damage);
            if (damage >= 10 && definition(target).addsMissOnHighDamage)
            {
                ++target.misses;
                recordStatusGained(target, statusId("miss"), events);
            }
            updateNofaceBuff(target);
            recordDamageApplied(actor.id, target, damage, gameplay::DamageKind::direct, events);
            if (target.bountyTurns > 0)
            {
                content::BuffDefinition bounty;
                bounty.duration = 3;
                bounty.positive = true;
                bounty.attributes.damageMinimum = 2;
                bounty.attributes.damageMaximum = 2;
                applyBuff(actor, bounty, bounty.duration, events);
            }
            const auto defeatedByAttack = !target.alive();
            emitDefeatIfNeeded(target, actor.id, events);
            if (defeatedByAttack && effect.transferDecayOnKill)
            {
                transferDecayAfterDefeat(actor, target, events);
            }
        }
        if (effect.transferredMissingHealthDamageFactor != 0.0 || effect.transferredMissingHealthHealingFactor != 0.0)
        {
            auto* next = nextLivingTeamMember(target);
            if (next != nullptr)
            {
                const auto missingHealth = std::max(0, next->attributes.maximumHealth - next->health);
                const auto transferredDamage =
                    static_cast<int>(std::lround(missingHealth * effect.transferredMissingHealthDamageFactor * effectFactor));
                const auto transferredHealing =
                    static_cast<int>(std::lround(missingHealth * effect.transferredMissingHealthHealingFactor * effectFactor));
                if (target.alive())
                {
                    applyUnavoidableDamage(actor, target, transferredDamage, events);
                }
                if (next->alive())
                {
                    applyHealing(actor.id, *next, transferredHealing, events);
                }
            }
        }
        if (effect.inflictAllDecayAndRemove && target.alive())
        {
            inflictRemainingDecay(actor, target, events);
        }
    }

    void BattleSimulation::applyHealingEffects(CombatantState& actor,
                                               CombatantState& target,
                                               const content::AbilityEffect& effect,
                                               int effectFactor,
                                               std::vector<gameplay::GameplayEvent>& events)
    {
        if (effect.heal != 0)
        {
            const auto healing = static_cast<int>(effect.heal * (1.0 + actor.secondaryAttributes.healing / 100.0) * effectFactor);
            applyHealing(actor.id, target, healing, events);
        }
        if (effect.healSelf != 0)
        {
            auto& healingTarget = actor.confusedThisTurn ? target : actor;
            const auto healingModifier = 1.0 + actor.secondaryAttributes.healing / 100.0 + effectFactor;
            applyHealing(actor.id, healingTarget, static_cast<int>(effect.healSelf * healingModifier), events);
        }
    }

    void BattleSimulation::applyStatusEffects(CombatantState& actor,
                                              CombatantState& target,
                                              const content::AbilityEffect& effect,
                                              int durationBonus,
                                              std::vector<gameplay::GameplayEvent>& events)
    {
        const auto applyResistedStatus =
            [this, &target, &events](double probability, StatusResistance category, const gameplay::StatusId& status, auto apply) {
                if (!random_.chance(std::min(probability, 1.0)))
                {
                    return false;
                }
                if (random_.between(1, 100) <= resistance(target, category))
                {
                    events.emplace_back(gameplay::EffectResisted{target.id, status});
                    return false;
                }
                apply();
                return true;
            };

        if (effect.confusionRounds > 0)
        {
            applyResistedStatus(effect.confusionProbability, StatusResistance::confusion, statusId("confused"), [&] {
                const auto duration = std::max(0, effect.confusionRounds + durationBonus);
                if (duration > 0)
                {
                    target.confusedTurns += duration;
                    recordStatusGained(target, statusId("confused"), events);
                }
            });
        }
        if (effect.sleepProbability > 0.0)
        {
            auto sleepProbability = effect.sleepProbability;
            if (effect.increaseSleepProbabilityByFatigue)
            {
                sleepProbability += static_cast<double>(actor.fatigue) / gameplay::maximumFatigue;
            }
            const auto sleepApplied = applyResistedStatus(sleepProbability, StatusResistance::sleep, statusId("sleeping"), [&] {
                target.sleeping = true;
                recordStatusGained(target, statusId("sleeping"), events);
            });
            if (!sleepApplied && effect.sleepFailureDecayDamage > 0 && effect.sleepFailureDecayRounds > 0)
            {
                static_cast<void>(tryApplyDecay(
                    target, effect.sleepFailureDecayDamage, effect.sleepFailureDecayRounds + durationBonus, effect.ignoreDecayResistance, events));
            }
        }
        if (effect.decayRounds > 0)
        {
            const auto conditionalRounds =
                (target.confusedTurns > 0 ? effect.decayRoundsAgainstConfused : 0) + (target.sleeping ? effect.decayRoundsAgainstSleeping : 0);
            const auto conditionalDamage = target.markedTurns > 0 ? effect.decayDamageAgainstMarked : 0;
            const auto duration = std::max(0, effect.decayRounds + conditionalRounds + durationBonus);
            static_cast<void>(tryApplyDecay(target, effect.decayDamage + conditionalDamage, duration, effect.ignoreDecayResistance, events));
        }
        if (effect.markRounds > 0)
        {
            const auto duration = std::max(0, effect.markRounds + durationBonus);
            if (duration > 0)
            {
                target.markedTurns += duration;
                recordStatusGained(target, statusId("marked"), events);
            }
        }
        if (effect.bountyRounds > 0)
        {
            target.bountyTurns += std::max(0, effect.bountyRounds + durationBonus);
            recordStatusGained(target, statusId("bounty"), events);
        }
        if (effect.riposteRounds > 0)
        {
            target.riposteTurns += std::max(0, effect.riposteRounds + durationBonus);
            recordStatusGained(target, statusId("riposte"), events);
        }
        if (effect.riposteSelfRounds > 0)
        {
            actor.riposteTurns += std::max(0, effect.riposteSelfRounds + durationBonus);
            recordStatusGained(actor, statusId("riposte"), events);
        }
        if (effect.rageRounds > 0)
        {
            target.rageTurns += std::max(0, effect.rageRounds + durationBonus);
            recordStatusGained(target, statusId("rage"), events);
        }
        if (effect.hackedRounds > 0)
        {
            applyResistedStatus(1.0, StatusResistance::debuff, statusId("hacked"), [&] {
                target.hackedTurns += std::max(0, effect.hackedRounds + durationBonus);
                recordStatusGained(target, statusId("hacked"), events);
            });
        }
        if (effect.transfusionHealing > 0 && !target.decay.empty())
        {
            target.transfusions.push_back({effect.transfusionHealing});
            recordStatusGained(target, statusId("transfusion"), events);
        }
        if (effect.doublePanic && target.panicStacks > 0)
        {
            changePanic(target, target.panicStacks, events);
        }
        if (effect.damageFromPanic && target.panicStacks >= 10)
        {
            const auto removedStacks = target.panicStacks / 2;
            changePanic(target, -removedStacks, events);
            applyUnavoidableDamage(actor, target, removedStacks, events);
        }
        if (!target.alive())
        {
            return;
        }
        const auto panic = effect.panicStacks + (target.markedTurns > 0 ? effect.panicAgainstMarked : 0);
        if (panic > 0)
        {
            changePanic(target, panic, events);
        }
        if (effect.spreadPanicTargets > 0 && target.panicStacks > 0)
        {
            std::vector<CombatantState*> candidates;
            for (auto& candidate : combatants_)
            {
                if (candidate.alive() && candidate.team == target.team && candidate.id != target.id)
                {
                    candidates.push_back(&candidate);
                }
            }
            const auto attempts = std::min(effect.spreadPanicTargets, static_cast<int>(candidates.size()));
            for (auto attempt = 0; attempt < attempts; ++attempt)
            {
                const auto candidateIndex = static_cast<std::size_t>(random_.between(0, static_cast<int>(candidates.size()) - 1));
                auto* candidate = candidates.at(candidateIndex);
                candidates.erase(candidates.begin() + static_cast<std::ptrdiff_t>(candidateIndex));
                const auto probability = std::min(1.0, target.panicStacks * 0.05 * (attempt + 1));
                if (random_.chance(probability))
                {
                    changePanic(*candidate, 2, events);
                }
            }
        }
        applyRandomNegativeStatus(target, effect, durationBonus, events);
    }

    bool BattleSimulation::tryApplyDecay(
        CombatantState& target, int damage, int duration, bool ignoreResistance, std::vector<gameplay::GameplayEvent>& events)
    {
        damage = std::max(0, damage);
        duration = std::max(0, duration);
        if (damage == 0 || duration == 0)
        {
            return false;
        }
        if (!ignoreResistance && random_.between(1, 100) <= resistance(target, StatusResistance::decay))
        {
            events.emplace_back(gameplay::EffectResisted{target.id, statusId("decay")});
            return false;
        }
        target.decay.push_back({duration, damage});
        recordStatusGained(target, statusId("decay"), events);
        return true;
    }

    void BattleSimulation::applyRandomNegativeStatus(CombatantState& target,
                                                     const content::AbilityEffect& effect,
                                                     int durationBonus,
                                                     std::vector<gameplay::GameplayEvent>& events)
    {
        if (effect.randomNegativeStatusProbability <= 0.0 || !random_.chance(effect.randomNegativeStatusProbability))
        {
            return;
        }

        const auto duration = std::max(1, effect.randomNegativeStatusDuration + durationBonus);
        const auto resisted = [this, &target, &events](StatusResistance category, const gameplay::StatusId& status) {
            if (random_.between(1, 100) > resistance(target, category))
            {
                return false;
            }
            events.emplace_back(gameplay::EffectResisted{target.id, status});
            return true;
        };

        switch (random_.between(0, 6))
        {
        case 0: {
            if (resisted(StatusResistance::debuff, statusId("debuffed")))
            {
                return;
            }
            content::BuffDefinition debuff;
            debuff.duration = duration;
            debuff.positive = false;
            const auto percentage = random_.between(10, 20) / 100.0;
            const auto reduction = [percentage](int value) {
                return -std::max(1, static_cast<int>(std::round(std::abs(value) * percentage)));
            };
            const auto attributes = effectiveAttributes(target);
            switch (random_.between(0, 5))
            {
            case 0:
                debuff.attributes.armour = reduction(attributes.armour);
                break;
            case 1:
                debuff.attributes.damageMinimum = reduction(attributes.damageMinimum);
                debuff.attributes.damageMaximum = reduction(attributes.damageMaximum);
                break;
            case 2:
                debuff.attributes.initiative = reduction(attributes.initiative);
                break;
            case 3:
                debuff.attributes.criticalHitChance = reduction(attributes.criticalHitChance);
                break;
            case 4:
                debuff.attributes.precision = reduction(attributes.precision);
                break;
            default:
                debuff.attributes.dodge = reduction(attributes.dodge);
                break;
            }
            applyBuff(target, debuff, duration, events);
            break;
        }
        case 1:
            if (!resisted(StatusResistance::sleep, statusId("sleeping")))
            {
                target.sleeping = true;
                recordStatusGained(target, statusId("sleeping"), events);
            }
            break;
        case 2:
            if (!resisted(StatusResistance::confusion, statusId("confused")))
            {
                target.confusedTurns += duration;
                recordStatusGained(target, statusId("confused"), events);
            }
            break;
        case 3:
            static_cast<void>(tryApplyDecay(target, random_.between(1, 2), duration, false, events));
            break;
        case 4:
            if (!resisted(StatusResistance::debuff, statusId("bounty")))
            {
                target.bountyTurns += duration;
                recordStatusGained(target, statusId("bounty"), events);
            }
            break;
        case 5:
            if (!resisted(StatusResistance::debuff, statusId("marked")))
            {
                target.markedTurns += duration;
                recordStatusGained(target, statusId("marked"), events);
            }
            break;
        default:
            if (!resisted(StatusResistance::debuff, statusId("hacked")))
            {
                target.hackedTurns += duration;
                recordStatusGained(target, statusId("hacked"), events);
            }
            break;
        }
    }

    void BattleSimulation::prolongNegativeEffects(CombatantState& target, int rounds, std::vector<gameplay::GameplayEvent>& events)
    {
        rounds = std::max(0, rounds);
        const auto hasNegativeEffect = !target.debuffs.empty() || !target.decay.empty() || target.confusedTurns > 0 || target.markedTurns > 0 ||
                                       target.hackedTurns > 0 || target.bountyTurns > 0;
        for (auto& debuff : target.debuffs)
        {
            debuff.remainingTurns += rounds;
        }
        for (auto& decay : target.decay)
        {
            decay.remainingTurns += rounds;
        }
        if (target.confusedTurns > 0)
        {
            target.confusedTurns += rounds;
        }
        if (target.markedTurns > 0)
        {
            target.markedTurns += rounds;
        }
        if (target.hackedTurns > 0)
        {
            target.hackedTurns += rounds;
        }
        if (target.bountyTurns > 0)
        {
            target.bountyTurns += rounds;
        }
        if (hasNegativeEffect && rounds > 0)
        {
            events.emplace_back(gameplay::NegativeEffectsProlonged{target.id, rounds});
        }
    }

} // namespace soma::simulation
