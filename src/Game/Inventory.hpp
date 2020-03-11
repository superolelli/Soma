#pragma once

#include "GameStatus.hpp"
#include "../Framework/Gui/Button.hpp"
#include "EquipmentPanel.hpp"
#include "InventoryItemWrapper.hpp"

#include "ScrollableItemPanel.hpp"
#include "ItemPanelDragAndDropDecorator.hpp"



class Inventory
{
public:

	void Init(GameStatus *_gameStatus, CGameEngine *_engine);
	void Update();
	void Render();
	void Quit();

	void Open(int _player = 0);
	bool IsOpen() { return !closed; }

private:
	GameStatus *gameStatus;
	CGameEngine *engine;

	bool closed;
	int currentPlayer;

	CSprite inventoryPanel;
	EquipmentPanel equipmentPanel;
	ItemPanelDragAndDropDecorator scrollableItemPanel;
	sf::Text currentPlayerName;
	sf::Text panelTitle;
	CButton buttonNext;
	CButton buttonPrevious;
	CButton buttonClose;


	void CheckButtonsForPlayerChoosing();
	void UpdateGUIForChosenPlayer();

	void OnItemAdded(Item _item);

	InventoryItemWrapper* OnItemFromItemPanelReceived(InventoryItemWrapper* _receivedItem);
	InventoryItemWrapper* OnItemFromEquipmentPanelReceived(InventoryItemWrapper *_receivedItem, int _currentPlayer, int _equipmentId);
};