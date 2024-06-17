#pragma once

#include "ItemPanelDecorator.hpp"

class ItemPanelSelectDecorator : public ItemPanelDecorator
{
public:

	ItemPanelSelectDecorator(CGameEngine* _engine, ItemPanel* _itemPanel);

	void Update() override;
	void Render() override;
	void SetOnItemSelectedCallback(std::function<void(Item &)> _onItemSelected);


	void UnselectSelectedItem() { currentlySelectedItem = -1; }
	Item RetrieveCurrentlySelectedItem();
	void ReduceAmountOfCurrentlySelectedItem(int _amount);
	Item CurrentlySelectedItem();
	bool IsItemSelected();
	int CurrentItemPrice();

private:
	std::function<void(Item &)> OnItemSelected;

	CSprite selectedItemFrame;
	int currentlySelectedItem;
};