#include "Tooltip.hpp"
#include "Resources\FontManager.hpp"
#include <deque>

Tooltip::Tooltip(bool _showAboveY)
	: showTooltipAboveY(_showAboveY)
{
	tooltipText.setCharacterSize(18);
	tooltipText.setFont(g_pFonts->f_arial);

	tooltipBackground.setFillColor(sf::Color(0, 0, 0, 220));
	tooltipBackground.setOutlineThickness(2.0f);
	tooltipBackground.setOutlineColor(sf::Color(40, 40, 40));
	tooltipBackground.setCornerPointCount(20);
	tooltipBackground.setCornersRadius(8);
}


void Tooltip::ShowTooltip(sf::RenderTarget &_target, int _x, int _y)
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

	_target.draw(tooltipBackground);
	_target.draw(tooltipText);
}


void Tooltip::AppendCombatantAttributesTooltip(std::string& _tooltip, CombatantAttributes& _stats, std::string _indentation, std::string _sign, std::string _color)
{
	std::deque<std::string> toShow = { "damageMin", "damageMax", "armour", "maxHealth", "initiative", "criticalHit", "dodge", "precision", 
									   "healing", "sleepResistance", "confusionResistance", "debuffResistance", "decayResistance"};

	if (_stats["damageMin"] != 0 && _stats["damageMin"] == _stats["damageMax"])
	{
		_tooltip.append(_indentation + _color + _sign + std::to_string(_stats["damageMin"]) + " " + "Schaden" + "\n");
		toShow.pop_front();
		toShow.pop_front();
	}


	for (auto& s : toShow)
	{
		if (_stats[s] != 0)
			_tooltip.append(_indentation + _color + _sign + std::to_string(_stats[s]) + " " + CombatantAttributes::attributeDisplayNames.at(s) + "\n");
	}
}
