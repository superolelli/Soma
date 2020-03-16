#pragma once

#include "InventoryItemWrapper.hpp"
#include "ItemTooltip.hpp"
#include "../Framework/Graphics/SpriteWithText.hpp"
#include <unordered_map>

class ShopPanel
{
public:

	void Init(CGameEngine *_engine, std::unordered_map<ItemID, bool> *_consumablesAvailability);
	void Update();
	void Render();
	void Quit();

	void SetOnItemSelectedCallback(std::function<void(Item &)> _onItemSelected);

	void SetPos(int _x, int _y);
	sf::IntRect GetRect() { return shopPanel.GetGlobalRect(); }

	void UnselectSelectedItem() { currentlySelectedItem = -1; }
	bool IsItemSelected();

	void ChooseNewRandomItems(int _bangLevel, int _kutschfahrtLevel, int _tichuLevel);

	Item RetrieveCurrentlySelectedItem();
	int CurrentItemPrice();

private:
	CGameEngine *engine;

	CSprite shopPanel;
	CSprite selectedItemFrame;
	SpriteWithText priceSign;

	InventoryItemWrapper *items[15];

	ItemTooltip tooltip;

	sf::Text title;

	int currentlySelectedItem;

	std::unordered_map<ItemID, bool> *consumablesAvailability;

	std::function<void(Item &)> OnItemSelected;
};