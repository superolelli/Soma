#pragma once

#include "GameStatus.hpp"
#include "../Framework/Gui/Button.hpp"
#include "../Framework/Gui/Scrollbar.hpp"
#include "ItemTooltip.hpp"
#include "InventoryItemWrapper.hpp"


class ScrollableItemPanel
{
public:

	virtual void Init(CGameEngine *_engine);
	virtual void Update();
	void Render();
	void Quit();

	void SetPos(int _x, int _y);

	void AddItem(Item _item);

protected:
	CGameEngine *engine;

	int currentUpperRow;

	CSprite scrollableItemPanel;
	VerticalScrollbar scrollbar;
	std::vector<InventoryItemWrapper*> items;
	ItemTooltip tooltip;

	void UpdateScrollbar();

	virtual void RenderItems();

	void RecalculatePositionsOfItems();

	int GetFirstFreeItemSlot();

	void ShowTooltipForItem(ItemID _itemID);
};