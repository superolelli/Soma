#pragma once

#include "../Framework/Gameengine.hpp"
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include "CombatantAttributes.hpp"

class Tooltip
{
public:

	Tooltip(bool _showAboveY=true);

	void ShowTooltip(sf::RenderTarget &_target, int _x, int _y);

protected:
	bool showTooltipAboveY;

	sfe::RichText tooltipText;
	sf::RoundedRectangleShape tooltipBackground;

	virtual void GenerateTooltipString(std::string &_tooltip) {}
	void AppendCombatantAttributesTooltip(std::string& _tooltip, const CombatantAttributes& _stats, const std::string &_indentation = "", const std::string &_sign = "", const std::string &_color="#aaaadd ");
	inline const std::string DurationString(int duration) { return duration == 1 ? "1 Runde" : std::to_string(duration) + " Runden"; }
};