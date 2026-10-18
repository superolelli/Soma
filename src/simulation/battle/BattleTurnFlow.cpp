#include "simulation/battle/BattleSimulation.hpp"

#include <algorithm>
#include <ranges>

namespace soma::simulation
{
    namespace
    {
        constexpr int wakeSleepResistanceDuration = 3;
        constexpr int wakeSleepResistanceBonus = 40;

        gameplay::StatusId statusId(std::string value)
        {
            return gameplay::StatusId{"status." + std::move(value)};
        }
    } // namespace

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
        if (stage >= 1 && !blocksStage(1))
        {
            desiredStages |= 1;
        }
        if (stage >= 2 && !blocksStage(2))
        {
            desiredStages |= 2;
        }
        if (stage >= 3)
        {
            desiredStages |= 4;
        }

        const auto stageDifference =
            std::popcount(static_cast<unsigned int>(desiredStages)) - std::popcount(static_cast<unsigned int>(target.appliedFatiguePenaltyStages));
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
