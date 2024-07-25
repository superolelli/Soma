#pragma once

#include "InventoryItemWrapper.hpp"
#include "ItemTooltip.hpp"
#include "../Framework/Graphics/SpriteWithText.hpp"
#include <unordered_map>

class ShopPanel
{
public:

	ShopPanel(CGameEngine* _engine);
	~ShopPanel();
	void Update();
	void Render();

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

	InventoryItemWrapper *items[16];

	ItemTooltip tooltip;

	sf::Text title;

	int currentlySelectedItem;

	std::function<void(Item &)> OnItemSelected;
};