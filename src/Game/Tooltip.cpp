#include "Tooltip.hpp"
#include "Resources\FontManager.hpp"

void Tooltip::Init()
{
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
	if (_stats.initiative != 0)
		_tooltip.append(_indentation + _color + _sign + std::to_string(_stats.initiative) + " Initiative\n");

	if (_stats.armour != 0)
		_tooltip.append(_indentation + _color + _sign + std::to_string(_stats.armour) + " R�stung\n");

	if (_stats.damageMax != 0)
		_tooltip.append(_indentation + _color + _sign + std::to_string(_stats.damageMax) + " Schaden\n");

	if (_stats.criticalHit != 0)
		_tooltip.append(_indentation + _color + _sign + std::to_string(_stats.criticalHit) + " Kritische Trefferchance\n");

	if (_stats.dodge != 0)
		_tooltip.append(_indentation + _color + _sign + std::to_string(_stats.dodge) + " Ausweichen\n");

	if (_stats.precision != 0)
		_tooltip.append(_indentation + _color + _sign + std::to_string(_stats.precision) + " Pr�zision\n");

	if (_stats.maxHealth != 0)
		_tooltip.append(_indentation + _color + _sign + std::to_string(_stats.maxHealth) + " Maximales Leben\n");
}
