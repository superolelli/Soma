#pragma once

#include "InventoryItemWrapper.hpp"
#include "GameStatus.hpp"
#include "AdventureGroup.hpp"

class ConsumablePanel
{
public:

	void Init(CGameEngine *_engine, GameStatus *_gameStatus, AdventureGroup *_adventureGroup);
	void Update();
	void Render();
	void Quit();

	void SetPos(int _x, int _y);
	void SetAdventureGroup(AdventureGroup *_adventureGroup) { adventureGroup = _adventureGroup; }

	bool IsOpen() { return isOpen; }

private:
	CGameEngine *engine;
	GameStatus *gameStatus;
	AdventureGroup *adventureGroup;

	bool isOpen;

	CSprite consumablePanel;
	InventoryItemWrapper *items[5];
	ItemTooltip tooltip;

	int currentDraggedItem;
	int currentDraggedItemOldX;
	int currentDraggedItemOldY;

	void OnItemAdded(Item _item, bool _onlyAmountChanged);
	int GetFirstFreeSlot();

	void HandleStartedDrag();
	void HandleContinuedDrag();
	void HandleDrop();
};