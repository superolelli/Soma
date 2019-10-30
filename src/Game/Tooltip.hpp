#pragma once

#include "../Framework/Gameengine.hpp"
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"

class Tooltip
{
public:

	virtual void Init();

	void SetShowAboveY(bool _aboveY);
	void ShowTooltip(sf::RenderTarget &_target, int _x, int _y);

protected:
	bool showTooltipAboveY;

	sfe::RichText tooltipText;
	sf::RoundedRectangleShape tooltipBackground;

	virtual void GenerateTooltipString(std::string &_tooltip) {}
};