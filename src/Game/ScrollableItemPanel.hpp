#pragma once

#include "ItemPanel.hpp"
#include "../Framework/Gui/Button.hpp"
#include "../Framework/Gui/Scrollbar.hpp"
#include "ItemTooltip.hpp"


class ScrollableItemPanel : public ItemPanel
{
public:
	void Init(CGameEngine *_engine) override;
	void Update() override;
	void Render(int _excludeItemNumber = -1) override;
	void SetPos(int _x, int _y) override;
	void AddItem(Item _item) override;

private:
	int currentUpperRow;
	VerticalScrollbar scrollbar;

	void UpdateScrollbar();
	void RecalculatePositionsOfItems();
};