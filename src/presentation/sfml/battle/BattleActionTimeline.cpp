#include "presentation/sfml/battle/BattleActionTimeline.hpp"
#include "presentation/battle/BattleEventBatcher.hpp"

#include <algorithm>
#include <iterator>

namespace
{
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
    BattleActionTimeline::BattleActionTimeline(IBattleSoundPlayer& sounds)
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
        action->events = BattleEventBatcher::takeNext(action->deferredEvents);
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
        const auto riposte = std::ranges::find_if(action->events, BattleEventBatcher::isRiposteTrigger);
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
