#pragma once

#include "InventoryItemWrapper.hpp"
#include "ItemTooltip.hpp"
#include "../Framework/Graphics/SpriteWithText.hpp"
#include "ConfirmationDialog.hpp"
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
	static const int SLOT_UNLOCK_COST = 200;
	static const int DICE_PRICE_REDUCE_COST = 200;
	static const int DICE_PRICE_REDUCE_STEP = 10;
	static const int MINIMUM_DICE_PRICE = 50;
	enum UnlockID {ShopSlot, DicePrice};

	CGameEngine *engine;

	CSprite shopPanel;
	CSprite selectedItemFrame;
	SpriteWithText priceSign;

	SpriteWithText shopSlotLock;
	SpriteWithText dicePriceLock;

	ConfirmationDialog *confirmationDialog;

	InventoryItemWrapper *items[16];

	ItemTooltip tooltip;

	sf::Text title;

	int currentlySelectedItem;

	std::function<void(Item &)> OnItemSelected;

	void HandleConfirmationDialog();
};