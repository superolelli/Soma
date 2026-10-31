#include "presentation/sfml/battle/BattleActionStarter.hpp"

#include "presentation/battle/BattleEventPresentation.hpp"
#include "presentation/sfml/common/SpriterAnimationPlayer.hpp"
#include "simulation/battle/AbilityTargets.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>

namespace
{
    bool isRiposteTrigger(const soma::gameplay::GameplayEvent& event)
    {
        const auto* triggered = std::get_if<soma::gameplay::StatusTriggered>(&event);
        return triggered != nullptr && triggered->status == soma::gameplay::StatusId{"status.riposte"};
    }

    const soma::simulation::CombatantState* findBeforeAction(const std::vector<soma::simulation::CombatantState>& combatants,
                                                             const soma::gameplay::EntityId& id)
    {
        const auto found = std::ranges::find(combatants, id, &soma::simulation::CombatantState::id);
        return found == combatants.end() ? nullptr : &*found;
    }

    std::vector<soma::gameplay::EntityId> intendedTargets(const soma::simulation::BattleActionCommand& command,
                                                          const soma::content::AbilityDefinition& ability,
                                                          const std::vector<soma::simulation::CombatantState>& combatants,
                                                          bool redirected)
    {
        const auto* actor = findBeforeAction(combatants, command.actor);
        const auto* primary = findBeforeAction(combatants, command.primaryTarget);
        if (actor == nullptr || primary == nullptr || redirected)
        {
            return {command.primaryTarget};
        }

        return soma::simulation::abilityTargets(ability.targeting, *actor, *primary, combatants);
    }

    bool presentsOnActor(const soma::content::AbilityEffect& effect)
    {
        return effect.actorBuff.duration > 0 || effect.healSelf != 0 || effect.addMissSelf != 0 || effect.riposteSelfRounds != 0;
    }
} // namespace

namespace soma::presentation
{
    BattleActionStarter::BattleActionStarter(sf::RenderWindow& window,
                                             const content::BattleContent& content,
                                             simulation::BattleSimulation& battle,
                                             CombatantViews& views,
                                             IAnimationPlayer& effect,
                                             std::optional<PresentedAction>& action,
                                             std::vector<FloatingText>& floatingTexts)
        : window_(window)
        , content_(content)
        , battle_(battle)
        , views_(views)
        , effect_(effect)
        , action_(action)
        , floatingTexts_(floatingTexts)
    {
    }

    void BattleActionStarter::start(simulation::BattleActionCommand command, bool announce)
    {
        const auto combatantsBeforeAction = battle_.combatants();
        auto result = battle_.execute(command);
        if (!result.hasValue())
        {
            floatingTexts_.push_back({command.actor, result.error(), sf::Color::White, 0.0F, 22U});
            return;
        }
        addSummonedCombatants(result.value());
        preparePresentation(std::move(command), std::move(result.value()), combatantsBeforeAction, announce);
    }

    void BattleActionStarter::startPass(const gameplay::EntityId& actor)
    {
        const auto combatantsBeforeAction = battle_.combatants();
        auto result = battle_.passTurn(actor);
        if (!result.hasValue())
        {
            floatingTexts_.push_back({actor, result.error(), sf::Color::White, 0.0F, 22U});
            return;
        }
        action_ = PresentedAction{{actor, gameplay::AbilityId{"ability.pass"}, actor},
                                  {},
                                  std::move(result.value()),
                                  {},
                                  {},
                                  combatantsBeforeAction,
                                  {},
                                  ActionStage::result,
                                  1.0F};
    }

    void BattleActionStarter::addSummonedCombatants(const std::vector<gameplay::GameplayEvent>& events)
    {
        auto addedCombatant = false;
        for (const auto& event : events)
        {
            const auto* summoned = std::get_if<gameplay::CombatantSummoned>(&event);
            if (summoned == nullptr)
            {
                continue;
            }
            const auto* state = battle_.find(summoned->combatant);
            const auto& presentation = content_.presentation.at(summoned->definition);
            auto animation = std::make_unique<SpriterAnimationPlayer>(window_, presentation.model, presentation.entity);
            const auto normalScale = static_cast<float>(content_.layout.normalScale);
            const auto scale = normalScale + static_cast<float>(presentation.scaleAdjustment);
            animation->setScale(scale, scale);
            animation->setPosition(0.0F, static_cast<float>(content_.layout.groundY));
            animation->play("idle", 0.7F);
            views_.emplace(summoned->combatant,
                           CombatantView{std::move(animation),
                                         summoned->definition,
                                         {},
                                         {},
                                         normalScale,
                                         static_cast<float>(presentation.scaleAdjustment),
                                         static_cast<float>(state->health),
                                         static_cast<float>(state->health),
                                         state->health,
                                         0.7F,
                                         state->attributes.maximumHealth});
            addedCombatant = true;
        }
        if (addedCombatant)
        {
            positionEnemyViews(views_, battle_);
        }
    }

    void BattleActionStarter::preparePresentation(simulation::BattleActionCommand command,
                                                  std::vector<gameplay::GameplayEvent> events,
                                                  std::vector<simulation::CombatantState> combatantsBeforeAction,
                                                  bool announce)
    {
        const auto nextTurn = std::ranges::find_if(events, [](const auto& event) {
            return std::holds_alternative<gameplay::TurnStarted>(event);
        });
        std::vector<gameplay::GameplayEvent> deferred;
        if (nextTurn != events.end())
        {
            deferred.assign(nextTurn, events.end());
            events.erase(nextTurn, events.end());
        }
        const auto firstInterruption = std::ranges::find_if(events, [](const auto& event) {
            return std::holds_alternative<gameplay::RuleActivated>(event) || std::holds_alternative<gameplay::ItemActivated>(event) ||
                   isRiposteTrigger(event);
        });
        if (firstInterruption != events.end())
        {
            std::vector<gameplay::GameplayEvent> interruptionEvents{firstInterruption, events.end()};
            events.erase(firstInterruption, events.end());
            deferred.insert(
                deferred.begin(), std::make_move_iterator(interruptionEvents.begin()), std::make_move_iterator(interruptionEvents.end()));
        }
        events = groupedForPresentation(std::move(events));
        action_ = PresentedAction{std::move(command), std::move(events), std::move(deferred), {}, {}, std::move(combatantsBeforeAction)};
        const auto redirected = std::ranges::find_if(action_->events, [](const auto& event) {
            return std::holds_alternative<gameplay::TargetRedirected>(event);
        });
        if (redirected != action_->events.end())
        {
            action_->command.primaryTarget = std::get<gameplay::TargetRedirected>(*redirected).resolvedTarget;
        }
        action_->affectedCombatants.clear();
        const auto targets = intendedTargets(
            action_->command, content_.abilities.at(action_->command.ability), action_->combatantsBeforeAction, redirected != action_->events.end());
        action_->affectedCombatants.insert(action_->affectedCombatants.end(), targets.begin(), targets.end());

        const auto* actorBeforeAction = findBeforeAction(action_->combatantsBeforeAction, action_->command.actor);
        const auto* targetBeforeAction = findBeforeAction(action_->combatantsBeforeAction, action_->command.primaryTarget);
        const auto& ability = content_.abilities.at(action_->command.ability);
        if (actorBeforeAction != nullptr && targetBeforeAction != nullptr)
        {
            const auto friendly = (actorBeforeAction->team == targetBeforeAction->team) != actorBeforeAction->confusedThisTurn;
            const auto& effect = friendly ? ability.friendlyEffect : ability.hostileEffect;
            if (presentsOnActor(effect) && !actorBeforeAction->confusedThisTurn)
            {
                action_->affectedCombatants.push_back(action_->command.actor);
            }
        }
        std::ranges::sort(action_->affectedCombatants);
        const auto uniqueEnd = std::ranges::unique(action_->affectedCombatants).begin();
        action_->affectedCombatants.erase(uniqueEnd, action_->affectedCombatants.end());
        if (action_->affectedCombatants.empty())
        {
            action_->affectedCombatants.push_back(action_->command.primaryTarget);
        }

        action_->stage = announce ? ActionStage::announcement : ActionStage::execution;
        if (!announce)
        {
            moveToAbilityStage(*action_, views_, effect_, battle_);
        }
    }
} // namespace soma::presentation
