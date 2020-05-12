#include "ItemPanel.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

void ItemPanel::Init(CGameEngine *_engine)
{
	engine = _engine;
	tooltip.Init();
}


void ItemPanel::Clear()
{
	for (auto i : items)
		SAFE_DELETE(i);
	items.clear();
}

void ItemPanel::SortItemsAccordingToColor()
{
	std::cout << "Sorting not implemented for this item panel" << std::endl;
}

void ItemPanel::SortItemsAccordingToNames()
{
	std::cout << "Sorting not implemented for this item panel" << std::endl;
}


void ItemPanel::RenderItems(int _start, int _end, int _excludeItemNumber)
{
	int showTooltipForItem = -1;
	for (int i = _start; i < _end && i < items.size(); i++)
	{
		if (items[i] != nullptr && i != _excludeItemNumber)
		{
			items[i]->Render(engine->GetRenderTarget());
			if (items[i]->Contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) != Held)
				showTooltipForItem = i;
		}
	}

	ShowTooltipForItem(showTooltipForItem);
}


void ItemPanel::ShowTooltipForItem(int _itemID)
{
	if (_itemID != -1)
	{
		tooltip.SetItem(items[_itemID]->GetItem().id);
		tooltip.ShowTooltip(engine->GetRenderTarget(), engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
	}
}


int ItemPanel::GetFirstFreeSlot()
{
	for (int i = 0; i < items.size(); i++)
	{
		if (items[i] == nullptr)
			return i;
	}
	return -1;
}