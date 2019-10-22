#pragma once
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include "../Framework/Graphics/Sprite.hpp"
#include "../Framework/Gameengine.hpp"
#include "GameStatus.hpp"
#include "Tooltip.hpp"


class ItemTooltip : public Tooltip
{
public:

	void Init(CGameEngine *_engine) override;

	void SetItemID(ItemID _id);

private:
	ItemID itemID;

	void GenerateTooltipString(std::string &_tooltip) override;
};