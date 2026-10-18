#pragma once

#include "content/BattleContent.hpp"
#include "presentation/sfml/BattleScreenView.hpp"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace soma::presentation
{
    struct StatusIconInfo
    {
        const sf::Texture* texture;
        std::string tooltip;
        gameplay::StatusId status;
    };

    [[nodiscard]] std::vector<StatusIconInfo> activeStatusIcons(const BattleTextures& textures, const simulation::CombatantState& state);

    void drawHealthBar(sf::RenderWindow& window, const BattleTextures& textures, const CombatantView& view);
    [[nodiscard]] sf::Vector2f fatigueBarPosition(const BattleTextures& textures, const CombatantView& view);
    void drawFatigueBar(sf::RenderWindow& window,
                        const BattleTextures& textures,
                        const simulation::CombatantState& state,
                        const CombatantView& view);
    void drawCombatantShadow(sf::RenderWindow& window, const CombatantView& view);
    void drawStatusIcons(sf::RenderWindow& window,
                         const BattleTextures& textures,
                         const simulation::CombatantState& state,
                         const CombatantView& view,
                         const std::vector<gameplay::StatusId>& newlyGainedStatuses = {},
                         float statusAnimationElapsed = 1.0F);
    [[nodiscard]] sf::Vector2f tooltipSize(const sf::Font& font, const std::string& value, float maximumContentWidth = 500.0F);
    [[nodiscard]] sf::FloatRect drawTooltip(sf::RenderWindow& window,
                                            const sf::Font& font,
                                            const std::string& value,
                                            sf::Vector2f position,
                                            bool alignBottom = false,
                                            float maximumContentWidth = 500.0F);
    void drawCombatantInformation(sf::RenderWindow& window,
                                  const BattleTextures& textures,
                                  const sf::Font& font,
                                  const simulation::CombatantState& state,
                                  const content::CombatantDefinition& baseDefinition,
                                  const content::BattleLayoutDefinition& layout,
                                  const CombatantView& view);
    void drawExpectedAttack(sf::RenderWindow& window,
                            const sf::Font& font,
                            const simulation::BattleSimulation& battle,
                            const simulation::CombatantState& actor,
                            const simulation::CombatantState& target,
                            const content::AbilityDefinition& ability,
                            const content::BattleLayoutDefinition& layout);
} // namespace soma::presentation
