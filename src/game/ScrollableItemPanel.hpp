#pragma once

#include "ItemPanel.hpp"
#include "../engine/Gui/Button.hpp"
#include "../engine/Gui/Scrollbar.hpp"
#include "ItemTooltip.hpp"


class ScrollableItemPanel : public ItemPanel
{
public:

	ScrollableItemPanel(CGameEngine* _engine);

	void Update() override;
	void Render(int _excludeItemNumber = -1) override;
	void SetPos(int _x, int _y) override;
	void AddItem(Item _item) override;

	void SortItemsAccordingToColor() override;
	void SortItemsAccordingToNames() override;

private:
	int currentUpperRow;
	VerticalScrollbar scrollbar;

	void UpdateScrollbar();
	void RecalculatePositionsOfItems();
};