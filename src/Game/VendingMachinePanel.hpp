#pragma once

#include "GameStatus.hpp"
#include "../Framework/Gui/Button.hpp"
#include "ScrollableItemPanel.hpp"
#include "ItemRowPanel.hpp"
#include "ItemPanelSelectDecorator.hpp"
#include "ShopPanel.hpp"
#include "SellMultipleDialog.hpp"


class VendingMachinePanel
{
public:

	VendingMachinePanel(CGameEngine* _engine);
	void Update();
	void Render();

	void Open();
	void Close() { closed = true; }
	bool IsOpen() { return !closed; }

	void ChooseNewShopItems();

private:
	CGameEngine *engine;

	CSprite vendingMachinePanel;
	ItemPanelSelectDecorator scrollableItemPanel;
	ItemPanelSelectDecorator itemRowPanel;
	ShopPanel shopPanel;

	SellMultipleDialog *dialog;

	CButton buttonSortNames;
	CButton buttonSortColors;
	CButton buttonBuy;
	CButton buttonClose;

	sf::Text title;

	sf::Text priceText;
	CSprite cardsSymbol;

	bool closed;

	void OnItemOfRowPanelSelected(Item &_item);
	void OnItemOfScrollablePanelSelected(Item &_item);
	void OnItemOfShopPanelSelected(Item &_item);

	void SellItem(Item const &_item, int _amount = 1);

	void HandleDialog();
	void BuyCurrentItemFromShop();
	void OpenSellMultipleDialog();
};