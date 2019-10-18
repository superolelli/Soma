#pragma once
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include "../Framework/Graphics/Sprite.hpp"
#include "../Framework/Gameengine.hpp"
#include "GameStatus.hpp"


class ItemTooltip
{
public:

	void Init(CGameEngine *_engine);

	void SetItemID(ItemID _id);
	void ShowTooltip(int _x, int _y);

private:
	CGameEngine *engine;
	ItemID itemID;

	sfe::RichText tooltipText;
	sf::RoundedRectangleShape tooltipBackground;

	void GenerateTooltipString(std::string &_tooltip);
};