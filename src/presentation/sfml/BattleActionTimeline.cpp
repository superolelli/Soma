#include "presentation/sfml/BattleActionTimeline.hpp"

#include <algorithm>
#include <iterator>

namespace
{
    bool isRiposteTrigger(const soma::gameplay::GameplayEvent& event)
    {
        const auto* triggered = std::get_if<soma::gameplay::StatusTriggered>(&event);
        return triggered != nullptr && triggered->status == soma::gameplay::StatusId{"status.riposte"};
    }

    bool startsPresentationBatch(const soma::gameplay::GameplayEvent& event)
    {
        return std::holds_alternative<soma::gameplay::TurnStarted>(event) ||
               std::holds_alternative<soma::gameplay::ItemActivated>(event) || isRiposteTrigger(event);
    }

    bool isPresentationMetadata(const soma::gameplay::GameplayEvent& event)
    {
        return std::holds_alternative<soma::gameplay::TurnStarted>(event) ||
               std::holds_alternative<soma::gameplay::AbilityUsed>(event) ||
               std::holds_alternative<soma::gameplay::TargetRedirected>(event);
    }

    bool pairsWithFollowingResult(const soma::gameplay::GameplayEvent& event)
    {
        return std::holds_alternative<soma::gameplay::CriticalHit>(event) ||
               std::holds_alternative<soma::gameplay::StatusTriggered>(event) || isRiposteTrigger(event);
    }

    bool isImmediateOutcome(const soma::gameplay::GameplayEvent& event)
    {
        return std::holds_alternative<soma::gameplay::DamageApplied>(event) ||
               std::holds_alternative<soma::gameplay::HealingApplied>(event) ||
               std::holds_alternative<soma::gameplay::FatigueChanged>(event);
    }

    enum class OutcomeBatch
    {
        single,
        dodge,
        damage,
        healing,
        fatigue,
        statusGained,
        statusExpired,
        statusTriggered,
        resisted,
        buffsRemoved,
        debuffsRemoved,
        decayTransferred,
        decayDetonated,
        effectsProlonged,
        defeated
    };

    OutcomeBatch outcomeBatch(const std::vector<soma::gameplay::GameplayEvent>& deferred)
    {
        const auto& event = deferred.front();
        if (std::holds_alternative<soma::gameplay::AttackDodged>(event))
        {
            return OutcomeBatch::dodge;
        }
        if (std::holds_alternative<soma::gameplay::DamageApplied>(event))
        {
            return OutcomeBatch::damage;
        }
        if (std::holds_alternative<soma::gameplay::HealingApplied>(event))
        {
            return OutcomeBatch::healing;
        }
        if (std::holds_alternative<soma::gameplay::FatigueChanged>(event))
        {
            return OutcomeBatch::fatigue;
        }
        if (std::holds_alternative<soma::gameplay::StatusGained>(event))
        {
            return OutcomeBatch::statusGained;
        }
        if (std::holds_alternative<soma::gameplay::StatusExpired>(event))
        {
            return OutcomeBatch::statusExpired;
        }
        if (std::holds_alternative<soma::gameplay::StatusTriggered>(event))
        {
            return OutcomeBatch::statusTriggered;
        }
        if (std::holds_alternative<soma::gameplay::EffectResisted>(event))
        {
            return OutcomeBatch::resisted;
        }
        if (std::holds_alternative<soma::gameplay::BuffsRemoved>(event))
        {
            return OutcomeBatch::buffsRemoved;
        }
        if (std::holds_alternative<soma::gameplay::DebuffsRemoved>(event))
        {
            return OutcomeBatch::debuffsRemoved;
        }
        if (std::holds_alternative<soma::gameplay::DecayTransferred>(event))
        {
            return OutcomeBatch::decayTransferred;
        }
        if (std::holds_alternative<soma::gameplay::DecayDetonated>(event))
        {
            return OutcomeBatch::decayDetonated;
        }
        if (std::holds_alternative<soma::gameplay::NegativeEffectsProlonged>(event))
        {
            return OutcomeBatch::effectsProlonged;
        }
        if (std::holds_alternative<soma::gameplay::CombatantDefeated>(event))
        {
            return OutcomeBatch::defeated;
        }
        if (!std::holds_alternative<soma::gameplay::CriticalHit>(event))
        {
            return OutcomeBatch::single;
        }

        const auto firstResult = std::ranges::find_if(deferred, [](const auto& candidate) {
            return !std::holds_alternative<soma::gameplay::CriticalHit>(candidate);
        });
        if (firstResult != deferred.end() && std::holds_alternative<soma::gameplay::DamageApplied>(*firstResult))
        {
            return OutcomeBatch::damage;
        }
        if (firstResult != deferred.end() && std::holds_alternative<soma::gameplay::HealingApplied>(*firstResult))
        {
            return OutcomeBatch::healing;
        }
        return OutcomeBatch::single;
    }

    bool sameStatus(const soma::gameplay::GameplayEvent& left,
                    const soma::gameplay::GameplayEvent& right,
                    OutcomeBatch batch)
    {
        if (batch == OutcomeBatch::statusGained)
        {
            return std::get<soma::gameplay::StatusGained>(left).status == std::get<soma::gameplay::StatusGained>(right).status;
        }
        if (batch == OutcomeBatch::statusExpired)
        {
            return std::get<soma::gameplay::StatusExpired>(left).status == std::get<soma::gameplay::StatusExpired>(right).status;
        }
        if (batch == OutcomeBatch::statusTriggered)
        {
            return std::get<soma::gameplay::StatusTriggered>(left).status == std::get<soma::gameplay::StatusTriggered>(right).status;
        }
        if (batch == OutcomeBatch::resisted)
        {
            return std::get<soma::gameplay::EffectResisted>(left).status == std::get<soma::gameplay::EffectResisted>(right).status;
        }
        return true;
    }

    bool belongsToBatch(const soma::gameplay::GameplayEvent& first,
                        const soma::gameplay::GameplayEvent& candidate,
                        OutcomeBatch batch)
    {
        if (batch == OutcomeBatch::damage)
        {
            return std::holds_alternative<soma::gameplay::DamageApplied>(candidate) ||
                   std::holds_alternative<soma::gameplay::CriticalHit>(candidate);
        }
        if (batch == OutcomeBatch::healing)
        {
            return std::holds_alternative<soma::gameplay::HealingApplied>(candidate) ||
                   std::holds_alternative<soma::gameplay::CriticalHit>(candidate);
        }
        if (batch == OutcomeBatch::dodge)
        {
            return std::holds_alternative<soma::gameplay::AttackDodged>(candidate);
        }
        if (batch == OutcomeBatch::fatigue)
        {
            return std::holds_alternative<soma::gameplay::FatigueChanged>(candidate);
        }
        if (batch == OutcomeBatch::statusGained)
        {
            return std::holds_alternative<soma::gameplay::StatusGained>(candidate) && sameStatus(first, candidate, batch);
        }
        if (batch == OutcomeBatch::statusExpired)
        {
            return std::holds_alternative<soma::gameplay::StatusExpired>(candidate) && sameStatus(first, candidate, batch);
        }
        if (batch == OutcomeBatch::statusTriggered)
        {
            return std::holds_alternative<soma::gameplay::StatusTriggered>(candidate) && sameStatus(first, candidate, batch);
        }
        if (batch == OutcomeBatch::resisted)
        {
            return std::holds_alternative<soma::gameplay::EffectResisted>(candidate) && sameStatus(first, candidate, batch);
        }
        if (batch == OutcomeBatch::buffsRemoved)
        {
            return std::holds_alternative<soma::gameplay::BuffsRemoved>(candidate);
        }
        if (batch == OutcomeBatch::debuffsRemoved)
        {
            return std::holds_alternative<soma::gameplay::DebuffsRemoved>(candidate);
        }
        if (batch == OutcomeBatch::decayTransferred)
        {
            return std::holds_alternative<soma::gameplay::DecayTransferred>(candidate);
        }
        if (batch == OutcomeBatch::decayDetonated)
        {
            return std::holds_alternative<soma::gameplay::DecayDetonated>(candidate);
        }
        if (batch == OutcomeBatch::effectsProlonged)
        {
            return std::holds_alternative<soma::gameplay::NegativeEffectsProlonged>(candidate);
        }
        if (batch == OutcomeBatch::defeated)
        {
            return std::holds_alternative<soma::gameplay::CombatantDefeated>(candidate);
        }
        return false;
    }

    std::vector<soma::gameplay::GameplayEvent> takeNextBatch(std::vector<soma::gameplay::GameplayEvent>& deferred)
    {
        while (!deferred.empty() && isPresentationMetadata(deferred.front()))
        {
            deferred.erase(deferred.begin());
        }
        if (deferred.empty())
        {
            return {};
        }
        auto nextBatch = std::next(deferred.begin());
        if (std::holds_alternative<soma::gameplay::ItemActivated>(deferred.front()))
        {
            nextBatch = std::ranges::find_if(std::next(deferred.begin()), deferred.end(), startsPresentationBatch);
        }
        else if (isRiposteTrigger(deferred.front()) && nextBatch != deferred.end() && isImmediateOutcome(*nextBatch))
        {
            ++nextBatch;
        }
        else
        {
            const auto batch = outcomeBatch(deferred);
            if (batch != OutcomeBatch::single)
            {
                nextBatch = std::ranges::find_if(std::next(deferred.begin()), deferred.end(), [&](const auto& candidate) {
                    return !belongsToBatch(deferred.front(), candidate, batch);
                });
            }
            else if (pairsWithFollowingResult(deferred.front()) && nextBatch != deferred.end() && isImmediateOutcome(*nextBatch))
            {
                ++nextBatch;
            }
        }
        std::vector<soma::gameplay::GameplayEvent> result;
        result.assign(std::make_move_iterator(deferred.begin()), std::make_move_iterator(nextBatch));
        deferred.erase(deferred.begin(), nextBatch);
        return result;
    }

    void updatePositionsAfterMovement(const soma::presentation::PresentedAction& action,
                                      soma::presentation::CombatantViews& views,
                                      const soma::simulation::BattleSimulation& battle)
    {
        const auto abilityFound = battle.content().abilities.find(action.command.ability);
        if (abilityFound == battle.content().abilities.end())
        {
            return;
        }
        const auto& ability = abilityFound->second;
        if (ability.friendlyEffect.moveTargetsRandomly || ability.hostileEffect.moveTargetsRandomly)
        {
            soma::presentation::positionCombatantViews(views, battle);
        }
    }
} // namespace

namespace soma::presentation
{
    BattleActionTimeline::BattleActionTimeline(BattleSoundPlayer& sounds)
        : sounds_(sounds)
    {
    }

    void BattleActionTimeline::update(float elapsedSeconds,
                                      std::optional<PresentedAction>& action,
                                      CombatantViews& views,
                                      IAnimationPlayer& effect,
                                      const simulation::BattleSimulation& battle,
                                      std::vector<FloatingText>& floatingTexts) const
    {
        if (!action)
        {
            return;
        }

        action->elapsed += elapsedSeconds;
        switch (action->stage)
        {
        case ActionStage::announcement:
            updateAnnouncement(*action, views, effect, battle);
            break;
        case ActionStage::execution:
            updateExecution(action, views, effect, battle, floatingTexts);
            break;
        case ActionStage::itemInterruption:
            updateItemInterruption(*action, views, battle, floatingTexts);
            break;
        case ActionStage::reaction:
            updateReaction(*action, views, battle, floatingTexts);
            break;
        case ActionStage::result:
            updateResult(action, views, battle, floatingTexts);
            break;
        case ActionStage::defeat:
            updateDefeat(action, views, battle, floatingTexts);
            break;
        }
    }

    void BattleActionTimeline::updateAnnouncement(PresentedAction& action,
                                                  CombatantViews& views,
                                                  IAnimationPlayer& effect,
                                                  const simulation::BattleSimulation& battle) const
    {
        if (action.elapsed >= static_cast<float>(battle.content().layout.announcementDuration))
        {
            moveToAbilityStage(action, views, effect, battle);
        }
    }

    void BattleActionTimeline::updateExecution(std::optional<PresentedAction>& action,
                                               CombatantViews& views,
                                               IAnimationPlayer& effect,
                                               const simulation::BattleSimulation& battle,
                                               std::vector<FloatingText>& floatingTexts) const
    {
        if (!action->startSoundsPlayed)
        {
            sounds_.playActionStart(action->events);
            action->startSoundsPlayed = true;
        }
        const auto actorFinished = !views.at(action->command.actor).animation->isPlaying();
        if (!((actorFinished && !effect.isPlaying()) ||
              action->elapsed >= static_cast<float>(battle.content().layout.executionTimeout)))
        {
            return;
        }

        std::vector<gameplay::GameplayEvent> pending;
        pending.reserve(action->events.size() + action->deferredEvents.size());
        pending.insert(pending.end(), std::make_move_iterator(action->events.begin()), std::make_move_iterator(action->events.end()));
        pending.insert(pending.end(),
                       std::make_move_iterator(action->deferredEvents.begin()),
                       std::make_move_iterator(action->deferredEvents.end()));
        action->events.clear();
        action->deferredEvents = std::move(pending);
        restoreCombatants(views);
        applyPersistentAnimations(views, battle);
        beginDeferredBatch(action, views, battle, floatingTexts);
    }

    void BattleActionTimeline::updateItemInterruption(PresentedAction& action,
                                                       CombatantViews& views,
                                                       const simulation::BattleSimulation& battle,
                                                       std::vector<FloatingText>& floatingTexts) const
    {
        if (action.elapsed < static_cast<float>(battle.content().layout.itemSpeechDuration))
        {
            return;
        }
        const auto activation = std::ranges::find_if(action.events, [](const auto& event) {
            return std::holds_alternative<gameplay::ItemActivated>(event);
        });
        if (activation != action.events.end())
        {
            const auto& item = std::get<gameplay::ItemActivated>(*activation);
            const auto presentation = battle.content().itemActivations.find(item.item);
            if (presentation != battle.content().itemActivations.end())
            {
                floatingTexts.push_back({item.carrier,
                                         presentation->second.notification,
                                         sf::Color{235, 196, 65},
                                         0.0F,
                                         26U,
                                         FloatingTextAnchor::combatant,
                                         sf::Color::Black,
                                         3.0F,
                                         FloatingTextFont::interface});
            }
        }
        presentResults(action, views, battle, floatingTexts);
    }

    void BattleActionTimeline::updateReaction(PresentedAction& action,
                                              CombatantViews& views,
                                              const simulation::BattleSimulation& battle,
                                              std::vector<FloatingText>& floatingTexts) const
    {
        if (action.elapsed >= static_cast<float>(battle.content().layout.reactionDuration))
        {
            presentResults(action, views, battle, floatingTexts);
        }
    }

    void BattleActionTimeline::presentResults(PresentedAction& action,
                                              CombatantViews& views,
                                              const simulation::BattleSimulation& battle,
                                              std::vector<FloatingText>& floatingTexts) const
    {
        sounds_.playResults(action.events);
        applyPresentedResults(action, views, floatingTexts);
        restoreCombatants(views);
        applyPersistentAnimations(views, battle);
        action.stage = ActionStage::result;
        action.elapsed = 0.0F;
    }

    void BattleActionTimeline::updateResult(std::optional<PresentedAction>& action,
                                            CombatantViews& views,
                                            const simulation::BattleSimulation& battle,
                                            std::vector<FloatingText>& floatingTexts) const
    {
        const auto resultDelay = action->wakingCombatant ? battle.content().layout.wakeResultDuration : battle.content().layout.resultDuration;
        if (action->elapsed < static_cast<float>(resultDelay))
        {
            return;
        }
        const auto hasDefeat = std::ranges::any_of(action->events, [](const auto& event) {
            return std::holds_alternative<gameplay::CombatantDefeated>(event);
        });
        if (hasDefeat)
        {
            beginDefeatOrFinish(action, views);
        }
        else if (!action->deferredEvents.empty())
        {
            beginDeferredBatch(action, views, battle, floatingTexts);
        }
        else
        {
            action.reset();
        }
    }

    void BattleActionTimeline::updateDefeat(std::optional<PresentedAction>& action,
                                            CombatantViews& views,
                                            const simulation::BattleSimulation& battle,
                                            std::vector<FloatingText>& floatingTexts) const
    {
        if (action->elapsed < static_cast<float>(battle.content().layout.defeatDuration))
        {
            return;
        }
        if (action->deferredEvents.empty())
        {
            action.reset();
            return;
        }
        beginDeferredBatch(action, views, battle, floatingTexts);
    }

    void BattleActionTimeline::beginDeferredBatch(std::optional<PresentedAction>& action,
                                                  CombatantViews& views,
                                                  const simulation::BattleSimulation& battle,
                                                  std::vector<FloatingText>& floatingTexts) const
    {
        while (!action->deferredEvents.empty() && std::holds_alternative<gameplay::TurnStarted>(action->deferredEvents.front()))
        {
            action->command.actor = std::get<gameplay::TurnStarted>(action->deferredEvents.front()).combatant;
            action->deferredEvents.erase(action->deferredEvents.begin());
        }
        action->events = takeNextBatch(action->deferredEvents);
        action->wakingCombatant.reset();
        if (action->events.empty())
        {
            action.reset();
            return;
        }
        const auto itemActivation = std::ranges::find_if(action->events, [](const auto& event) {
            return std::holds_alternative<gameplay::ItemActivated>(event);
        });
        if (itemActivation != action->events.end())
        {
            action->stage = ActionStage::itemInterruption;
            action->elapsed = 0.0F;
            return;
        }
        const auto riposte = std::ranges::find_if(action->events, isRiposteTrigger);
        if (riposte != action->events.end())
        {
            const auto& triggered = std::get<gameplay::StatusTriggered>(*riposte);
            floatingTexts.push_back({triggered.combatant, "Gegenschlag!", sf::Color{235, 196, 65}});
            const auto damage = std::ranges::find_if(action->events, [](const auto& event) {
                return std::holds_alternative<gameplay::DamageApplied>(event);
            });
            if (damage != action->events.end())
            {
                views.at(std::get<gameplay::DamageApplied>(*damage).target).animation->play("attacked");
            }
            action->stage = ActionStage::reaction;
            action->elapsed = 0.0F;
            return;
        }
        for (const auto& event : action->events)
        {
            const auto* expired = std::get_if<gameplay::StatusExpired>(&event);
            if (expired != nullptr && expired->status == gameplay::StatusId{"status.sleeping"})
            {
                action->wakingCombatant = expired->combatant;
                break;
            }
        }
        sounds_.playResults(action->events);
        applyPresentedResults(*action, views, floatingTexts);
        updatePositionsAfterMovement(*action, views, battle);
        restoreCombatants(views);
        applyPersistentAnimations(views, battle);
        action->stage = ActionStage::result;
        action->elapsed = 0.0F;
    }

    void BattleActionTimeline::beginDefeatOrFinish(std::optional<PresentedAction>& action, CombatantViews& views)
    {
        auto defeated = false;
        for (const auto& event : action->events)
        {
            const auto* defeat = std::get_if<gameplay::CombatantDefeated>(&event);
            if (defeat == nullptr)
            {
                continue;
            }
            views.at(defeat->combatant).animation->play("dying");
            defeated = true;
        }
        if (!defeated)
        {
            action.reset();
            return;
        }
        action->stage = ActionStage::defeat;
        action->elapsed = 0.0F;
    }
} // namespace soma::presentation
