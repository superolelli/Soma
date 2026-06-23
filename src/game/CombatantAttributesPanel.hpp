#pragma once

#include "Resources\FontManager.hpp"
#include "CombatantStatus.hpp"
#include <SFML/Graphics.hpp>
#include "../engine/Graphics/RoundedRectangleShape.hpp"

class CombatantAttributesPanel
{
public:

	CombatantAttributesPanel();

	void Render(sf::RenderTarget &_target);
	void Update(CombatantStatus &_status);

	void SetPos(float _x, float _y);

	sf::FloatRect GetRect() { return background.getGlobalBounds(); }

private:
	void InitDescriptionTexts();
	void InitValueTexts();

	void SetValueTextColor(sf::Text &text, int buffValue, int debuffValue);

	sf::Text statsText{SfmlCompat::defaultFont()};
	std::vector<sf::Text> statValueText = std::vector<sf::Text>(6, sf::Text(SfmlCompat::defaultFont()));

	sf::RoundedRectangleShape background;
};
