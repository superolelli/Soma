#pragma once

#include "ScrollableItemPanel.hpp"

class ScrollableItemPanelSelect : public ScrollableItemPanel
{
public:

	void Init(CGameEngine *_engine) override;
	void Update() override;
	void SetOnItemSelectedCallback(std::function<void(Item &)> _onItemSelected);

	void UnselectSelectedItem() { currentlySelectedItem = -1; }

	Item RetrieveCurrentlySelectedItem();
	bool IsItemSelected();
	int CurrentItemPrice();

private:

	void RenderItems() override;

	std::function<void(Item &)> OnItemSelected;

	CSprite selectedItemFrame;
	int currentlySelectedItem;
};