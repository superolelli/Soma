#pragma once

#include "GameStatus.hpp"
#include "../Framework/Gui/Button.hpp"
#include "../Framework/Gui/Scrollbar.hpp"
#include "ItemTooltip.hpp"
#include "EquipmentPanel.hpp"
#include "InventoryItemWrapper.hpp"



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
	int currentUpperRow;

	CSprite inventoryPanel;
	EquipmentPanel equipmentPanel;
	VerticalScrollbar scrollbar;
	sf::Text currentPlayerName;
	sf::Text panelTitle;
	CButton buttonNext;
	CButton buttonPrevious;
	CButton buttonClose;

	std::vector<InventoryItemWrapper*> items;
	ItemTooltip tooltip;

	int currentDraggedItemOldX;
	int currentDraggedItemOldY;
	int currentDraggedItem;

	void CheckButtonsForPlayerChoosing();
	void UpdateGUIForChosenPlayer();
	void UpdateScrollbar();

	void HandleDragAndDrop();
	void HandleStartedDrag();
	void HandleContinuedDrag();
	void HandleDrop();

	void PlaceCurrentDraggedItemAsEquipment();
	void OnItemAdded(Item _item);
	void RenderItems();
	void RecalculatePositionsOfItems();

	int GetFirstFreeInventorySlot();
};