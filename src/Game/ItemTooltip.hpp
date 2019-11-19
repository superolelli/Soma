#pragma once
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include "../Framework/Graphics/Sprite.hpp"
#include "../Framework/Gameengine.hpp"
#include "GameStatus.hpp"
#include "Tooltip.hpp"
#include "Resources\ObjectPropertiesManager.hpp"


class ItemTooltip : public Tooltip
{
public:

	void Init() override;

	void SetItem(ItemProperties *_itemProperties);

private:
	ItemProperties *itemProperties;

	void GenerateTooltipString(std::string &_tooltip) override;
};