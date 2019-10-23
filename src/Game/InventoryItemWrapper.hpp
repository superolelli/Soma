#pragma once

#include "GameStatus.hpp"
#include "ItemTooltip.hpp"

class InventoryItemWrapper
{
public:

	void Init(Item &&_item, CSprite &&_sprite);
	void Render(sf::RenderTarget &_target);

	void SetPos(int _x, int _y);
	void SetCenterPos(int _x, int _y);

	sf::IntRect &GetGlobalBounds();
	Item &GetItem();
	CSprite &GetSprite();

	bool Contains(const sf::Vector2i &_point);

private:

	Item item;
	CSprite sprite;
};