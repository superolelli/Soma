#include "ItemRowPanel.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

ItemRowPanel::ItemRowPanel(CGameEngine *_engine)
	: ItemPanel(_engine, g_pTextures->itemRowPanel)
{
	items.resize(5);
}

void ItemRowPanel::Clear()
{
	ItemPanel::Clear();
	items.resize(5);
}

void ItemRowPanel::SetPos(int _x, int _y)
{
	itemPanel.SetPos(_x, _y);

	for (int i = 0; i < items.size(); i++)
	{
		if (items[i] != nullptr)
			items[i]->SetPos(_x + 10 + 116 * i, _y + 10);
	}
}

void ItemRowPanel::Update()
{
}


void ItemRowPanel::Render(int _excludeItemNumber)
{
	itemPanel.Render(engine->GetRenderTarget());
	RenderItems(0, 5, _excludeItemNumber);
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

	CSprite newSprite(g_pTextures->item[_item.id]);
	InventoryItemWrapper *newItem = new InventoryItemWrapper(std::move(_item), std::move(newSprite));

	int freeSlot = GetFirstFreeSlot();
	int xPos = freeSlot * 116 + itemPanel.GetGlobalRect().left + 10;
	int yPos = itemPanel.GetGlobalRect().top + 10;
	newItem->SetPos(xPos, yPos);

	items[freeSlot] = newItem;
}