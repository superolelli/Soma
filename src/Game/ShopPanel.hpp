#pragma once

#include "InventoryItemWrapper.hpp"
#include "ItemTooltip.hpp"
#include <unordered_map>

class ShopPanel
{
public:

	void Init(CGameEngine *_engine, std::unordered_map<ItemID, bool> &_consumablesAvailability);
	void Update();
	void Render();
	void Quit();

	void SetPos(int _x, int _y);

	void ChooseNewRandomItems(int _level);

private:
	CGameEngine *engine;

	CSprite shopPanel;
	CSprite selectedItemFrame;

	InventoryItemWrapper *items[15];

	ItemTooltip tooltip;

	sf::Text title;

	int currentlySelectedItem;

	std::unordered_map<ItemID, bool> consumablesAvailability;

	sf::Color GetRandomItemColor();
	ItemID GetRandomItemID(int _level);
};