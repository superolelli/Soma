#pragma once

#include "content/battle/BattleContent.hpp"
#include "presentation/battle/IBattleActionStarter.hpp"
#include "presentation/sfml/battle/BattleActionPresentation.hpp"

namespace soma::presentation
{
    /// Converts a resolved battle command into its initial staged SFML presentation state.
    class BattleActionStarter final : public IBattleActionStarter
    {
      public:
        BattleActionStarter(sf::RenderWindow& window,
                            const content::BattleContent& content,
                            simulation::BattleSimulation& battle,
                            CombatantViews& views,
                            IAnimationPlayer& effect,
                            std::optional<PresentedAction>& action,
                            std::vector<FloatingText>& floatingTexts);

        void start(simulation::BattleActionCommand command, bool announce) override;
        void startPass(const gameplay::EntityId& actor) override;

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
