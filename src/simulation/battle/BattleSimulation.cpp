#include "simulation/battle/BattleSimulation.hpp"

#include "simulation/battle/AbilityTargets.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <ranges>
#include <stdexcept>

namespace soma::simulation
{
    namespace
    {
        constexpr int wakeSleepResistanceDuration = 3;
        constexpr int wakeSleepResistanceBonus = 40;
        constexpr int slotsPerTeam = 4;

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

    bool CombatantState::alive() const noexcept
    {
        return health > 0;
    }

    BattleSimulation::BattleSimulation(const content::BattleContent& content,
                                       std::vector<BattleSpawn> combatants,
                                       gameplay::IRandom& random,
                                       const content::RuleContent* rules,
                                       const gameplay::ComposedRules* activeRules)
        : content_(content)
        , random_(random)
        , rules_(rules)
        , activeRules_(activeRules)
    {
        for (const auto& spawn : combatants)
        {
            const auto found = content_.combatants.find(spawn.definition);
            if (found == content_.combatants.end())
            {
                throw std::invalid_argument("Unknown combatant: " + spawn.definition.value());
            }
            const auto& definition = found->second;
            auto id = definition.id.value() + ".slot" + std::to_string(spawn.slot);
            combatants_.push_back({gameplay::EntityId{std::move(id)},
                                   definition.id,
                                   definition.name,
                                   definition.team,
                                   spawn.slot,
                                   definition.attributes,
                                   definition.resistances,
                                   0});
            for (const auto& buff : spawn.initialBuffs)
            {
                addAttributes(combatants_.back(), buff.attributes, 1);
                combatants_.back().buffs.push_back(buff);
            }
            combatants_.back().fatigue = std::clamp(spawn.initialFatigue, 0, gameplay::maximumFatigue);
            updateFatiguePenalties(combatants_.back());
            combatants_.back().health =
                std::clamp(spawn.initialHealth.value_or(combatants_.back().attributes.maximumHealth), 0, combatants_.back().attributes.maximumHealth);
            if (definition.id == gameplay::ContentDefinitionId{"enemy.bill_noface"})
            {
                combatants_.back().nofaceBuffLevel = 0;
            }
        }
        rebuildTurnOrder();
        if (turnOrder_.empty())
        {
            throw std::invalid_argument("A battle requires combatants");
        }
    }

    gameplay::Result<std::vector<gameplay::GameplayEvent>> BattleSimulation::execute(const BattleActionCommand& command)
    {
        if (finished_)
        {
            return gameplay::Result<std::vector<gameplay::GameplayEvent>>::failure("The battle is finished");
        }
        if (command.actor != currentActor())
        {
            return gameplay::Result<std::vector<gameplay::GameplayEvent>>::failure("The actor does not have the current turn");
        }

        auto* actor = findMutable(command.actor);
        auto* primaryTarget = findMutable(command.primaryTarget);
        const auto abilityFound = content_.abilities.find(command.ability);
        if (actor == nullptr || primaryTarget == nullptr || !primaryTarget->alive() || abilityFound == content_.abilities.end())
        {
            return gameplay::Result<std::vector<gameplay::GameplayEvent>>::failure("Unknown actor, ability, or target");
        }
        const auto& ability = abilityFound->second;
        const auto& ownedAbilities = definition(*actor).abilities;
        if (std::ranges::find(ownedAbilities, command.ability) == ownedAbilities.end())
        {
            return gameplay::Result<std::vector<gameplay::GameplayEvent>>::failure("The actor does not own this ability");
        }

        const auto validTargets = validPrimaryTargets(command.ability);
        if (std::ranges::find(validTargets, command.primaryTarget) == validTargets.end())
        {
            return gameplay::Result<std::vector<gameplay::GameplayEvent>>::failure("The selected target is not valid");
        }

        std::vector<gameplay::GameplayEvent> events;
        if (actor->confusedThisTurn)
        {
            // Confusion inverts the side selected by the player. Hostile abilities
            // are redirected to another ally, while friendly/self abilities are
            // redirected to an enemy. The actor must never hit themselves.
            std::vector<CombatantState*> redirectedTargets;
            for (auto& candidate : combatants_)
            {
                if (candidate.alive() && candidate.team != primaryTarget->team && candidate.id != actor->id)
                {
                    redirectedTargets.push_back(&candidate);
                }
            }
            if (redirectedTargets.empty())
            {
                advanceTurn(events);
                return gameplay::Result<std::vector<gameplay::GameplayEvent>>::success(std::move(events));
            }
            const auto originalTarget = primaryTarget->id;
            primaryTarget = redirectedTargets.at(static_cast<std::size_t>(random_.between(0, static_cast<int>(redirectedTargets.size()) - 1)));
            events.emplace_back(gameplay::TargetRedirected{actor->id, originalTarget, primaryTarget->id});
        }
        else if (ability.targeting.scope == content::TargetScope::randomSingle)
        {
            const auto originalTarget = primaryTarget->id;
            primaryTarget = findMutable(validTargets.at(static_cast<std::size_t>(random_.between(0, static_cast<int>(validTargets.size()) - 1))));
            events.emplace_back(gameplay::TargetRedirected{actor->id, originalTarget, primaryTarget->id});
        }

        auto targets = actor->confusedThisTurn ? std::vector<CombatantState*>{primaryTarget} : expandTargets(*actor, ability, *primaryTarget);
        events.emplace_back(gameplay::AbilityUsed{actor->id, ability.id});
        if (actor->team == gameplay::Team::players)
        {
            usedPlayerAbilities_.insert(ability.id);
        }
        applyAbilityFatigue(*actor, ability, targets, events);

        applyAbilityToTargets(*actor, ability, targets, events);

        if (ability.summon.has_value())
        {
            summonCombatants(*ability.summon, events);
        }

        resolveTriggeredRules(events, actor->id, 0);
        const auto firstEquipmentEvent = events.size();
        resolveEquipmentEffects(events);
        resolveTriggeredRules(events, actor->id, firstEquipmentEvent);
        updateFinished(events);
        if (!finished_)
        {
            advanceToPlayableTurn(events);
        }
        return gameplay::Result<std::vector<gameplay::GameplayEvent>>::success(std::move(events));
    }

    gameplay::Result<std::vector<gameplay::GameplayEvent>> BattleSimulation::passTurn(const gameplay::EntityId& actor)
    {
        if (finished_)
        {
            return gameplay::Result<std::vector<gameplay::GameplayEvent>>::failure("The battle is finished");
        }
        if (actor != currentActor())
        {
            return gameplay::Result<std::vector<gameplay::GameplayEvent>>::failure("The actor does not have the current turn");
        }

        auto* state = findMutable(actor);
        if (state == nullptr)
        {
            return gameplay::Result<std::vector<gameplay::GameplayEvent>>::failure("Unknown actor");
        }

        std::vector<gameplay::GameplayEvent> events;
        if (definition(*state).healsOnPass)
        {
            changeFatigue(*state, -3, events);
            applyHealing(state->id, *state, 3, events);
        }
        resolveTriggeredRules(events, state->id, 0);
        const auto firstEquipmentEvent = events.size();
        resolveEquipmentEffects(events);
        resolveTriggeredRules(events, state->id, firstEquipmentEvent);
        updateFinished(events);
        if (finished_)
        {
            return gameplay::Result<std::vector<gameplay::GameplayEvent>>::success(std::move(events));
        }
        advanceToPlayableTurn(events);
        return gameplay::Result<std::vector<gameplay::GameplayEvent>>::success(std::move(events));
    }

    gameplay::Result<std::vector<gameplay::GameplayEvent>> BattleSimulation::useConsumable(const gameplay::EntityId& targetId,
                                                                                           const gameplay::ConsumableItem& consumable)
    {
        auto* target = findMutable(targetId);
        if (finished_ || target == nullptr || !target->alive() || target->team != gameplay::Team::players)
        {
            return gameplay::Result<std::vector<gameplay::GameplayEvent>>::failure("The consumable target is not valid");
        }
        std::vector<gameplay::GameplayEvent> events;
        if (consumable.healing > 0)
        {
            applyHealing(target->id, *target, consumable.healing, events);
        }
        if (consumable.duration > 0)
        {
            content::BuffDefinition buff;
            buff.duration = consumable.duration;
            buff.positive = true;
            buff.attributes.armour = consumable.temporaryBonuses.armour;
            buff.attributes.maximumHealth = consumable.temporaryBonuses.maximumHealth;
            buff.attributes.damageMinimum = consumable.temporaryBonuses.damageMinimum;
            buff.attributes.damageMaximum = consumable.temporaryBonuses.damageMaximum;
            buff.attributes.initiative = consumable.temporaryBonuses.initiative;
            buff.attributes.criticalHitChance = consumable.temporaryBonuses.criticalHitChance;
            buff.attributes.dodge = consumable.temporaryBonuses.dodge;
            buff.attributes.precision = consumable.temporaryBonuses.precision;
            buff.attributes.healing = consumable.temporaryBonuses.healing;
            buff.attributes.sleepResistance = consumable.temporaryBonuses.sleepResistance;
            buff.attributes.confusionResistance = consumable.temporaryBonuses.confusionResistance;
            buff.attributes.debuffResistance = consumable.temporaryBonuses.debuffResistance;
            buff.attributes.decayResistance = consumable.temporaryBonuses.decayResistance;
            applyBuff(*target, buff, buff.duration, events);
        }
        resolveTriggeredRules(events, target->id, 0);
        const auto firstEquipmentEvent = events.size();
        resolveEquipmentEffects(events);
        resolveTriggeredRules(events, target->id, firstEquipmentEvent);
        updateFinished(events);
        return gameplay::Result<std::vector<gameplay::GameplayEvent>>::success(std::move(events));
    }

    std::vector<gameplay::EntityId> BattleSimulation::validPrimaryTargets(const gameplay::AbilityId& abilityId) const
    {
        std::vector<gameplay::EntityId> targets;
        if (finished_)
        {
            return targets;
        }
        const auto found = content_.abilities.find(abilityId);
        const auto* actor = find(currentActor());
        if (found == content_.abilities.end() || actor == nullptr)
        {
            return targets;
        }
        const auto& targeting = found->second.targeting;
        for (const auto& candidate : combatants_)
        {
            if (isValidPrimaryTarget(targeting, *actor, candidate)) targets.push_back(candidate.id);
        }
        return targets;
    }

    const CombatantState* BattleSimulation::find(const gameplay::EntityId& id) const
    {
        const auto found = std::ranges::find(combatants_, id, &CombatantState::id);
        return found == combatants_.end() ? nullptr : &*found;
    }

    const std::vector<CombatantState>& BattleSimulation::combatants() const noexcept
    {
        return combatants_;
    }

    const gameplay::EntityId& BattleSimulation::currentActor() const
    {
        if (finished_)
        {
            throw std::logic_error("A finished battle has no actor");
        }
        return turnOrder_.at(turnIndex_);
    }

    bool BattleSimulation::finished() const noexcept
    {
        return finished_;
    }

    std::optional<gameplay::Team> BattleSimulation::winner() const noexcept
    {
        return winner_;
    }

    const content::BattleContent& BattleSimulation::content() const noexcept
    {
        return content_;
    }

    bool BattleSimulation::enemyDiedLastRound() const noexcept
    {
        return roundsSinceEnemyDefeat_ >= 0 && roundsSinceEnemyDefeat_ <= 1;
    }

    const std::unordered_set<gameplay::AbilityId>& BattleSimulation::usedPlayerAbilities() const noexcept
    {
        return usedPlayerAbilities_;
    }

    CombatantState* BattleSimulation::findMutable(const gameplay::EntityId& id)
    {
        const auto found = std::ranges::find(combatants_, id, &CombatantState::id);
        return found == combatants_.end() ? nullptr : &*found;
    }

    const content::CombatantDefinition& BattleSimulation::definition(const CombatantState& combatant) const
    {
        return content_.combatants.at(combatant.definition);
    }

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
            if (auto* target = findMutable(id)) result.push_back(target);
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

    void BattleSimulation::resolveRiposte(CombatantState& counteringCombatant,
                                          CombatantState& attacker,
                                          std::vector<gameplay::GameplayEvent>& events)
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

    void BattleSimulation::recordDamageApplied(const gameplay::EntityId& source,
                                                CombatantState& target,
                                                int damage,
                                                gameplay::DamageKind kind,
                                                std::vector<gameplay::GameplayEvent>& events)
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
            emitDefeatIfNeeded(target, events);
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
        const auto applyResistedStatus = [this, &target, &events](double probability,
                                                                  const std::string& kind,
                                                                  const gameplay::StatusId& status,
                                                                  auto apply) {
            if (!random_.chance(std::min(probability, 1.0)))
            {
                return false;
            }
            if (random_.between(1, 100) <= resistance(target, kind))
            {
                events.emplace_back(gameplay::EffectResisted{target.id, status});
                return false;
            }
            apply();
            return true;
        };

        if (effect.confusionRounds > 0)
        {
            applyResistedStatus(effect.confusionProbability, "confusion", statusId("confused"), [&] {
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
            const auto sleepApplied = applyResistedStatus(sleepProbability, "sleep", statusId("sleeping"), [&] {
                target.sleeping = true;
                recordStatusGained(target, statusId("sleeping"), events);
            });
            if (!sleepApplied && effect.sleepFailureDecayDamage > 0 && effect.sleepFailureDecayRounds > 0)
            {
                static_cast<void>(tryApplyDecay(target,
                                                effect.sleepFailureDecayDamage,
                                                effect.sleepFailureDecayRounds + durationBonus,
                                                effect.ignoreDecayResistance,
                                                events));
            }
        }
        if (effect.decayRounds > 0)
        {
            const auto conditionalRounds = (target.confusedTurns > 0 ? effect.decayRoundsAgainstConfused : 0) +
                                           (target.sleeping ? effect.decayRoundsAgainstSleeping : 0);
            const auto conditionalDamage = target.markedTurns > 0 ? effect.decayDamageAgainstMarked : 0;
            const auto duration = std::max(0, effect.decayRounds + conditionalRounds + durationBonus);
            static_cast<void>(
                tryApplyDecay(target, effect.decayDamage + conditionalDamage, duration, effect.ignoreDecayResistance, events));
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
            applyResistedStatus(1.0, "debuff", statusId("hacked"), [&] {
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

    bool BattleSimulation::tryApplyDecay(CombatantState& target,
                                         int damage,
                                         int duration,
                                         bool ignoreResistance,
                                         std::vector<gameplay::GameplayEvent>& events)
    {
        damage = std::max(0, damage);
        duration = std::max(0, duration);
        if (damage == 0 || duration == 0)
        {
            return false;
        }
        if (!ignoreResistance && random_.between(1, 100) <= resistance(target, "decay"))
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
        const auto resisted = [this, &target, &events](const std::string& kind, const gameplay::StatusId& status) {
            if (random_.between(1, 100) > resistance(target, kind))
            {
                return false;
            }
            events.emplace_back(gameplay::EffectResisted{target.id, status});
            return true;
        };

        switch (random_.between(0, 6))
        {
        case 0: {
            if (resisted("debuff", statusId("debuffed")))
            {
                return;
            }
            content::BuffDefinition debuff;
            debuff.duration = duration;
            debuff.positive = false;
            const auto percentage = random_.between(10, 20) / 100.0;
            const auto reduction = [percentage](int value) { return -std::max(1, static_cast<int>(std::round(std::abs(value) * percentage))); };
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
            if (!resisted("sleep", statusId("sleeping")))
            {
                target.sleeping = true;
                recordStatusGained(target, statusId("sleeping"), events);
            }
            break;
        case 2:
            if (!resisted("confusion", statusId("confused")))
            {
                target.confusedTurns += duration;
                recordStatusGained(target, statusId("confused"), events);
            }
            break;
        case 3:
            static_cast<void>(tryApplyDecay(target, random_.between(1, 2), duration, false, events));
            break;
        case 4:
            if (!resisted("debuff", statusId("bounty")))
            {
                target.bountyTurns += duration;
                recordStatusGained(target, statusId("bounty"), events);
            }
            break;
        case 5:
            if (!resisted("debuff", statusId("marked")))
            {
                target.markedTurns += duration;
                recordStatusGained(target, statusId("marked"), events);
            }
            break;
        default:
            if (!resisted("debuff", statusId("hacked")))
            {
                target.hackedTurns += duration;
                recordStatusGained(target, statusId("hacked"), events);
            }
            break;
        }
    }

    void BattleSimulation::prolongNegativeEffects(CombatantState& target,
                                                   int rounds,
                                                   std::vector<gameplay::GameplayEvent>& events)
    {
        rounds = std::max(0, rounds);
        const auto hasNegativeEffect = !target.debuffs.empty() || !target.decay.empty() || target.confusedTurns > 0 ||
                                       target.markedTurns > 0 || target.hackedTurns > 0 || target.bountyTurns > 0;
        for (auto& debuff : target.debuffs)
        {
            debuff.remainingTurns += rounds;
        }
        for (auto& decay : target.decay)
        {
            decay.remainingTurns += rounds;
        }
        if (target.confusedTurns > 0) target.confusedTurns += rounds;
        if (target.markedTurns > 0) target.markedTurns += rounds;
        if (target.hackedTurns > 0) target.hackedTurns += rounds;
        if (target.bountyTurns > 0) target.bountyTurns += rounds;
        if (hasNegativeEffect && rounds > 0)
        {
            events.emplace_back(gameplay::NegativeEffectsProlonged{target.id, rounds});
        }
    }

    void BattleSimulation::inflictRemainingDecay(CombatantState& actor,
                                                  CombatantState& target,
                                                  std::vector<gameplay::GameplayEvent>& events)
    {
        auto damage = 0;
        for (const auto& decay : target.decay)
        {
            damage += std::max(0, decay.damage) * std::max(0, decay.remainingTurns);
        }
        if (damage == 0)
        {
            return;
        }
        target.decay.clear();
        events.emplace_back(gameplay::DecayDetonated{target.id});
        events.emplace_back(gameplay::StatusTriggered{target.id, statusId("decay")});
        events.emplace_back(gameplay::StatusExpired{target.id, statusId("decay")});
        damage = std::max(0, standingModifiedValue(content::ModifierKind::damageReceived, target, damage));
        if (target.rageTurns > 0)
        {
            damage = static_cast<int>(std::round(damage * 1.5));
        }
        target.health = std::max(0, target.health - damage);
        updateNofaceBuff(target);
        recordDamageApplied(actor.id, target, damage, gameplay::DamageKind::decay, events);
        triggerTransfusions(target, events);
        expireTransfusionsWhenDecayEnds(target, events);
        emitDefeatIfNeeded(target, events);
    }

    void BattleSimulation::transferDecayAfterDefeat(CombatantState& actor,
                                                     CombatantState& defeated,
                                                     std::vector<gameplay::GameplayEvent>& events)
    {
        if (defeated.decay.empty())
        {
            return;
        }
        std::vector<CombatantState*> candidates;
        for (auto& candidate : combatants_)
        {
            if (candidate.alive() && candidate.team != actor.team && candidate.id != defeated.id)
            {
                candidates.push_back(&candidate);
            }
        }
        if (candidates.empty())
        {
            return;
        }
        auto& recipient = *candidates.at(static_cast<std::size_t>(random_.between(0, static_cast<int>(candidates.size()) - 1)));
        recipient.decay.insert(recipient.decay.end(), defeated.decay.begin(), defeated.decay.end());
        defeated.decay.clear();
        events.emplace_back(gameplay::DecayTransferred{defeated.id, recipient.id});
        events.emplace_back(gameplay::StatusExpired{defeated.id, statusId("decay")});
        expireTransfusionsWhenDecayEnds(defeated, events);
        recordStatusGained(recipient, statusId("decay"), events);
    }

    void BattleSimulation::triggerTransfusions(CombatantState& source, std::vector<gameplay::GameplayEvent>& events)
    {
        if (source.transfusions.empty())
        {
            return;
        }
        for (const auto& transfusion : source.transfusions)
        {
            std::vector<CombatantState*> candidates;
            for (auto& candidate : combatants_)
            {
                if (candidate.alive() && candidate.team != source.team)
                {
                    candidates.push_back(&candidate);
                }
            }
            if (candidates.empty())
            {
                continue;
            }
            auto& target = *candidates.at(static_cast<std::size_t>(random_.between(0, static_cast<int>(candidates.size()) - 1)));
            events.emplace_back(gameplay::StatusTriggered{source.id, statusId("transfusion")});
            applyHealing(source.id, target, transfusion.healing, events);
        }
    }

    void BattleSimulation::expireTransfusionsWhenDecayEnds(CombatantState& target, std::vector<gameplay::GameplayEvent>& events)
    {
        if (target.transfusions.empty() || !target.decay.empty())
        {
            return;
        }
        target.transfusions.clear();
        events.emplace_back(gameplay::StatusExpired{target.id, statusId("transfusion")});
    }

    void BattleSimulation::removeDecayAndHealLivingPlayers(CombatantState& actor,
                                                            CombatantState& target,
                                                            std::vector<gameplay::GameplayEvent>& events)
    {
        auto outstandingDamage = 0;
        for (const auto& decay : target.decay)
        {
            outstandingDamage += std::max(0, decay.damage) * std::max(0, decay.remainingTurns);
        }
        if (outstandingDamage == 0)
        {
            return;
        }

        target.decay.clear();
        events.emplace_back(gameplay::StatusExpired{target.id, statusId("decay")});
        expireTransfusionsWhenDecayEnds(target, events);

        std::vector<CombatantState*> players;
        for (auto& combatant : combatants_)
        {
            if (combatant.alive() && combatant.team == gameplay::Team::players)
            {
                players.push_back(&combatant);
            }
        }
        if (players.empty())
        {
            return;
        }

        const auto healingPerPlayer = outstandingDamage / static_cast<int>(players.size());
        auto remainder = outstandingDamage % static_cast<int>(players.size());
        std::vector<CombatantState*> remainderCandidates = players;
        std::unordered_set<gameplay::EntityId> remainderRecipients;
        while (remainder-- > 0)
        {
            const auto index = static_cast<std::size_t>(random_.between(0, static_cast<int>(remainderCandidates.size()) - 1));
            remainderRecipients.insert(remainderCandidates.at(index)->id);
            remainderCandidates.erase(remainderCandidates.begin() + static_cast<std::ptrdiff_t>(index));
        }
        for (auto* player : players)
        {
            const auto remainderHealing = remainderRecipients.contains(player->id) ? 1 : 0;
            const auto healing = healingPerPlayer + remainderHealing;
            if (healing > 0)
            {
                applyHealing(actor.id, *player, healing, events);
            }
        }
    }

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
        const auto applyEffectBuff = [this, &actor, &target, durationBonus, &events](const content::BuffDefinition& buff,
                                                                                    bool targetsActor) {
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

    void BattleSimulation::applyBuff(CombatantState& target,
                                     const content::BuffDefinition& buff,
                                     int duration,
                                     std::vector<gameplay::GameplayEvent>& events,
                                     bool ignoresHacked)
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
                   active.damageAgainstMarked == buff.damageAgainstMarked &&
                   active.damagePerNegativeStatus == buff.damagePerNegativeStatus && active.damageFromFatigue == buff.damageFromFatigue &&
                   active.blocksFatigueStageOne == buff.blocksFatigueStageOne &&
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

    void BattleSimulation::recordStatusGained(CombatantState& target,
                                               const gameplay::StatusId& status,
                                               std::vector<gameplay::GameplayEvent>& events)
    {
        events.emplace_back(gameplay::StatusGained{target.id, status});
        if (target.panicStacks == 0 || status == statusId("panic"))
        {
            return;
        }
        const auto& value = status.value();
        const auto negative = value == "status.debuffed" || value == "status.sleeping" || value == "status.confused" ||
                              value == "status.decay" || value == "status.bounty" || value == "status.marked" ||
                              value == "status.hacked" || value == "status.transfusion";
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
        return static_cast<int>(!target.debuffs.empty()) + static_cast<int>(target.sleeping) +
               static_cast<int>(target.confusedTurns > 0) + static_cast<int>(!target.decay.empty()) +
               static_cast<int>(target.bountyTurns > 0) + static_cast<int>(target.markedTurns > 0) +
               static_cast<int>(target.hackedTurns > 0) + static_cast<int>(!target.transfusions.empty()) +
               static_cast<int>(target.panicStacks > 0);
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

    void BattleSimulation::updateStatuses(CombatantState& combatant, std::vector<gameplay::GameplayEvent>& events)
    {
        if (combatant.panicStacks > 0)
        {
            changePanic(combatant, -1, events);
        }
        const auto expire = [this, &combatant, &events](std::vector<ActiveBuff>& buffs, const gameplay::StatusId& status) {
            auto expired = false;
            std::erase_if(buffs, [this, &combatant, &expired](auto& buff) {
                --buff.remainingTurns;
                if (buff.remainingTurns > 0)
                {
                    return false;
                }
                addAttributes(combatant, buff.attributes, -1);
                expired = true;
                return true;
            });
            if (expired)
            {
                events.emplace_back(gameplay::StatusExpired{combatant.id, status});
            }
        };
        expire(combatant.buffs, statusId("buffed"));
        expire(combatant.debuffs, statusId("debuffed"));
        combatant.health = std::min(combatant.health, combatant.attributes.maximumHealth);

        if (combatant.markedTurns > 0)
        {
            --combatant.markedTurns;
        }
        if (combatant.confusedTurns > 0)
        {
            --combatant.confusedTurns;
        }
        if (combatant.bountyTurns > 0)
        {
            --combatant.bountyTurns;
        }
        if (combatant.riposteTurns > 0 && --combatant.riposteTurns == 0)
        {
            events.emplace_back(gameplay::StatusExpired{combatant.id, statusId("riposte")});
        }
        if (combatant.rageTurns > 0 && --combatant.rageTurns == 0)
        {
            events.emplace_back(gameplay::StatusExpired{combatant.id, statusId("rage")});
        }
        if (combatant.hackedTurns > 0 && --combatant.hackedTurns == 0)
        {
            events.emplace_back(gameplay::StatusExpired{combatant.id, statusId("hacked")});
        }

        auto decayDamage = 0;
        std::erase_if(combatant.decay, [&decayDamage](auto& decay) {
            decayDamage += decay.damage;
            --decay.remainingTurns;
            return decay.remainingTurns <= 0;
        });
        if (decayDamage > 0)
        {
            events.emplace_back(gameplay::StatusTriggered{combatant.id, statusId("decay")});
            decayDamage = std::max(0, standingModifiedValue(content::ModifierKind::damageReceived, combatant, decayDamage));
            if (combatant.rageTurns > 0)
            {
                decayDamage = static_cast<int>(std::round(decayDamage * 1.5));
            }
            combatant.health = std::max(0, combatant.health - decayDamage);
            updateNofaceBuff(combatant);
            recordDamageApplied(combatant.id, combatant, decayDamage, gameplay::DamageKind::decay, events);
            triggerTransfusions(combatant, events);
            expireTransfusionsWhenDecayEnds(combatant, events);
            emitDefeatIfNeeded(combatant, events);
        }

        if (combatant.dynamite)
        {
            combatant.dynamite = false;
            if (random_.between(0, 4) == 0)
            {
                events.emplace_back(gameplay::StatusTriggered{combatant.id, statusId("dynamite")});
                const auto rawDamage = standingModifiedValue(content::ModifierKind::damageReceived, combatant, 15);
                const auto rageDamage = combatant.rageTurns > 0 ? static_cast<int>(std::round(rawDamage * 1.5)) : rawDamage;
                const auto effectiveArmour = effectiveAttributes(combatant).armour;
                const auto armour = static_cast<int>(std::round(effectiveArmour / 100.0 * rageDamage));
                const auto damage = std::max(0, rageDamage - armour);
                combatant.health = std::max(0, combatant.health - damage);
                updateNofaceBuff(combatant);
                recordDamageApplied(combatant.id, combatant, damage, gameplay::DamageKind::direct, events);
                emitDefeatIfNeeded(combatant, events);
            }
            else
            {
                auto next = std::ranges::find_if(combatants_, [&combatant](const auto& candidate) {
                    return candidate.alive() && candidate.slot > combatant.slot;
                });
                if (next == combatants_.end())
                {
                    next = std::ranges::min_element(combatants_, {}, [](const auto& candidate) {
                        return candidate.alive() ? candidate.slot : 100;
                    });
                }
                if (next != combatants_.end())
                {
                    next->dynamite = true;
                    recordStatusGained(*next, statusId("dynamite"), events);
                }
            }
        }
    }

    std::vector<gameplay::EntityId> BattleSimulation::initiativeOrder() const
    {
        std::vector<gameplay::EntityId> order;
        for (const auto& combatant : combatants_)
        {
            if (combatant.alive())
            {
                order.push_back(combatant.id);
            }
        }
        std::ranges::stable_sort(order, [this](const auto& left, const auto& right) {
            return effectiveAttributes(*find(left)).initiative > effectiveAttributes(*find(right)).initiative;
        });
        return order;
    }

    std::vector<gameplay::EntityId> BattleSimulation::remainingInitiativeOrder() const
    {
        auto order = initiativeOrder();
        std::erase_if(order, [this](const auto& combatant) {
            return actedThisRound_.contains(combatant);
        });
        return order;
    }

    void BattleSimulation::rebuildTurnOrder()
    {
        turnOrder_ = initiativeOrder();
    }

    void BattleSimulation::advanceTurn(std::vector<gameplay::GameplayEvent>& events)
    {
        while (true)
        {
            actedThisRound_.insert(currentActor());
            turnOrder_ = remainingInitiativeOrder();
            if (turnOrder_.empty())
            {
                actedThisRound_.clear();
                rebuildTurnOrder();
                if (roundsSinceEnemyDefeat_ >= 0)
                {
                    ++roundsSinceEnemyDefeat_;
                }
            }
            turnIndex_ = 0;

            auto* actor = findMutable(turnOrder_.at(turnIndex_));
            if (!actor->alive())
            {
                continue;
            }

            actor->confusedThisTurn = false;
            actor->fistOfRevengeTarget = false;
            events.emplace_back(gameplay::TurnStarted{actor->id});
            updateStatuses(*actor, events);
            resolveEquipmentEffects(events);
            updateFinished(events);
            if (finished_)
            {
                return;
            }
            if (!actor->alive())
            {
                continue;
            }
            if (actor->sleeping)
            {
                actor->sleeping = false;
                events.emplace_back(gameplay::StatusExpired{actor->id, statusId("sleeping")});
                content::BuffDefinition wakeResistance;
                wakeResistance.duration = wakeSleepResistanceDuration;
                wakeResistance.positive = true;
                wakeResistance.attributes.sleepResistance = wakeSleepResistanceBonus;
                applyBuff(*actor, wakeResistance, wakeResistance.duration, events, true);
                continue;
            }
            if (actor->confusedTurns > 0 && random_.between(0, 9) < 5)
            {
                actor->confusedThisTurn = true;
                events.emplace_back(gameplay::StatusGained{actor->id, statusId("confused_action")});
            }
            return;
        }
    }

    void BattleSimulation::updateFinished(std::vector<gameplay::GameplayEvent>& events)
    {
        const auto playersAlive = std::ranges::any_of(combatants_, [](const auto& combatant) {
            return combatant.team == gameplay::Team::players && combatant.alive();
        });
        const auto enemiesAlive = std::ranges::any_of(combatants_, [](const auto& combatant) {
            return combatant.team == gameplay::Team::enemies && combatant.alive();
        });
        if (playersAlive && enemiesAlive)
        {
            return;
        }
        if (!finished_)
        {
            finished_ = true;
            winner_ = playersAlive ? gameplay::Team::players : gameplay::Team::enemies;
            events.emplace_back(gameplay::BattleFinished{*winner_});
        }
    }

    void BattleSimulation::emitDefeatIfNeeded(CombatantState& target, std::vector<gameplay::GameplayEvent>& events)
    {
        if (!target.alive())
        {
            events.emplace_back(gameplay::CombatantDefeated{target.id});
            if (target.team == gameplay::Team::enemies)
            {
                roundsSinceEnemyDefeat_ = 0;
            }
        }
    }

    void BattleSimulation::updateFatiguePenalties(CombatantState& target)
    {
        const auto stage = gameplay::fatigueStage(target.fatigue);
        const auto blocksStage = [&target](int blockedStage) {
            return std::ranges::any_of(target.buffs, [blockedStage](const ActiveBuff& buff) {
                return blockedStage == 1 ? buff.blocksFatigueStageOne : buff.blocksFatigueStageTwo;
            });
        };
        auto desiredStages = 0;
        if (stage >= 1 && !blocksStage(1)) desiredStages |= 1;
        if (stage >= 2 && !blocksStage(2)) desiredStages |= 2;
        if (stage >= 3) desiredStages |= 4;

        const auto stageDifference = std::popcount(static_cast<unsigned int>(desiredStages)) -
                                     std::popcount(static_cast<unsigned int>(target.appliedFatiguePenaltyStages));
        if (stageDifference == 0)
        {
            target.appliedFatiguePenaltyStages = desiredStages;
            return;
        }
        content::AttributeDelta fatiguePenalty;
        fatiguePenalty.damageMinimum = -3;
        fatiguePenalty.damageMaximum = -3;
        fatiguePenalty.dodge = -5;
        fatiguePenalty.precision = -5;
        fatiguePenalty.initiative = -1;
        addAttributes(target, fatiguePenalty, stageDifference);
        target.appliedFatiguePenaltyStages = desiredStages;
    }

    void BattleSimulation::updateNofaceBuff(CombatantState& target)
    {
        if (target.nofaceBuffLevel < 0)
        {
            return;
        }
        const auto newLevel = (target.attributes.maximumHealth - target.health) / 5;
        const auto difference = newLevel - target.nofaceBuffLevel;
        target.attributes.damageMinimum += difference * 2;
        target.attributes.damageMaximum += difference * 2;
        target.attributes.criticalHitChance += difference;
        target.attributes.precision += difference * 2;
        target.attributes.initiative += difference;
        target.nofaceBuffLevel = newLevel;
    }

    void BattleSimulation::changeFatigue(CombatantState& target, int amount, std::vector<gameplay::GameplayEvent>& events)
    {
        const auto previous = target.fatigue;
        const auto previousStage = gameplay::fatigueStage(previous);
        target.fatigue = std::clamp(previous + amount, 0, gameplay::maximumFatigue);
        const auto applied = target.fatigue - previous;
        if (applied == 0)
        {
            return;
        }

        const auto currentStage = gameplay::fatigueStage(target.fatigue);
        updateFatiguePenalties(target);
        events.emplace_back(gameplay::FatigueChanged{target.id, applied, previous, target.fatigue});
        if (currentStage != previousStage)
        {
            if (previousStage > 0)
            {
                events.emplace_back(gameplay::StatusExpired{target.id, gameplay::StatusId{"status.fatigue." + std::to_string(previousStage)}});
            }
            if (currentStage > 0)
            {
                events.emplace_back(gameplay::StatusGained{target.id, gameplay::StatusId{"status.fatigue." + std::to_string(currentStage)}});
            }
        }
    }

    void BattleSimulation::applyAbilityFatigue(CombatantState& actor,
                                               const content::AbilityDefinition& ability,
                                               const std::vector<CombatantState*>& targets,
                                               std::vector<gameplay::GameplayEvent>& events)
    {
        if (ability.fatigue == 0)
        {
            return;
        }
        if (ability.fatigueTarget == content::FatigueTarget::actor)
        {
            changeFatigue(actor, ability.fatigue, events);
            return;
        }

        std::unordered_set<gameplay::EntityId> affected;
        if (ability.fatigueTarget == content::FatigueTarget::targets)
        {
            for (auto* target : targets)
            {
                if (affected.emplace(target->id).second)
                {
                    changeFatigue(*target, ability.fatigue, events);
                }
            }
            return;
        }

        if (ability.fatigueTarget == content::FatigueTarget::randomPlayer)
        {
            std::vector<CombatantState*> players;
            for (auto& target : combatants_)
            {
                if (target.alive() && target.team == gameplay::Team::players)
                {
                    players.push_back(&target);
                }
            }
            if (!players.empty())
            {
                auto* target = players.at(static_cast<std::size_t>(random_.between(0, static_cast<int>(players.size()) - 1)));
                changeFatigue(*target, ability.fatigue, events);
            }
            return;
        }

        for (auto& target : combatants_)
        {
            if (target.alive() && target.team == gameplay::Team::players)
            {
                changeFatigue(target, ability.fatigue, events);
            }
        }
    }
} // namespace soma::simulation
