#include "ItemPanelSelectDecorator.hpp"

void ItemPanelSelectDecorator::Init(CGameEngine *_engine, ItemPanel *_itemPanel)
{
	ItemPanelDecorator::Init(_engine, _itemPanel);
	selectedItemFrame.Load(g_pTextures->selectedItemFrame);
	currentlySelectedItem = -1;
}

void ItemPanelSelectDecorator::Update()
{
	itemPanel->Update();

	if (engine->GetButtonstates(ButtonID::Left) == Pressed && itemPanel->itemPanel.GetRect().contains(engine->GetMousePos()))
	{
		for (int i = 0; i < itemPanel->items.size(); i++)
		{
			if (itemPanel->items[i] != nullptr && itemPanel->items[i]->Contains(engine->GetMousePos()))
			{
				if (currentlySelectedItem == i)
					currentlySelectedItem = -1;
				else
				{
					currentlySelectedItem = i;
					selectedItemFrame.SetPos(itemPanel->items[i]->GetGlobalBounds().left - 7, itemPanel->items[i]->GetGlobalBounds().top - 7);
					OnItemSelected(itemPanel->items[currentlySelectedItem]->GetItem());
				}
			}
		}
	}
}

void ItemPanelSelectDecorator::SetOnItemSelectedCallback(std::function<void(Item&)> _onItemSelected)
{
	OnItemSelected = _onItemSelected;
}


Item ItemPanelSelectDecorator::RetrieveCurrentlySelectedItem()
{
	auto item = CurrentlySelectedItem();

	if (currentlySelectedItem != -1)
	{
		SAFE_DELETE(itemPanel->items[currentlySelectedItem]);
		currentlySelectedItem = -1;
	}
	return item;
}

Item ItemPanelSelectDecorator::CurrentlySelectedItem()
{
	if (currentlySelectedItem != -1)
		return itemPanel->items[currentlySelectedItem]->GetItem();

	Item item;
	item.id = ItemID::empty;
	return item;
}

void ItemPanelSelectDecorator::ReduceAmountOfCurrentlySelectedItem(int _amount)
{
	if (currentlySelectedItem != -1)
	{
		itemPanel->items[currentlySelectedItem]->SetItemAmount(itemPanel->items[currentlySelectedItem]->GetItem().number - _amount);
		if (itemPanel->items[currentlySelectedItem]->GetItem().number <= 0)
		{
			SAFE_DELETE(itemPanel->items[currentlySelectedItem]);
			currentlySelectedItem = -1;
		}
	}
}


bool ItemPanelSelectDecorator::IsItemSelected()
{
	return currentlySelectedItem != -1;
}

int ItemPanelSelectDecorator::CurrentItemPrice()
{
	if (currentlySelectedItem != -1)
		return g_pObjectProperties->getItemStats(itemPanel->items[currentlySelectedItem]->GetItem().id).price;
	else
		return 0;
}


void ItemPanelSelectDecorator::Render()
{
	itemPanel->Render();
	if (currentlySelectedItem != -1)
		selectedItemFrame.Render(engine->GetWindow());
}