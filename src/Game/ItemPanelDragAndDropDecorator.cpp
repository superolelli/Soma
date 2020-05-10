#include "ItemPanelDragAndDropDecorator.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

void ItemPanelDragAndDropDecorator::Init(CGameEngine *_engine, ItemPanel *_itemPanel)
{
	ItemPanelDecorator::Init(_engine, _itemPanel);
	currentDraggedItem = -1;
	currentDraggedItemOldX = 0;
	currentDraggedItemOldY = 0;
}


void ItemPanelDragAndDropDecorator::SetOnItemDroppedCallback(std::function<InventoryItemWrapper*(InventoryItemWrapper*)> _onItemDropped)
{
	OnItemDropped = _onItemDropped;
}


void ItemPanelDragAndDropDecorator::Update()
{
	if(currentDraggedItem == -1)
		itemPanel->Update();

	HandleStartedDrag();
	HandleContinuedDrag();
	HandleDrop();
}


void ItemPanelDragAndDropDecorator::HandleStartedDrag()
{
	if (engine->GetButtonstates(ButtonID::Left) != Pressed || !itemPanel->itemPanel.GetRect().contains(engine->GetMousePos()))
		return;

	for (int i = 0; i < itemPanel->items.size(); i++)
	{
		if (itemPanel->items[i] == nullptr)
			continue;

		if (itemPanel->items[i]->Contains(engine->GetMousePos()))
		{
			currentDraggedItem = i;
			currentDraggedItemOldX = itemPanel->items[i]->GetGlobalBounds().left;
			currentDraggedItemOldY = itemPanel->items[i]->GetGlobalBounds().top;
		}
	}
}

void ItemPanelDragAndDropDecorator::HandleContinuedDrag()
{
	if (engine->GetButtonstates(ButtonID::Left) == Held && currentDraggedItem != -1)
		itemPanel->items[currentDraggedItem]->SetCenterPos(engine->GetMousePos().x, engine->GetMousePos().y);
}

void ItemPanelDragAndDropDecorator::HandleDrop()
{
	if (engine->GetButtonstates(ButtonID::Left) == Released && currentDraggedItem != -1)
	{
		itemPanel->items[currentDraggedItem] = OnItemDropped(itemPanel->items[currentDraggedItem]);

		if (itemPanel->items[currentDraggedItem] != nullptr)
			itemPanel->items[currentDraggedItem]->SetPos(currentDraggedItemOldX, currentDraggedItemOldY);
		currentDraggedItem = -1;
	}
}


void ItemPanelDragAndDropDecorator::Render()
{
	itemPanel->Render(currentDraggedItem);
}


void ItemPanelDragAndDropDecorator::RenderCurrentlyDraggedItem()
{
	if (currentDraggedItem != -1)
		itemPanel->items[currentDraggedItem]->Render(engine->GetRenderTarget());
}