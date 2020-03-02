#pragma once

#include "InventoryItemWrapper.hpp"
#include "GameStatus.hpp"

class ItemRowPanel
{
public:

	void Init(CGameEngine *_engine, std::function<InventoryItemWrapper*(InventoryItemWrapper*)> _onItemDropped);
	void Update();
	void Render();
	void RenderCurrentlyDraggedItem();
	void Quit();
	void AddItem(Item _item);

	void SetPos(int _x, int _y);

private:
	CGameEngine *engine;

	CSprite itemRowPanel;
	InventoryItemWrapper *items[5];
	ItemTooltip tooltip;

	int currentDraggedItem;
	int currentDraggedItemOldX;
	int currentDraggedItemOldY;

	std::function<InventoryItemWrapper*(InventoryItemWrapper*)> OnItemDropped;

	int GetFirstFreeSlot();

	void HandleStartedDrag();
	void HandleContinuedDrag();
	void HandleDrop();
};