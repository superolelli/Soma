#include "ScrollableItemPanelSelect.hpp"

void ScrollableItemPanelSelect::Init(CGameEngine *_engine)
{
	ScrollableItemPanel::Init(_engine);
	selectedItemFrame.Load(g_pTextures->selectedItemFrame);
	currentlySelectedItem = -1;
}


void ScrollableItemPanelSelect::Update()
{
	if (engine->GetButtonstates(ButtonID::Left) == Pressed)
	{
		for (int i = currentUpperRow * 5; i < currentUpperRow * 5 + 25 && i < items.size(); i++)
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

void ScrollableItemPanelSelect::SetOnItemSelectedCallback(std::function<void(Item&)> _onItemSelected)
{
	OnItemSelected = _onItemSelected;
}


Item ScrollableItemPanelSelect::RetrieveCurrentlySelectedItem()
{
	if (currentlySelectedItem != -1)
	{
		auto item = items[currentlySelectedItem]->GetItem();
		SAFE_DELETE(items[currentlySelectedItem]);
		currentlySelectedItem = -1;
		return item;
	}

	Item item;
	item.id = ItemID::empty;
	return item;
}


bool ScrollableItemPanelSelect::IsItemSelected()
{
	return currentlySelectedItem != -1;
}

int ScrollableItemPanelSelect::CurrentItemPrice()
{
	if (currentlySelectedItem != -1)
		return g_pObjectProperties->getItemStats(items[currentlySelectedItem]->GetItem().id).price;
	else
		return 0;
}

void ScrollableItemPanelSelect::RenderItems()
{
	int showTooltipForItem = -1;
	for (int i = currentUpperRow * 5; i < currentUpperRow * 5 + 25 && i < items.size(); i++)
	{
		if (items[i] != nullptr)
		{
			items[i]->Render(engine->GetWindow());

			if (currentlySelectedItem == i)
				selectedItemFrame.Render(engine->GetWindow());

			if (items[i]->Contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) != Held)
				showTooltipForItem = items[i]->GetItem().id;
		}
	}
	ShowTooltipForItem(ItemID(showTooltipForItem));
}
