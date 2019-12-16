#pragma once

#include "InventoryItemWrapper.hpp"
#include "GameStatus.hpp"

class ConsumablePanel
{
public:

	void Init(CGameEngine *_engine, GameStatus *_gameStatus);
	void Update();
	void Render();
	void Quit();

	void SetPos(int _x, int _y);

	bool IsOpen() { return isOpen; }

private:
	CGameEngine *engine;
	GameStatus *gameStatus;

	bool isOpen;

	CSprite consumablePanel;
	InventoryItemWrapper *items[5];
	ItemTooltip tooltip;

	void OnItemAdded(Item _item, bool _onlyAmountChanged);
	int GetFirstFreeSlot();
};