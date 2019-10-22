#include "Tooltip.hpp"
#include "Resources\FontManager.hpp"

void Tooltip::Init(CGameEngine * _engine)
{
	engine = _engine;

	tooltipText.setCharacterSize(18);
	tooltipText.setFont(g_pFonts->f_arial);

	tooltipBackground.setFillColor(sf::Color(0, 0, 0, 220));
	tooltipBackground.setOutlineThickness(2.0f);
	tooltipBackground.setOutlineColor(sf::Color(40, 40, 40));
	tooltipBackground.setCornerPointCount(20);
	tooltipBackground.setCornersRadius(8);

	showTooltipAboveY = true;
}


void Tooltip::SetShowAboveY(bool _aboveY)
{
	showTooltipAboveY = _aboveY;
}


void Tooltip::ShowTooltip(int _x, int _y)
{
	std::string tooltipString("");
	GenerateTooltipString(tooltipString);
	tooltipText.setString(tooltipString);

	sf::FloatRect backgroundRect = tooltipText.getLocalBounds();
	tooltipBackground.setSize(sf::Vector2f(backgroundRect.width + 20.0f, backgroundRect.height + 20.0f));

	if (showTooltipAboveY)
		tooltipBackground.setPosition(_x, _y - tooltipText.getLocalBounds().height);
	else
		tooltipBackground.setPosition(_x, _y);

	tooltipText.setPosition(tooltipBackground.getGlobalBounds().left + 10, tooltipBackground.getGlobalBounds().top + 7);

	engine->GetWindow().draw(tooltipBackground);
	engine->GetWindow().draw(tooltipText);
}