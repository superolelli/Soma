#include "simulation/battle/BattleSimulation.hpp"

#include "simulation/battle/AbilityTargets.hpp"

#include <algorithm>
#include <cmath>
#include <ranges>
#include <stdexcept>

namespace soma::simulation
{
    namespace
    {
        int scaled(int value, double multiplier)
        {
            return static_cast<int>(std::round(static_cast<double>(value) * multiplier));
        }

        gameplay::CombatantAttributes scaled(gameplay::CombatantAttributes value, double multiplier)
        {
            value.maximumHealth = scaled(value.maximumHealth, multiplier);
            value.damageMinimum = scaled(value.damageMinimum, multiplier);
            value.damageMaximum = scaled(value.damageMaximum, multiplier);
            value.armour = scaled(value.armour, multiplier);
            value.criticalHitChance = scaled(value.criticalHitChance, multiplier);
            value.dodge = scaled(value.dodge, multiplier);
            value.initiative = scaled(value.initiative, multiplier);
            value.precision = scaled(value.precision, multiplier);
            return value;
        }

        content::AttributeDelta scaled(content::AttributeDelta value, double multiplier)
        {
            value.armour = scaled(value.armour, multiplier);
            value.damageMinimum = scaled(value.damageMinimum, multiplier);
            value.damageMaximum = scaled(value.damageMaximum, multiplier);
            value.initiative = scaled(value.initiative, multiplier);
            value.maximumHealth = scaled(value.maximumHealth, multiplier);
            value.criticalHitChance = scaled(value.criticalHitChance, multiplier);
            value.precision = scaled(value.precision, multiplier);
            value.dodge = scaled(value.dodge, multiplier);
            value.healing = scaled(value.healing, multiplier);
            value.sleepResistance = scaled(value.sleepResistance, multiplier);
            value.confusionResistance = scaled(value.confusionResistance, multiplier);
            value.debuffResistance = scaled(value.debuffResistance, multiplier);
            value.decayResistance = scaled(value.decayResistance, multiplier);
            return value;
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
            const auto attributes = scaled(definition.attributes, spawn.attributeMultiplier);
            combatants_.push_back({gameplay::EntityId{std::move(id)},
                                   definition.id,
                                   definition.name,
                                   definition.team,
                                   spawn.slot,
                                   attributes,
                                   scaled(definition.resistances, spawn.attributeMultiplier),
                                   0});
            combatants_.back().baseAttributes = attributes;
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
            if (isValidPrimaryTarget(targeting, *actor, candidate))
            {
                targets.push_back(candidate.id);
            }
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

} // namespace soma::simulation
