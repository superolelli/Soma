#pragma once

#include "Resources\FontManager.hpp"
#include "CombatantAttributes.hpp"
#include <SFML/Graphics.hpp>

class PlayerAttributesText
{
public:

	PlayerAttributesText();

	void Render(sf::RenderTarget &_target);
	void Update(CombatantAttributes const &_status);

	void SetPos(float _x, float _y);

private:
	void InitDescriptionTexts();
	void InitValueTexts();

	sf::Text statsText{SfmlCompat::defaultFont()};
	std::vector<sf::Text> statValueText = std::vector<sf::Text>(11, sf::Text(SfmlCompat::defaultFont()));
};
