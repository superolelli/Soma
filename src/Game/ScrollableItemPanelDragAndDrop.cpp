#include "ScrollableItemPanelDragAndDrop.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\StringManager.hpp"

void ScrollableItemPanelDragAndDrop::Init(CGameEngine * _engine)
{
	ScrollableItemPanel::Init(_engine);
	currentDraggedItem = -1;
}



void ScrollableItemPanelDragAndDrop::Update()
{
	if (currentDraggedItem == -1)
		ScrollableItemPanel::Update();

	HandleDragAndDrop();
}



void ScrollableItemPanelDragAndDrop::HandleDragAndDrop()
{
	HandleStartedDrag();
	HandleContinuedDrag();
	HandleDrop();
}


void ScrollableItemPanelDragAndDrop::HandleStartedDrag()
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


void ScrollableItemPanelDragAndDrop::HandleContinuedDrag()
{
	if (engine->GetButtonstates(ButtonID::Left) == Held && currentDraggedItem != -1)
		items[currentDraggedItem]->SetCenterPos(engine->GetMousePos().x, engine->GetMousePos().y);
}



void ScrollableItemPanelDragAndDrop::HandleDrop()
{
	if (engine->GetButtonstates(ButtonID::Left) == Released && currentDraggedItem != -1)
	{
		items[currentDraggedItem] = OnItemDropped(items[currentDraggedItem]);

		if (items[currentDraggedItem] != nullptr)
			items[currentDraggedItem]->SetPos(currentDraggedItemOldX, currentDraggedItemOldY);

		currentDraggedItem = -1;
	}
}



void ScrollableItemPanelDragAndDrop::RenderItems()
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
	ShowTooltipForItem(ItemID(showTooltipForItem));
}


void ScrollableItemPanelDragAndDrop::RenderCurrentlyDraggedItem()
{
	if (currentDraggedItem != -1)
		items[currentDraggedItem]->Render(engine->GetWindow());
}

void ScrollableItemPanelDragAndDrop::SetOnItemDroppedCallback(std::function<InventoryItemWrapper*(InventoryItemWrapper*)> _onItemDropped)
{
	OnItemDropped = _onItemDropped;
}
