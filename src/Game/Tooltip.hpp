#pragma once

#include "../Framework/Gameengine.hpp"
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"

class Tooltip
{
public:

	virtual void Init(CGameEngine *_engine);

	void SetShowAboveY(bool _aboveY);
	void ShowTooltip(int _x, int _y);

protected:
	CGameEngine *engine;

	bool showTooltipAboveY;

	sfe::RichText tooltipText;
	sf::RoundedRectangleShape tooltipBackground;

	virtual void GenerateTooltipString(std::string &_tooltip) {}
};