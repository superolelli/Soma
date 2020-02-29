#pragma once

#include "GameStatus.hpp"
#include "../Framework/Gui/Button.hpp"
#include "ScrollableItemPanel.hpp"
#include "ItemRowPanel.hpp"



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
	ScrollableItemPanel scrollableItemPanel;
	ItemRowPanel itemRowPanel;
	CButton buttonClose;

	bool closed;

	InventoryItemWrapper* OnItemFromItemPanelReceived(InventoryItemWrapper* _receivedItem);
	InventoryItemWrapper* OnItemFromConsumablePanelReceived(InventoryItemWrapper* _receivedItem);
};