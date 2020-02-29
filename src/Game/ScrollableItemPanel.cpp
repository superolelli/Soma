#include "ScrollableItemPanel.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\StringManager.hpp"

void ScrollableItemPanel::Init(CGameEngine * _engine, std::function<InventoryItemWrapper*(InventoryItemWrapper*)> _onItemDropped)
{
	engine = _engine;
	OnItemDropped = _onItemDropped;

	currentUpperRow = 0;

	currentDraggedItem = -1;

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
	HandleDragAndDrop();
}


void ScrollableItemPanel::UpdateScrollbar()
{
	if (currentDraggedItem == -1)
		scrollbar.Update(*engine);

	if (scrollbar.GetCurrentStep() != currentUpperRow)
	{
		currentUpperRow = scrollbar.GetCurrentStep();
		RecalculatePositionsOfItems();
	}
}


void ScrollableItemPanel::HandleDragAndDrop()
{
	HandleStartedDrag();
	HandleContinuedDrag();
	HandleDrop();
}


void ScrollableItemPanel::HandleStartedDrag()
{
	if (engine->GetButtonstates(ButtonID::Left) != Pressed)
		return;

	int x = engine->GetMousePos().x - (scrollableItemPanel.GetGlobalRect().left + 11);
	int y = engine->GetMousePos().y - (scrollableItemPanel.GetGlobalRect().top + 11);
	int itemPointedTo = (y / 116 + currentUpperRow) * 5 + x / 116;

	if (itemPointedTo >= 0 && items.size() > itemPointedTo && items[itemPointedTo] != nullptr)
	{
		if (items[itemPointedTo]->Contains(engine->GetMousePos()))
		{
			currentDraggedItemOldX = items[itemPointedTo]->GetGlobalBounds().left;
			currentDraggedItemOldY = items[itemPointedTo]->GetGlobalBounds().top;
			currentDraggedItem = itemPointedTo;
		}
	}
}


void ScrollableItemPanel::HandleContinuedDrag()
{
	if (engine->GetButtonstates(ButtonID::Left) == Held && currentDraggedItem != -1)
		items[currentDraggedItem]->SetCenterPos(engine->GetMousePos().x, engine->GetMousePos().y);
}



void ScrollableItemPanel::HandleDrop()
{
	if (engine->GetButtonstates(ButtonID::Left) == Released && currentDraggedItem != -1)
	{
		items[currentDraggedItem] = OnItemDropped(items[currentDraggedItem]);

		if(items[currentDraggedItem] != nullptr)
			items[currentDraggedItem]->SetPos(currentDraggedItemOldX, currentDraggedItemOldY);

		currentDraggedItem = -1;
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
		if (items[i] != nullptr && i != currentDraggedItem)
		{
			items[i]->Render(engine->GetWindow());

			if (items[i]->Contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) != Held)
				showTooltipForItem = items[i]->GetItem().id;
		}
	}

	if (showTooltipForItem != -1)
	{
		tooltip.SetItem(ItemID(showTooltipForItem));
		tooltip.ShowTooltip(engine->GetWindow(), engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
	}
}

void ScrollableItemPanel::RenderCurrentlyDraggedItem()
{
	if (currentDraggedItem != -1)
		items[currentDraggedItem]->Render(engine->GetWindow());
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