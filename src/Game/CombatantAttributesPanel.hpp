#pragma once

#include "Resources\FontManager.hpp"
#include "CombatantStatus.hpp"
#include <SFML/Graphics.hpp>
#include "../Framework/Graphics/RoundedRectangleShape.hpp"

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

	void SetValueTextColor(sf::Text &text, int buffValue, int debuffValue, bool _attribute);

	sf::Text attributeText;
	sf::Text statsText;

	sf::Text attributeValueText[4];
	sf::Text statValueText[6];

	sf::RoundedRectangleShape background;
};