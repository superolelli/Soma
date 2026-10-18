#pragma once

#include "content/BattleContent.hpp"
#include "presentation/sfml/BattleActionPresentation.hpp"

namespace soma::presentation
{
    /// Converts a resolved battle command into its initial staged SFML presentation state.
    class BattleActionStarter
    {
      public:
        BattleActionStarter(sf::RenderWindow& window,
                            const content::BattleContent& content,
                            simulation::BattleSimulation& battle,
                            CombatantViews& views,
                            IAnimationPlayer& effect,
                            std::optional<PresentedAction>& action,
                            std::vector<FloatingText>& floatingTexts);

        void start(simulation::BattleActionCommand command, bool announce);
        void startPass(const gameplay::EntityId& actor);

      private:
        void addSummonedCombatants(const std::vector<gameplay::GameplayEvent>& events);
        void preparePresentation(simulation::BattleActionCommand command,
                                 std::vector<gameplay::GameplayEvent> events,
                                 std::vector<simulation::CombatantState> combatantsBeforeAction,
                                 bool announce);

        sf::RenderWindow& window_;
        const content::BattleContent& content_;
        simulation::BattleSimulation& battle_;
        CombatantViews& views_;
        IAnimationPlayer& effect_;
        std::optional<PresentedAction>& action_;
        std::vector<FloatingText>& floatingTexts_;
    };
}
