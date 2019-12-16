#include "ConsumablePanel.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

void ConsumablePanel::Init(CGameEngine *_engine, GameStatus * _gameStatus)
{
	engine = _engine;
	gameStatus = _gameStatus;

	gameStatus->SetOnConsumableAddedCallback([&](Item _item, bool _onlyAmountChanged) {OnItemAdded(_item, _onlyAmountChanged); });

	consumablePanel.Load(g_pTextures->consumablePanel);
	consumablePanel.SetPos(1200, 878);

	tooltip.Init();

	isOpen = true;
}


void ConsumablePanel::Quit()
{
	for (int i = 0; i < 5; i++)
		SAFE_DELETE(items[i]);
}


void ConsumablePanel::SetPos(int _x, int _y)
{
	consumablePanel.SetPos(_x, _y);
}

void ConsumablePanel::Update()
{
	if (isOpen)
	{
		if (engine->GetButtonstates(ButtonID::Left) == Released)
		{
			for (int i = 0; i < 5; i++)
			{
				if (items[i] == nullptr)
					continue;

				if (items[i]->Contains(engine->GetMousePos()))
				{
					//TODO: Start Drag
				}
			}
		}
	}
}


void ConsumablePanel::Render()
{
	if (isOpen)
	{
		consumablePanel.Render(engine->GetWindow());

		for (auto i : items)
		{
			if (i != nullptr)
				i->Render(engine->GetWindow());
		}

		for (auto i : items)
		{
			if (i == nullptr)
				continue;

			if (i->Contains(engine->GetMousePos()))
			{
				tooltip.SetItem(i->GetItem().id);
				tooltip.ShowTooltip(engine->GetWindow(), engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
				break;
			}
		}
	}
}


void ConsumablePanel::OnItemAdded(Item _item, bool _onlyAmountChanged)
{
	if (_onlyAmountChanged)
	{
		for (auto c : items)
			c->SetItemAmount(_item.number);
		return;
	}

	InventoryItemWrapper *newItem = new InventoryItemWrapper;

	CSprite newSprite;
	newSprite.Load(g_pTextures->item[_item.id]);
	newItem->Init(std::move(_item), std::move(newSprite));

	int freeSlot = GetFirstFreeSlot();
	int xPos = freeSlot * 116 + consumablePanel.GetGlobalRect().left + 60;
	int yPos = consumablePanel.GetGlobalRect().top + 54;
	newItem->SetPos(xPos, yPos);

	items[freeSlot] = newItem;
}



int ConsumablePanel::GetFirstFreeSlot()
{
	for (int i = 0; i < 5; i++)
	{
		if (items[i] == nullptr)
			return i;
	}
	return -1;
}