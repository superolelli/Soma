#include "ItemRowPanelSelect.hpp"

void ItemRowPanelSelect::Init(CGameEngine *_engine)
{
	ItemRowPanel::Init(_engine);
	selectedItemFrame.Load(g_pTextures->selectedItemFrame);
	currentlySelectedItem = -1;
}


void ItemRowPanelSelect::Update()
{
	if (engine->GetButtonstates(ButtonID::Left) == Pressed)
	{
		for (int i = 0; i < 5; i++)
		{
			if (items[i] != nullptr && items[i]->Contains(engine->GetMousePos()))
			{
				if (currentlySelectedItem == i)
					currentlySelectedItem = -1;
				else
				{
					currentlySelectedItem = i;
					selectedItemFrame.SetPos(items[i]->GetGlobalBounds().left - 7, items[i]->GetGlobalBounds().top - 7);
					OnItemSelected(items[currentlySelectedItem]->GetItem());
				}
			}
		}
	}
}

void ItemRowPanelSelect::SetOnItemSelectedCallback(std::function<void(Item&)> _onItemSelected)
{
	OnItemSelected = _onItemSelected;
}


Item ItemRowPanelSelect::RetrieveCurrentlySelectedItem()
{
	auto item = CurrentlySelectedItem();

	if (currentlySelectedItem != -1)
	{
		SAFE_DELETE(items[currentlySelectedItem]);
		currentlySelectedItem = -1;
	}
	return item;
}

Item &ItemRowPanelSelect::CurrentlySelectedItem()
{
	if (currentlySelectedItem != -1)
		return items[currentlySelectedItem]->GetItem();

	Item item;
	item.id = ItemID::empty;
	return item;
}

void ItemRowPanelSelect::ReduceAmountOfCurrentlySelectedItem(int _amount)
{
	if (currentlySelectedItem != -1)
	{
		items[currentlySelectedItem]->SetItemAmount(items[currentlySelectedItem]->GetItem().number - _amount);
		if (items[currentlySelectedItem]->GetItem().number <= 0)
		{
			SAFE_DELETE(items[currentlySelectedItem]);
			currentlySelectedItem = -1;
		}
	}
}


bool ItemRowPanelSelect::IsItemSelected()
{
	return currentlySelectedItem != -1;
}

int ItemRowPanelSelect::CurrentItemPrice()
{
	if (currentlySelectedItem != -1)
		return g_pObjectProperties->getItemStats(items[currentlySelectedItem]->GetItem().id).price;
	else
		return 0;
}

void ItemRowPanelSelect::RenderItems()
{
	int showTooltipForItem = -1;
	for (int i = 0; i < 5; i++)
	{
		if (items[i] != nullptr)
		{
			items[i]->Render(engine->GetWindow());

			if (currentlySelectedItem == i)
				selectedItemFrame.Render(engine->GetWindow());

			if (items[i]->Contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) != Held)
				showTooltipForItem = i;
		}
	}

	ShowTooltipForItem(showTooltipForItem);
}
