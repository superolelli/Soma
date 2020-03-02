#include "ItemRowPanel.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

void ItemRowPanel::Init(CGameEngine *_engine, std::function<InventoryItemWrapper*(InventoryItemWrapper*)> _onItemDropped)
{
	engine = _engine;
	OnItemDropped = _onItemDropped;

	itemRowPanel.Load(g_pTextures->itemRowPanel);

	tooltip.Init();

	currentDraggedItem = -1;
	currentDraggedItemOldX = 0;
	currentDraggedItemOldY = 0;
}


void ItemRowPanel::Quit()
{
	for (int i = 0; i < 5; i++)
		SAFE_DELETE(items[i]);
}


void ItemRowPanel::SetPos(int _x, int _y)
{
	itemRowPanel.SetPos(_x, _y);

	for (int i = 0; i < 5; i++)
	{
		if (items[i] != nullptr)
			items[i]->SetPos(_x + 10 + 116 * i, _y + 10);
	}
}

void ItemRowPanel::Update()
{
	HandleStartedDrag();
	HandleContinuedDrag();
	HandleDrop();
}


void ItemRowPanel::HandleStartedDrag()
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

void ItemRowPanel::HandleContinuedDrag()
{
	if (engine->GetButtonstates(ButtonID::Left) == Held && currentDraggedItem != -1)
		items[currentDraggedItem]->SetCenterPos(engine->GetMousePos().x, engine->GetMousePos().y);
}

void ItemRowPanel::HandleDrop()
{
	if (engine->GetButtonstates(ButtonID::Left) == Released && currentDraggedItem != -1)
	{
		items[currentDraggedItem] = OnItemDropped(items[currentDraggedItem]);

		if(items[currentDraggedItem] != nullptr)
			items[currentDraggedItem]->SetPos(currentDraggedItemOldX, currentDraggedItemOldY);
		currentDraggedItem = -1;
	}
}


void ItemRowPanel::Render()
{
	itemRowPanel.Render(engine->GetWindow());

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

	if (showTooltipForItem != -1)
	{
		tooltip.SetItem(items[showTooltipForItem]->GetItem().id);
		tooltip.ShowTooltip(engine->GetWindow(), engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
	}
}

void ItemRowPanel::RenderCurrentlyDraggedItem()
{
	if (currentDraggedItem != -1)
		items[currentDraggedItem]->Render(engine->GetWindow());
}

void ItemRowPanel::AddItem(Item _item)
{
	for (auto c : items)
	{
		if (c != nullptr && c->GetItem().id == _item.id)
		{
			c->SetItemAmount(c->GetItem().number + _item.number);
			return;
		}
	}

	InventoryItemWrapper *newItem = new InventoryItemWrapper;

	CSprite newSprite;
	newSprite.Load(g_pTextures->item[_item.id]);
	newItem->Init(std::move(_item), std::move(newSprite));

	int freeSlot = GetFirstFreeSlot();
	int xPos = freeSlot * 116 + itemRowPanel.GetGlobalRect().left + 10;
	int yPos = itemRowPanel.GetGlobalRect().top + 10;
	newItem->SetPos(xPos, yPos);

	items[freeSlot] = newItem;
}



int ItemRowPanel::GetFirstFreeSlot()
{
	for (int i = 0; i < 5; i++)
	{
		if (items[i] == nullptr)
			return i;
	}
	return -1;
}