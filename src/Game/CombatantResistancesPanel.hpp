#pragma once

#include "Resources\FontManager.hpp"
#include "CombatantStatus.hpp"

class CombatantResistancesPanel
{
public:

	CombatantResistancesPanel();

	void Render(sf::RenderTarget& _target);
	void Update(CombatantStatus& _status);

	void SetPos(float _x, float _y);

	sf::FloatRect GetRect();

private:

	void SetValueTextColor(sf::Text& text, int buffValue, int debuffValue);

	sf::Text title;
	sf::Text resistancesText[4];
};