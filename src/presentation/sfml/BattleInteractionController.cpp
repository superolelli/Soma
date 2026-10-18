#include "presentation/sfml/BattleInteractionController.hpp"

#include <algorithm>

namespace soma::presentation
{
    BattleInteractionController::BattleInteractionController(
        sf::RenderWindow& window,
        const content::BattleContent& content,
        simulation::BattleSimulation& battle,
        simulation::EnemyPolicy& enemyPolicy,
        BattleActionStarter& actionStarter,
        const BattleLaunchOptions& launchOptions,
        CombatantViews& views,
        const std::optional<PresentedAction>& action,
        sf::FloatRect skipTurnBounds)
        : window_(window), content_(content), battle_(battle), enemyPolicy_(enemyPolicy),
          actionStarter_(actionStarter), launchOptions_(launchOptions), views_(views),
          action_(action), skipTurnBounds_(skipTurnBounds)
    {
    }

    void BattleInteractionController::prepareTurn()
    {
        if (action_.has_value() || battle_.finished()) return;
        const auto* actor = battle_.find(battle_.currentActor());
        if (actor->team == gameplay::Team::players) refreshAbilityButtons(*actor);
        if (actor->team == gameplay::Team::players && launchOptions_.automatic)
            runAutomaticTurn(*actor);
        else if (actor->team == gameplay::Team::enemies)
        {
            const auto command = enemyPolicy_.choose(battle_);
            if (command.hasValue()) actionStarter_.start(command.value(), true);
            else actionStarter_.startPass(actor->id);
        }
    }

    void BattleInteractionController::refreshAbilityButtons(
        const simulation::CombatantState& actor)
    {
        if (abilityButtonActor_ == actor.id) return;
        abilityButtons_ = createAbilityButtons(content_, actor);
        abilityButtonActor_ = actor.id;
        selectedAbility_.reset();
    }

    void BattleInteractionController::runAutomaticTurn(
        const simulation::CombatantState& actor)
    {
        const auto ownsRequested = launchOptions_.automaticAbility.has_value() &&
            std::ranges::any_of(abilityButtons_, [this](const auto& button) {
                return button.ability == *launchOptions_.automaticAbility;
            });
        for (const auto& button : abilityButtons_)
        {
            if (ownsRequested && button.ability != *launchOptions_.automaticAbility) continue;
            const auto targets = battle_.validPrimaryTargets(button.ability);
            if (!targets.empty())
            {
                actionStarter_.start({actor.id, button.ability, targets.front()}, false);
                return;
            }
        }
        actionStarter_.startPass(actor.id);
    }

    void BattleInteractionController::processEvents(const std::function<bool(const sf::Event&)>& intercept)
    {
        while (const auto event = window_.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window_.close();
            if (intercept && intercept(*event)) continue;
            if (const auto* key = event->getIf<sf::Event::KeyPressed>(); key && key->code == sf::Keyboard::Key::Escape)
                window_.close();
            if (const auto* moved = event->getIf<sf::Event::MouseMoved>())
                inspectAt(window_.mapPixelToCoords(moved->position));
            if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>();
                click && click->button == sf::Mouse::Button::Left)
            {
                const auto position = window_.mapPixelToCoords(click->position);
                handleClick(position);
            }
        }
    }

    bool BattleInteractionController::clickableAt(sf::Vector2f position) const
    {
        if (action_.has_value() || battle_.finished()) return false;
        const auto* actor = battle_.find(battle_.currentActor());
        if (actor == nullptr || actor->team != gameplay::Team::players) return false;
        if (skipTurnBounds_.contains(position) ||
            std::ranges::any_of(abilityButtons_, [position](const auto& button) { return button.bounds.contains(position); }))
            return true;
        if (!selectedAbility_) return false;
        return std::ranges::any_of(battle_.validPrimaryTargets(*selectedAbility_), [&](const auto& target) {
            return views_.at(target).bounds.contains(position);
        });
    }

    void BattleInteractionController::inspectAt(sf::Vector2f position)
    {
        inspectedCombatant_.reset();
        for (const auto& [id, view] : views_)
        {
            const auto* state = battle_.find(id);
            if (state != nullptr && state->alive() && view.bounds.contains(position))
            {
                inspectedCombatant_ = id;
            }
        }
    }

    void BattleInteractionController::handleClick(sf::Vector2f position)
    {
        if (action_.has_value() || battle_.finished()) return;
        const auto* actor = battle_.find(battle_.currentActor());
        if (actor->team != gameplay::Team::players) return;
        if (skipTurnBounds_.contains(position))
        {
            actionStarter_.startPass(actor->id);
            selectedAbility_.reset();
            return;
        }
        for (const auto& button : abilityButtons_)
            if (button.bounds.contains(position)) selectedAbility_ = button.ability;
        if (!selectedAbility_) return;
        for (const auto& target : battle_.validPrimaryTargets(*selectedAbility_))
        {
            if (!views_.at(target).bounds.contains(position)) continue;
            actionStarter_.start({actor->id, *selectedAbility_, target}, false);
            selectedAbility_.reset();
            return;
        }
    }

    const std::optional<gameplay::AbilityId>& BattleInteractionController::selectedAbility() const { return selectedAbility_; }
    const std::vector<AbilityButton>& BattleInteractionController::abilityButtons() const { return abilityButtons_; }
    const std::optional<gameplay::EntityId>& BattleInteractionController::inspectedCombatant() const { return inspectedCombatant_; }
}
