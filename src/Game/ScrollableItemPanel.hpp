#pragma once

#include "GameStatus.hpp"
#include "../Framework/Gui/Button.hpp"
#include "../Framework/Gui/Scrollbar.hpp"
#include "ItemTooltip.hpp"
#include "InventoryItemWrapper.hpp"


class ScrollableItemPanel
{
public:

	void Init(CGameEngine *_engine, std::function<InventoryItemWrapper*(InventoryItemWrapper*)> _onItemDropped);
	void Update();
	void Render();
	void RenderCurrentlyDraggedItem();
	void Quit();

	void SetPos(int _x, int _y);

	void AddItem(Item _item);

private:
	CGameEngine *engine;

	int currentUpperRow;

	CSprite scrollableItemPanel;
	VerticalScrollbar scrollbar;

	std::vector<InventoryItemWrapper*> items;
	ItemTooltip tooltip;

	std::function<InventoryItemWrapper*(InventoryItemWrapper*)> OnItemDropped;

	int currentDraggedItemOldX;
	int currentDraggedItemOldY;
	int currentDraggedItem;

	void UpdateScrollbar();

	void HandleDragAndDrop();
	void HandleStartedDrag();
	void HandleContinuedDrag();
	void HandleDrop();

	void RenderItems();

	void RecalculatePositionsOfItems();

	int GetFirstFreeItemSlot();
};