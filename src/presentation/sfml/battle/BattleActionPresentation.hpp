#pragma once

#include "content/battle/BattleContent.hpp"
#include "presentation/sfml/battle/BattleScreenView.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

#include <unordered_map>
#include <vector>

namespace soma::presentation
{
    using CombatantViews = std::unordered_map<gameplay::EntityId, CombatantView>;

    void updateCombatantBounds(CombatantView& view);
    void restoreCombatants(CombatantViews& views);
    void applyPersistentAnimations(CombatantViews& views,
                                   const simulation::BattleSimulation& battle);

    [[nodiscard]] CombatantViews createCombatantViews(
        sf::RenderWindow& window,
        const content::BattleContent& content,
        const simulation::BattleSimulation& battle);

    void positionEnemyViews(CombatantViews& views,
                            const simulation::BattleSimulation& battle);
    void positionCombatantViews(CombatantViews& views,
                                const simulation::BattleSimulation& battle);

    [[nodiscard]] std::vector<AbilityButton> createAbilityButtons(
        const content::BattleContent& content,
        const simulation::CombatantState& actor);

    void moveToAbilityStage(PresentedAction& action,
                            CombatantViews& views,
                            IAnimationPlayer& effect,
                            const simulation::BattleSimulation& battle);

    void applyPresentedResults(PresentedAction& action,
                               CombatantViews& views,
                               std::vector<FloatingText>& floating);
    void applyPresentedEvents(const std::vector<gameplay::GameplayEvent>& events,
                              CombatantViews& views,
                              std::vector<FloatingText>& floating);
}
