#pragma once

#include "InventoryItemWrapper.hpp"
#include "GameStatus.hpp"

class ItemRowPanel
{
public:

	virtual void Init(CGameEngine *_engine);
	virtual void Update();
	virtual void Render();
	void Quit();
	void AddItem(Item _item);

	void SetPos(int _x, int _y);

protected:
	CGameEngine *engine;

	CSprite itemRowPanel;
	InventoryItemWrapper *items[5];
	ItemTooltip tooltip;

	int GetFirstFreeSlot();

	virtual void RenderItems();
	void ShowTooltipForItem(int _itemID);
};