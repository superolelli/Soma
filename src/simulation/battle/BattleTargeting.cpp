#include "simulation/battle/AbilityTargets.hpp"
#include "simulation/battle/BattleSimulation.hpp"

#include <algorithm>
#include <cmath>
#include <ranges>
#include <unordered_set>

namespace soma::simulation
{
    namespace
    {
        constexpr int slotsPerTeam = 4;

        gameplay::StatusId statusId(std::string value)
        {
            return gameplay::StatusId{"status." + std::move(value)};
        }
    } // namespace

    std::vector<CombatantState*> BattleSimulation::expandTargets(CombatantState& actor,
                                                                 const content::AbilityDefinition& ability,
                                                                 CombatantState& primaryTarget)
    {
        if (ability.targeting.scope == content::TargetScope::randomSingle)
        {
            return {&primaryTarget};
        }
        std::vector<CombatantState*> result;
        for (const auto& id : abilityTargets(ability.targeting, actor, primaryTarget, combatants_))
        {
            if (auto* target = findMutable(id))
            {
                result.push_back(target);
            }
        }
        return result;
    }

    void BattleSimulation::applyAbilityToTargets(CombatantState& actor,
                                                 const content::AbilityDefinition& ability,
                                                 const std::vector<CombatantState*>& targets,
                                                 std::vector<gameplay::GameplayEvent>& events)
    {
        const auto additionalDamage =
            ability.hostileEffect.lessTargetsMoreDamage * (ability.targeting.targetCount - static_cast<int>(targets.size()));
        std::vector<CombatantState*> randomlyMovedTargets;
        for (std::size_t index = 0; index < targets.size(); ++index)
        {
            auto* target = targets.at(index);
            const auto friendly = (actor.team == target->team) != actor.confusedThisTurn;
            const auto& effect = friendly ? ability.friendlyEffect : ability.hostileEffect;
            if (!friendly && target != &actor && attackIsDodged(actor, *target, ability, events))
            {
                if (effect.damageFactor != 0.0)
                {
                    resolveRiposte(*target, actor, events);
                }
                continue;
            }
            const auto remainingDamageFactor = std::clamp(1.0 - effect.successiveTargetDamageReduction, 0.0, 1.0);
            const auto successiveFactor = std::pow(remainingDamageFactor, index);
            applyEffect(actor, *target, effect, additionalDamage, successiveFactor, events);
            if (effect.moveTargetsRandomly && target->alive())
            {
                randomlyMovedTargets.push_back(target);
            }
            if (!friendly && target != &actor && effect.damageFactor != 0.0)
            {
                resolveRiposte(*target, actor, events);
            }
            if (!actor.alive())
            {
                break;
            }
        }
        if (!randomlyMovedTargets.empty())
        {
            moveTargetsRandomly(randomlyMovedTargets);
        }

        if (ability.allCombatantsBuff.duration > 0)
        {
            for (auto& combatant : combatants_)
            {
                if (combatant.alive())
                {
                    applyBuff(combatant, ability.allCombatantsBuff, ability.allCombatantsBuff.duration, events);
                }
            }
        }
    }

    void BattleSimulation::moveTargetsRandomly(const std::vector<CombatantState*>& targets)
    {
        const auto team = targets.front()->team;
        const auto firstSlot = targets.front()->slot / slotsPerTeam * slotsPerTeam;
        std::unordered_set<gameplay::EntityId> targetIds;
        for (const auto* target : targets)
        {
            targetIds.insert(target->id);
        }

        std::vector<int> availableSlots;
        for (auto slot = firstSlot; slot < firstSlot + slotsPerTeam; ++slot)
        {
            const auto occupiedByOther = std::ranges::any_of(combatants_, [team, slot, &targetIds](const auto& combatant) {
                return combatant.alive() && combatant.team == team && combatant.slot == slot && !targetIds.contains(combatant.id);
            });
            if (!occupiedByOther)
            {
                availableSlots.push_back(slot);
            }
        }
        for (auto index = availableSlots.size(); index > 1; --index)
        {
            const auto other = static_cast<std::size_t>(random_.between(0, static_cast<int>(index) - 1));
            std::swap(availableSlots.at(index - 1), availableSlots.at(other));
        }
        for (std::size_t index = 0; index < targets.size(); ++index)
        {
            targets.at(index)->slot = availableSlots.at(index);
        }
    }

    CombatantState* BattleSimulation::nextLivingTeamMember(const CombatantState& combatant)
    {
        const auto firstSlot = combatant.slot / slotsPerTeam * slotsPerTeam;
        const auto offsetInTeam = combatant.slot - firstSlot;
        for (auto offset = 1; offset < slotsPerTeam; ++offset)
        {
            const auto slot = firstSlot + (offsetInTeam + offset) % slotsPerTeam;
            const auto found = std::ranges::find_if(combatants_, [&combatant, slot](const auto& candidate) {
                return candidate.alive() && candidate.team == combatant.team && candidate.slot == slot;
            });
            if (found != combatants_.end())
            {
                return &*found;
            }
        }
        return nullptr;
    }

    void BattleSimulation::resolveRiposte(CombatantState& counteringCombatant, CombatantState& attacker, std::vector<gameplay::GameplayEvent>& events)
    {
        if (counteringCombatant.riposteTurns <= 0 || !counteringCombatant.alive() || !attacker.alive())
        {
            return;
        }
        events.emplace_back(gameplay::StatusTriggered{counteringCombatant.id, statusId("riposte")});
        const auto baseDamage = random_.between(counteringCombatant.attributes.damageMinimum, counteringCombatant.attributes.damageMaximum);
        const auto rawDamage = modifiedDamage(counteringCombatant, attacker, baseDamage);
        const auto armour = effectiveAttributes(attacker).armour;
        const auto armourReduction = static_cast<int>(std::round(armour / 100.0 * rawDamage));
        applyUnavoidableDamage(counteringCombatant, attacker, std::max(0, rawDamage - armourReduction), events);
    }

    void BattleSimulation::resolveEquipmentEffects(std::vector<gameplay::GameplayEvent>& events)
    {
        for (auto& carrier : combatants_)
        {
            if (!carrier.alive() || carrier.team != gameplay::Team::players)
            {
                continue;
            }
            for (const auto& equipped : definition(carrier).equipmentEffects)
            {
                const auto activationKey = carrier.id.value() + ":" + equipped.source.value();
                if (activatedEquipmentEffects_.contains(activationKey))
                {
                    continue;
                }
                const auto* effect = std::get_if<gameplay::LoneVictoryEffect>(&equipped.effect);
                if (effect == nullptr)
                {
                    continue;
                }
                auto otherPartyHealth = 0;
                for (const auto& combatant : combatants_)
                {
                    if (combatant.team == gameplay::Team::players && combatant.id != carrier.id)
                    {
                        otherPartyHealth += combatant.health;
                    }
                }
                if (otherPartyHealth > effect->otherPartyHealthThreshold)
                {
                    continue;
                }

                activatedEquipmentEffects_.insert(activationKey);
                events.emplace_back(gameplay::ItemActivated{equipped.source, carrier.id});
                for (auto& target : combatants_)
                {
                    if (target.id != carrier.id && target.alive())
                    {
                        applyUnavoidableDamage(carrier, target, effect->damage, events);
                    }
                }
            }
        }
    }

    void BattleSimulation::applyUnavoidableDamage(CombatantState& source,
                                                  CombatantState& target,
                                                  int damage,
                                                  std::vector<gameplay::GameplayEvent>& events)
    {
        const auto appliedDamage = std::min(target.health, std::max(0, damage));
        target.health -= appliedDamage;
        updateNofaceBuff(target);
        recordDamageApplied(source.id, target, appliedDamage, gameplay::DamageKind::direct, events);
        emitDefeatIfNeeded(target, events);
    }

    void BattleSimulation::recordDamageApplied(
        const gameplay::EntityId& source, CombatantState& target, int damage, gameplay::DamageKind kind, std::vector<gameplay::GameplayEvent>& events)
    {
        events.emplace_back(gameplay::DamageApplied{source, target.id, damage, kind});
        if (damage > 0 && target.panicStacks > 0)
        {
            changePanic(target, 1, events);
        }
    }

    bool BattleSimulation::attackIsDodged(CombatantState& actor,
                                          CombatantState& target,
                                          const content::AbilityDefinition& ability,
                                          std::vector<gameplay::GameplayEvent>& events)
    {
        if (definition(target).marksAttackersForRevenge)
        {
            actor.fistOfRevengeTarget = true;
        }
        if (target.misses > 0)
        {
            --target.misses;
            events.emplace_back(gameplay::AttackDodged{actor.id, target.id});
            return true;
        }
        const auto targetAttributes = effectiveAttributes(target);
        const auto actorAttributes = effectiveAttributes(actor);
        const auto difference = targetAttributes.dodge - (actorAttributes.precision + ability.precisionModifier);
        if (!random_.chance(std::clamp(difference * 2, 0, 100) / 100.0))
        {
            return false;
        }
        events.emplace_back(gameplay::AttackDodged{actor.id, target.id});
        return true;
    }

    void BattleSimulation::summonCombatants(const content::SummonDefinition& summon, std::vector<gameplay::GameplayEvent>& events)
    {
        for (int count = 0; count < summon.count; ++count)
        {
            const auto emptySlot = std::ranges::find_if(summon.allowedSlots, [this](int slot) {
                return std::ranges::none_of(combatants_, [slot](const auto& combatant) {
                    return combatant.slot == slot && combatant.alive();
                });
            });
            if (emptySlot == summon.allowedSlots.end())
            {
                return;
            }

            const auto& definition = content_.combatants.at(summon.combatant);
            const auto id = gameplay::EntityId{summon.combatant.value() + ".summoned" + std::to_string(nextSummonNumber_++)};
            combatants_.push_back({id,
                                   definition.id,
                                   definition.name,
                                   definition.team,
                                   *emptySlot,
                                   definition.attributes,
                                   definition.resistances,
                                   definition.attributes.maximumHealth});
            events.emplace_back(gameplay::CombatantSummoned{id, definition.id, *emptySlot});
        }
    }

} // namespace soma::simulation
