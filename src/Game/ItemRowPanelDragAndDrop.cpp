#include "ItemRowPanelDragAndDrop.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

void ItemRowPanelDragAndDrop::Init(CGameEngine *_engine)
{
	ItemRowPanel::Init(_engine);

	currentDraggedItem = -1;
	currentDraggedItemOldX = 0;
	currentDraggedItemOldY = 0;
}

void ItemRowPanelDragAndDrop::SetOnItemDroppedCallback(std::function<InventoryItemWrapper*(InventoryItemWrapper*)> _onItemDropped)
{
	OnItemDropped = _onItemDropped;
}


void ItemRowPanelDragAndDrop::Update()
{
	HandleStartedDrag();
	HandleContinuedDrag();
	HandleDrop();
}


void ItemRowPanelDragAndDrop::HandleStartedDrag()
{
	if (engine->GetButtonstates(ButtonID::Left) != Pressed)
		return;

	for (int i = 0; i < 5; i++)
	{
		if (items[i] == nullptr)
			continue;

		if (items[i]->Contains(engine->GetMousePos()))
		{
			currentDraggedItem = i;
			currentDraggedItemOldX = items[i]->GetGlobalBounds().left;
			currentDraggedItemOldY = items[i]->GetGlobalBounds().top;
		}
	}
}

void ItemRowPanelDragAndDrop::HandleContinuedDrag()
{
	if (engine->GetButtonstates(ButtonID::Left) == Held && currentDraggedItem != -1)
		items[currentDraggedItem]->SetCenterPos(engine->GetMousePos().x, engine->GetMousePos().y);
}

void ItemRowPanelDragAndDrop::HandleDrop()
{
	if (engine->GetButtonstates(ButtonID::Left) == Released && currentDraggedItem != -1)
	{
		items[currentDraggedItem] = OnItemDropped(items[currentDraggedItem]);

		if (items[currentDraggedItem] != nullptr)
			items[currentDraggedItem]->SetPos(currentDraggedItemOldX, currentDraggedItemOldY);
		currentDraggedItem = -1;
	}
}


void ItemRowPanelDragAndDrop::RenderItems()
{
	int showTooltipForItem = -1;
	for (int i = 0; i < 5; i++)
	{
		if (items[i] != nullptr && currentDraggedItem != i)
		{
			items[i]->Render(engine->GetWindow());
			if (items[i]->Contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) != Held)
				showTooltipForItem = i;
		}
	}
	ShowTooltipForItem(showTooltipForItem);
}


void ItemRowPanelDragAndDrop::RenderCurrentlyDraggedItem()
{
	if (currentDraggedItem != -1)
		items[currentDraggedItem]->Render(engine->GetWindow());
}