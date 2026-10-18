#include "simulation/battle/BattleSimulation.hpp"
#include "simulation/rules/ModifierPipeline.hpp"
#include "simulation/rules/RuleEventQueue.hpp"

#include <algorithm>
#include <cmath>

namespace soma::simulation
{
    namespace
    {
        gameplay::RuleId ruleId(std::size_t slot)
        {
            return gameplay::RuleId{"active-rule." + std::to_string(slot + 1)};
        }

        bool statusIs(const gameplay::GameplayEvent& event, std::string_view id)
        {
            const auto* status = std::get_if<gameplay::StatusGained>(&event);
            return status != nullptr && status->status == gameplay::StatusId{std::string{id}};
        }

        bool isCompleteTriggeredRule(const gameplay::ComposedRule& rule)
        {
            return rule.trigger.has_value() && rule.subject.has_value() && rule.effect.has_value() && !rule.modifier.has_value();
        }

        bool isCompleteStandingRule(const gameplay::ComposedRule& rule)
        {
            return !rule.trigger.has_value() && rule.subject.has_value() && !rule.effect.has_value() && rule.modifier.has_value();
        }
    } // namespace

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
            if (!isCompleteStandingRule(rule))
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
                pipeline.add({ruleId(slot), ModifierOperation::additive, static_cast<double>(value), static_cast<int>(slot)});
            }
        }
        return static_cast<int>(pipeline.evaluate(baseValue).value);
    }

    std::optional<gameplay::EntityId> BattleSimulation::triggeredBy(const content::TriggerFragment& trigger,
                                                                    const gameplay::GameplayEvent& event,
                                                                    const gameplay::EntityId& source) const
    {
        if (const auto* damage = std::get_if<gameplay::DamageApplied>(&event))
        {
            if (trigger.kind == content::TriggerKind::healthLost && damage->amount > 0)
            {
                return damage->target;
            }
            if (trigger.kind == content::TriggerKind::highDamageReceived && damage->amount > 10)
            {
                return damage->target;
            }
            if (trigger.kind == content::TriggerKind::highDamageDealt && damage->amount > 10)
            {
                return damage->source;
            }
            if (trigger.kind == content::TriggerKind::decayDamageReceived && damage->kind == gameplay::DamageKind::decay && damage->amount > 0)
            {
                return damage->target;
            }
        }
        if (const auto* healing = std::get_if<gameplay::HealingApplied>(&event))
        {
            if (trigger.kind == content::TriggerKind::healthHealed && healing->amount > 0)
            {
                return healing->target;
            }
            if (trigger.kind == content::TriggerKind::healedOther && healing->amount > 0 && healing->source != healing->target)
            {
                return healing->source;
            }
        }
        if (const auto* fatigue = std::get_if<gameplay::FatigueChanged>(&event);
            fatigue != nullptr && trigger.kind == content::TriggerKind::fatigueReachedNine && fatigue->previous < 9 && fatigue->current >= 9)
        {
            return fatigue->combatant;
        }
        if (const auto* defeated = std::get_if<gameplay::CombatantDefeated>(&event))
        {
            if (trigger.kind == content::TriggerKind::combatantDied)
            {
                return defeated->combatant;
            }
            if (trigger.kind == content::TriggerKind::killedOther && source != defeated->combatant)
            {
                return source;
            }
        }
        if (const auto* turn = std::get_if<gameplay::TurnStarted>(&event); turn != nullptr && trigger.kind == content::TriggerKind::turnStarted)
        {
            return turn->combatant;
        }
        if (const auto* expired = std::get_if<gameplay::StatusExpired>(&event);
            expired != nullptr && trigger.kind == content::TriggerKind::wokeUp && expired->status == gameplay::StatusId{"status.sleeping"})
        {
            return expired->combatant;
        }
        if (const auto* dodged = std::get_if<gameplay::AttackDodged>(&event); dodged != nullptr && trigger.kind == content::TriggerKind::attackDodged)
        {
            return dodged->target;
        }
        if (std::holds_alternative<gameplay::CriticalHit>(event) && trigger.kind == content::TriggerKind::criticalHitDealt)
        {
            return source;
        }

        const auto* status = std::get_if<gameplay::StatusGained>(&event);
        if (status == nullptr)
        {
            return std::nullopt;
        }
        if (trigger.kind == content::TriggerKind::markedReceived && statusIs(event, "status.marked"))
        {
            return status->combatant;
        }
        if (trigger.kind == content::TriggerKind::decayReceived && statusIs(event, "status.decay"))
        {
            return status->combatant;
        }
        if (trigger.kind == content::TriggerKind::buffReceived && statusIs(event, "status.buffed"))
        {
            return status->combatant;
        }
        if (trigger.kind == content::TriggerKind::debuffReceived && statusIs(event, "status.debuffed"))
        {
            return status->combatant;
        }
        if (trigger.kind == content::TriggerKind::confused && statusIs(event, "status.confused_action"))
        {
            return status->combatant;
        }
        if (source == status->combatant)
        {
            return std::nullopt;
        }
        if (trigger.kind == content::TriggerKind::markedOther && statusIs(event, "status.marked"))
        {
            return source;
        }
        if (trigger.kind == content::TriggerKind::decayApplied && statusIs(event, "status.decay"))
        {
            return source;
        }
        if (trigger.kind == content::TriggerKind::buffedOther && statusIs(event, "status.buffed"))
        {
            return source;
        }
        if (trigger.kind == content::TriggerKind::debuffedOther && statusIs(event, "status.debuffed"))
        {
            return source;
        }
        if (trigger.kind == content::TriggerKind::confusedOther && statusIs(event, "status.confused"))
        {
            return source;
        }
        if (trigger.kind == content::TriggerKind::sleepApplied && statusIs(event, "status.sleeping"))
        {
            return source;
        }
        return std::nullopt;
    }

    std::vector<CombatantState*> BattleSimulation::ruleTargets(content::SubjectKind subject, const gameplay::EntityId& trigger)
    {
        auto* triggeringCombatant = findMutable(trigger);
        if (triggeringCombatant == nullptr)
        {
            return {};
        }
        std::vector<CombatantState*> candidates;
        for (auto& candidate : combatants_)
        {
            if (!candidate.alive())
            {
                continue;
            }
            const auto other = candidate.id != trigger;
            const auto friendOfTrigger = candidate.team == triggeringCombatant->team;
            const auto matches = subject == content::SubjectKind::thisCombatant     ? !other
                                 : subject == content::SubjectKind::allOthers       ? other
                                 : subject == content::SubjectKind::allFriends      ? friendOfTrigger && other
                                 : subject == content::SubjectKind::allOpponents    ? !friendOfTrigger
                                 : subject == content::SubjectKind::randomCombatant ? other
                                 : subject == content::SubjectKind::randomOpponent  ? !friendOfTrigger
                                 : subject == content::SubjectKind::randomFriend    ? friendOfTrigger && other
                                 : subject == content::SubjectKind::allMarked       ? candidate.markedTurns > 0
                                 : subject == content::SubjectKind::allConfused     ? candidate.confusedTurns > 0
                                 : subject == content::SubjectKind::allSleeping     ? candidate.sleeping
                                 : subject == content::SubjectKind::allDecaying     ? !candidate.decay.empty()
                                 : subject == content::SubjectKind::allFullHealth   ? candidate.health == candidate.attributes.maximumHealth
                                 : subject == content::SubjectKind::allBuffed
                                     ? !candidate.buffs.empty()
                                 : subject == content::SubjectKind::allDebuffed         ? !candidate.debuffs.empty()
                                 : subject == content::SubjectKind::allBelowNineFatigue ? candidate.fatigue < 9
                                                                                        : candidate.fatigue >= 9;
            if (matches)
            {
                candidates.push_back(&candidate);
            }
        }
        const auto randomSubject = subject == content::SubjectKind::randomCombatant || subject == content::SubjectKind::randomOpponent ||
                                   subject == content::SubjectKind::randomFriend;
        if (!randomSubject || candidates.empty())
        {
            return candidates;
        }
        return {candidates.at(static_cast<std::size_t>(random_.between(0, static_cast<int>(candidates.size()) - 1)))};
    }

    void BattleSimulation::applyRuleEffect(const gameplay::EntityId& source,
                                           CombatantState& target,
                                           const content::EffectFragment& effect,
                                           std::vector<gameplay::GameplayEvent>& events)
    {
        if (effect.kind == content::EffectKind::damage)
        {
            const auto* actor = find(source);
            auto damage = actor == nullptr ? standingModifiedValue(content::ModifierKind::damageReceived, target, effect.amount)
                                           : modifiedDamage(*actor, target, effect.amount);
            if (actor == nullptr && target.rageTurns > 0)
            {
                damage = static_cast<int>(std::round(damage * 1.5));
            }
            damage = std::max(0, damage);
            target.health = std::max(0, target.health - damage);
            updateNofaceBuff(target);
            recordDamageApplied(source, target, damage, gameplay::DamageKind::direct, events);
            emitDefeatIfNeeded(target, events);
            return;
        }
        if (effect.kind == content::EffectKind::healing)
        {
            applyHealing(source, target, effect.amount, events);
            return;
        }
        if (effect.kind == content::EffectKind::fatigue)
        {
            changeFatigue(target, effect.amount, events);
            return;
        }
        if (effect.kind == content::EffectKind::dynamite)
        {
            target.dynamite = true;
            recordStatusGained(target, gameplay::StatusId{"status.dynamite"}, events);
            return;
        }
        if (effect.kind == content::EffectKind::applyAttribute && effect.attribute)
        {
            content::BuffDefinition buff;
            buff.duration = effect.duration;
            buff.positive = effect.amount >= 0;
            switch (*effect.attribute)
            {
            case content::ModifierKind::damageDealt:
                buff.attributes.damageMinimum = effect.amount;
                buff.attributes.damageMaximum = effect.amount;
                break;
            case content::ModifierKind::armour:
                buff.attributes.armour = effect.amount;
                break;
            case content::ModifierKind::dodge:
                buff.attributes.dodge = effect.amount;
                break;
            case content::ModifierKind::criticalHitChance:
                buff.attributes.criticalHitChance = effect.amount;
                break;
            case content::ModifierKind::precision:
                buff.attributes.precision = effect.amount;
                break;
            case content::ModifierKind::initiative:
                buff.attributes.initiative = effect.amount;
                break;
            case content::ModifierKind::damageReceived:
            case content::ModifierKind::nofaceBuff:
                break;
            }
            applyBuff(target, buff, effect.duration, events);
            return;
        }
        if (effect.kind != content::EffectKind::applyStatus || !effect.status)
        {
            return;
        }
        const auto duration = std::max(0, effect.duration);
        if (duration == 0)
        {
            return;
        }
        if (*effect.status == gameplay::StatusId{"status.marked"})
        {
            target.markedTurns += duration;
        }
        else if (*effect.status == gameplay::StatusId{"status.bounty"})
        {
            target.bountyTurns += duration;
        }
        else if (*effect.status == gameplay::StatusId{"status.confused"})
        {
            target.confusedTurns += duration;
        }
        else if (*effect.status == gameplay::StatusId{"status.decay"})
        {
            const auto damage = std::max(0, effect.amount);
            if (damage == 0)
            {
                return;
            }
            target.decay.push_back({duration, damage});
        }
        else if (*effect.status == gameplay::StatusId{"status.rage"})
        {
            target.rageTurns += duration;
        }
        recordStatusGained(target, *effect.status, events);
    }

    void BattleSimulation::resolveTriggeredRules(std::vector<gameplay::GameplayEvent>& events,
                                                 const gameplay::EntityId& source,
                                                 std::size_t firstEvent)
    {
        if (rules_ == nullptr || activeRules_ == nullptr || firstEvent >= events.size())
        {
            return;
        }
        RuleEventQueue queue;
        for (auto index = firstEvent; index < events.size(); ++index)
        {
            static_cast<void>(queue.enqueue(events.at(index), 0, source));
        }
        static_cast<void>(queue.drain([&](const QueuedRuleEvent& queued, RuleEventQueue& pending) {
            const auto eventSource = queued.source.value_or(source);
            for (std::size_t slot = 0; slot < activeRules_->size(); ++slot)
            {
                const auto& rule = activeRules_->at(slot);
                const auto activeRuleId = ruleId(slot);
                if (std::ranges::find(queued.activatedRules, activeRuleId) != queued.activatedRules.end() || !isCompleteTriggeredRule(rule))
                {
                    continue;
                }
                const auto& trigger = std::get<content::TriggerFragment>(rules_->fragments.at(*rule.trigger).payload);
                const auto triggeredCombatant = triggeredBy(trigger, queued.event, eventSource);
                if (!triggeredCombatant)
                {
                    continue;
                }
                const auto& subject = std::get<content::SubjectFragment>(rules_->fragments.at(*rule.subject).payload);
                const auto& effect = std::get<content::EffectFragment>(rules_->fragments.at(*rule.effect).payload);
                auto targets = ruleTargets(subject.kind, *triggeredCombatant);
                if (targets.empty())
                {
                    continue;
                }
                events.emplace_back(gameplay::RuleActivated{activeRuleId, *triggeredCombatant});
                const auto firstGeneratedEvent = events.size();
                for (auto* target : targets)
                {
                    applyRuleEffect(*triggeredCombatant, *target, effect, events);
                }
                auto activatedRules = queued.activatedRules;
                activatedRules.push_back(activeRuleId);
                for (auto index = firstGeneratedEvent; index < events.size(); ++index)
                {
                    static_cast<void>(pending.enqueue(events.at(index), queued.depth + 1, *triggeredCombatant, activatedRules));
                }
            }
        }));
    }

    void BattleSimulation::advanceToPlayableTurn(std::vector<gameplay::GameplayEvent>& events)
    {
        while (!finished_)
        {
            const auto firstTurnEvent = events.size();
            advanceTurn(events);
            updateFinished(events);
            if (finished_)
            {
                return;
            }
            const auto actor = currentActor();
            resolveTriggeredRules(events, actor, firstTurnEvent);
            updateFinished(events);
            if (finished_ || find(actor)->alive())
            {
                return;
            }
        }
    }

    std::vector<gameplay::GameplayEvent> BattleSimulation::begin()
    {
        if (begun_ || finished_)
        {
            return {};
        }
        begun_ = true;
        std::vector<gameplay::GameplayEvent> events{gameplay::TurnStarted{currentActor()}};
        const auto actor = currentActor();
        resolveTriggeredRules(events, actor, 0);
        const auto firstEquipmentEvent = events.size();
        resolveEquipmentEffects(events);
        resolveTriggeredRules(events, actor, firstEquipmentEvent);
        updateFinished(events);
        if (!finished_ && !find(actor)->alive())
        {
            advanceToPlayableTurn(events);
        }
        return events;
    }
} // namespace soma::simulation
