#pragma once

#include "../Framework/Graphics/Sprite.hpp"
#include "../Framework/Gameengine.hpp"
#include "GameStatus.hpp"
#include "InventoryItemWrapper.hpp"
#include "EquipmentConnections.hpp"

class EquipmentPanel
{
public:

	void Init(CGameEngine *_engine, GameStatus *_gameStatus, int _xPos, int _yPos);
	void Update();
	void Render();
	void Quit();

	void SetCurrentPlayer(int _currentPlayer);
	void SetInventoryRect(sf::IntRect &_inventoryRect);

	InventoryItemWrapper *PlaceItem(InventoryItemWrapper *_item);
	bool CanBePlaced(CSprite &_itemSprite);

private:

	CGameEngine *engine;
	GameStatus *gameStatus;

	CSprite equipmentField[4];
	CSprite connectionsBackground;
	CSprite diamond;

	EquipmentConnections connections[4];

	InventoryItemWrapper *items[4][4];
	ItemTooltip tooltip;

	sf::IntRect inventoryRect;

	sf::Color oldDiamondColor;
	sf::Color newDiamondColor;

	int currentDraggedItemOldX;
	int currentDraggedItemOldY;
	int currentDraggedItem;

	int currentPlayer;

	float diamondUpdateTime;

	void HandleDragAndDrop();
	void HandleDragStarted();
	void HandleContinuedDrag();
	void HandleDrop();

	void PlaceCurrentDraggedItemIntoInventory();

	void RenderItems();

	void CheckConnections(int _slot);

	int LeftNeighbourSlot(int _slot);
	int RightNeighbourSlot(int _slot);

	void UpdateDiamond();
	void RecolorDiamond();
};