#pragma once

#include "InventoryItemWrapper.hpp"

class ItemPanel
{
	friend class ItemPanelSelectDecorator;
	friend class ItemPanelDragAndDropDecorator;

public:

	virtual void Init(CGameEngine *_engine);
	virtual void Update() = 0;
	virtual void Render(int _excludeItemNumber = -1) = 0;
	virtual void AddItem(Item _item) = 0;

	virtual void SetPos(int _x, int _y) = 0;
	virtual void Clear();

	virtual void SortItemsAccordingToColor();
	virtual void SortItemsAccordingToNames();

	sf::IntRect const &GetRect() { return itemPanel.GetRect(); }

protected:
	CGameEngine *engine;

	CSprite itemPanel;
	std::vector<InventoryItemWrapper*> items;
	ItemTooltip tooltip;

	int GetFirstFreeSlot();

	void RenderItems(int _start, int _end, int _excludeItemNumber = -1);
	void ShowTooltipForItem(int _itemID);
};