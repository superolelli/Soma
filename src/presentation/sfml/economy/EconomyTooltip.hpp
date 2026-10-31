#pragma once

#include "gameplay/items/ItemTypes.hpp"
#include "content/rules/RuleContent.hpp"

#include <SFML/Graphics.hpp>

namespace soma::presentation
{
    void drawItemTooltip(sf::RenderWindow& window,
                         const sf::Font& font,
                         const gameplay::ItemDefinition& definition,
                         sf::Vector2f mousePosition);
    void drawBonusTooltip(sf::RenderWindow& window,
                          const sf::Font& font,
                          std::string_view title,
                          const gameplay::AttributeBonuses& bonuses,
                          sf::Vector2f mousePosition);
    void drawRuleFragmentTooltip(sf::RenderWindow& window,
                                 const sf::Font& font,
                                 const content::RuleFragmentDefinition& definition,
                                 sf::Vector2f mousePosition);
}
