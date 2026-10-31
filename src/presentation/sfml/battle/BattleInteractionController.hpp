#pragma once

#include "presentation/battle/IBattleActionStarter.hpp"
#include "presentation/sfml/battle/BattleActionPresentation.hpp"
#include "presentation/sfml/app/BattleLaunchOptions.hpp"
#include "simulation/battle/EnemyPolicy.hpp"

#include <functional>

namespace soma::presentation
{
    /// Handles ability selection, target selection, passing, and automatic battle input.
    class BattleInteractionController
    {
      public:
        struct AbilityHitRegion
        {
            sf::FloatRect bounds;
            gameplay::AbilityId ability;
        };

        using AbilityHitRegionFactory = std::function<std::vector<AbilityHitRegion>(const simulation::CombatantState&)>;

        BattleInteractionController(
            const content::BattleContent& content,
            simulation::BattleSimulation& battle,
            simulation::EnemyPolicy& enemyPolicy,
            IBattleActionStarter& actionStarter,
            const BattleLaunchOptions& launchOptions,
            CombatantViews& views,
            const std::optional<PresentedAction>& action,
            sf::FloatRect skipTurnBounds,
            AbilityHitRegionFactory hitRegionFactory = {});

        void prepareTurn();
        void processEvents(sf::RenderWindow& window, const std::function<bool(const sf::Event&)>& intercept = {});
        void handlePointerClick(sf::Vector2f position);

        [[nodiscard]] const std::optional<gameplay::AbilityId>& selectedAbility() const;
        [[nodiscard]] const std::vector<AbilityButton>& abilityButtons() const;
        [[nodiscard]] const std::optional<gameplay::EntityId>& inspectedCombatant() const;
        [[nodiscard]] bool clickableAt(sf::Vector2f position) const;

      private:
        void refreshAbilityButtons(const simulation::CombatantState& actor);
        void runAutomaticTurn(const simulation::CombatantState& actor);
        void inspectAt(sf::Vector2f position);
        const content::BattleContent& content_;
        simulation::BattleSimulation& battle_;
        simulation::EnemyPolicy& enemyPolicy_;
        IBattleActionStarter& actionStarter_;
        const BattleLaunchOptions& launchOptions_;
        CombatantViews& views_;
        const std::optional<PresentedAction>& action_;
        sf::FloatRect skipTurnBounds_;
        std::optional<gameplay::AbilityId> selectedAbility_;
        std::vector<AbilityButton> abilityButtons_;
        std::vector<AbilityHitRegion> abilityHitRegions_;
        std::optional<gameplay::EntityId> inspectedCombatant_;
        std::optional<gameplay::EntityId> abilityButtonActor_;
        AbilityHitRegionFactory hitRegionFactory_;
    };
}
