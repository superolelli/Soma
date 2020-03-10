#pragma once

#include "GameStatus.hpp"
#include "../Framework/Gui/Button.hpp"
#include "ScrollableItemPanelSelect.hpp"
#include "ItemRowPanelSelect.hpp"
#include "ShopPanel.hpp"
#include "SellMultipleDialog.hpp"


class VendingMachinePanel
{
public:

	void Init(GameStatus *_gameStatus, CGameEngine *_engine);
	void Update();
	void Render();
	void Quit();

	void Open();
	bool IsOpen() { return !closed; }

private:
	GameStatus *gameStatus;
	CGameEngine *engine;

	CSprite vendingMachinePanel;
	ScrollableItemPanelSelect scrollableItemPanel;
	ItemRowPanelSelect itemRowPanel;
	ShopPanel shopPanel;

	SellMultipleDialog *dialog;

	CButton buttonBuy;
	CButton buttonClose;

	sf::Text title;

	sf::Text priceText;
	CSprite cardsSymbol;

	bool closed;

	void OnItemOfRowPanelSelected(Item &_item);
	void OnItemOfScrollablePanelSelected(Item &_item);
	void OnItemOfShopPanelSelected(Item &_item);

	void SellItem(Item &_item, int _amount = 1);

	void HandleDialog();
	void BuyCurrentItemFromShop();
	void OpenSellMultipleDialog();
};