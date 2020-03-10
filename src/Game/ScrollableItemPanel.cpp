#include "ScrollableItemPanel.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\StringManager.hpp"

void ScrollableItemPanel::Init(CGameEngine * _engine)
{
	engine = _engine;

	currentUpperRow = 0;

	scrollableItemPanel.Load(g_pTextures->scrollableItemPanel);

	scrollbar.Init(35, 566);
	scrollbar.SetColor(sf::Color(28, 28, 28));

	tooltip.Init();
}

void ScrollableItemPanel::SetPos(int _x, int _y)
{
	scrollableItemPanel.SetPos(_x, _y);
	scrollbar.SetPos(scrollableItemPanel.GetGlobalRect().left + 600, scrollableItemPanel.GetGlobalRect().top + 12);
}


void ScrollableItemPanel::Quit()
{
	for (auto &i : items)
		SAFE_DELETE(i);
}


void ScrollableItemPanel::Update()
{
	UpdateScrollbar();
}


void ScrollableItemPanel::UpdateScrollbar()
{
	scrollbar.Update(*engine);

	if (scrollbar.GetCurrentStep() != currentUpperRow)
	{
		currentUpperRow = scrollbar.GetCurrentStep();
		RecalculatePositionsOfItems();
	}
}


void ScrollableItemPanel::RecalculatePositionsOfItems()
{
	int yPos = scrollableItemPanel.GetGlobalRect().top + 11;
	int currentIndex = 0;

	for (int y = 0; y < 5; y++)
	{
		int xPos = scrollableItemPanel.GetGlobalRect().left + 11;
		for (int x = 0; x < 5; x++)
		{
			currentIndex = (currentUpperRow + y) * 5 + x;
			if (currentIndex < items.size() && items[currentIndex] != nullptr)
				items[currentIndex]->SetPos(xPos, yPos);

			xPos += 116;
		}
		yPos += 116;
	}
}



void ScrollableItemPanel::Render()
{
	scrollableItemPanel.Render(engine->GetWindow());
	scrollbar.Render(engine->GetWindow());
	RenderItems();
}


void ScrollableItemPanel::RenderItems()
{
	int showTooltipForItem = -1;
	for (int i = currentUpperRow * 5; i < currentUpperRow * 5 + 25 && i < items.size(); i++)
	{
		if (items[i] != nullptr)
		{
			items[i]->Render(engine->GetWindow());

			if (items[i]->Contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) != Held)
				showTooltipForItem = items[i]->GetItem().id;
		}
	}
	ShowTooltipForItem(ItemID(showTooltipForItem));
}


void ScrollableItemPanel::ShowTooltipForItem(ItemID _itemID)
{
	if (_itemID != -1)
	{
		tooltip.SetItem(_itemID);
		tooltip.ShowTooltip(engine->GetWindow(), engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
	}
}


void ScrollableItemPanel::AddItem(Item _item)
{
	InventoryItemWrapper *newItem = new InventoryItemWrapper;

	CSprite newSprite;
	newSprite.Load(g_pTextures->item[_item.id]);
	newItem->Init(std::move(_item), std::move(newSprite));

	int freeSlot = GetFirstFreeItemSlot();
	int xPos = (freeSlot % 5) * 116 + scrollableItemPanel.GetGlobalRect().left + 11;
	int yPos = (freeSlot / 5) * 116 + scrollableItemPanel.GetGlobalRect().top + 11 - currentUpperRow * 116;
	newItem->SetPos(xPos, yPos);

	if (freeSlot < items.size())
		items[freeSlot] = newItem;
	else
		items.push_back(newItem);

	scrollbar.SetNumberOfSteps(std::max(1, (int)(items.size() - 21) / 5 + 1));
}



int ScrollableItemPanel::GetFirstFreeItemSlot()
{
	for (int i = 0; i < items.size(); i++)
	{
		if (items[i] == nullptr)
			return i;
	}

	return items.size();
}