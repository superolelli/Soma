#pragma once

#include "../Framework/Graphics/Sprite.hpp"
#include "../Framework/Gameengine.hpp"
#include "GameStatus.hpp"
#include "InventoryItemWrapper.hpp"
#include "EquipmentPanelDiamond.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

class EquipmentPanel
{
public:

	void Init(CGameEngine *_engine, GameStatus *_gameStatus, int _xPos, int _yPos);
	void Update();
	void Render();
	void RenderCurrentDraggedItem();
	void Quit();

	void SetOnItemDroppedCallback(std::function<InventoryItemWrapper*(InventoryItemWrapper*, int, int)> _onItemDropped);

	void SetCurrentPlayer(int _currentPlayer);

	InventoryItemWrapper *PlaceItem(InventoryItemWrapper *_item);
	bool CanBePlaced(CSprite &_itemSprite);

	void Clear();
	void AddItem(Item &_item, int player, int slot);

private:

	CGameEngine *engine;
	GameStatus *gameStatus;

	CSprite equipmentField[4];
	CSprite connectionsBackground;

	EquipmentConnections connections[4][4];
	EquipmentPanelDiamond diamond[4];

	InventoryItemWrapper *items[4][4];
	ItemTooltip tooltip;

	int currentDraggedItemOldX;
	int currentDraggedItemOldY;
	int currentDraggedItem;

	int currentPlayer;

	std::function<InventoryItemWrapper*(InventoryItemWrapper*, int, int)> OnItemDropped;

	void HandleDragAndDrop();
	void HandleDragStarted();
	void HandleContinuedDrag();
	void HandleDrop();

	void RenderItems();

	void CheckConnections(int _slot);

	int LeftNeighbourSlot(int _slot);
	int RightNeighbourSlot(int _slot);
};