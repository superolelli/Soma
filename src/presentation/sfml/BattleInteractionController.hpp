#pragma once

#include "presentation/sfml/BattleActionStarter.hpp"
#include "presentation/sfml/BattleLaunchOptions.hpp"
#include "simulation/battle/EnemyPolicy.hpp"

#include <functional>

namespace soma::presentation
{
    /// Handles ability selection, target selection, passing, and automatic battle input.
    class BattleInteractionController
    {
      public:
        BattleInteractionController(
            sf::RenderWindow& window,
            const content::BattleContent& content,
            simulation::BattleSimulation& battle,
            simulation::EnemyPolicy& enemyPolicy,
            BattleActionStarter& actionStarter,
            const BattleLaunchOptions& launchOptions,
            CombatantViews& views,
            const std::optional<PresentedAction>& action,
            sf::FloatRect skipTurnBounds);

        void prepareTurn();
        void processEvents(const std::function<bool(const sf::Event&)>& intercept = {});

        [[nodiscard]] const std::optional<gameplay::AbilityId>& selectedAbility() const;
        [[nodiscard]] const std::vector<AbilityButton>& abilityButtons() const;
        [[nodiscard]] const std::optional<gameplay::EntityId>& inspectedCombatant() const;
        [[nodiscard]] bool clickableAt(sf::Vector2f position) const;

      private:
        void refreshAbilityButtons(const simulation::CombatantState& actor);
        void runAutomaticTurn(const simulation::CombatantState& actor);
        void inspectAt(sf::Vector2f position);
        void handleClick(sf::Vector2f position);

        sf::RenderWindow& window_;
        const content::BattleContent& content_;
        simulation::BattleSimulation& battle_;
        simulation::EnemyPolicy& enemyPolicy_;
        BattleActionStarter& actionStarter_;
        const BattleLaunchOptions& launchOptions_;
        CombatantViews& views_;
        const std::optional<PresentedAction>& action_;
        sf::FloatRect skipTurnBounds_;
        std::optional<gameplay::AbilityId> selectedAbility_;
        std::vector<AbilityButton> abilityButtons_;
        std::optional<gameplay::EntityId> inspectedCombatant_;
        std::optional<gameplay::EntityId> abilityButtonActor_;
    };
}
