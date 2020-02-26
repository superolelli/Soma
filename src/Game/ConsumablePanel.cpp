#include "ConsumablePanel.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

void ConsumablePanel::Init(CGameEngine *_engine, GameStatus * _gameStatus, AdventureGroup *_adventureGroup)
{
	engine = _engine;
	gameStatus = _gameStatus;
	adventureGroup = _adventureGroup;

	gameStatus->SetOnConsumableAddedCallback([&](Item _item, bool _onlyAmountChanged) {OnItemAdded(_item, _onlyAmountChanged); });

	consumablePanel.Load(g_pTextures->consumablePanel);
	consumablePanel.SetPos(1200, 855);

	tooltip.Init();

	isOpen = true;

	currentDraggedItem = -1;
	currentDraggedItemOldX = 0;
	currentDraggedItemOldY = 0;
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
		HandleStartedDrag();
		HandleContinuedDrag();
		HandleDrop();
	}
}


void ConsumablePanel::HandleStartedDrag()
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

void ConsumablePanel::HandleContinuedDrag()
{
	if (engine->GetButtonstates(ButtonID::Left) == Held && currentDraggedItem != -1)
		items[currentDraggedItem]->SetCenterPos(engine->GetMousePos().x, engine->GetMousePos().y);
}

void ConsumablePanel::HandleDrop()
{
	if (engine->GetButtonstates(ButtonID::Left) == Released && currentDraggedItem != -1)
	{
		if (adventureGroup != nullptr)
		{
			for (int i = 0; i < 4; i++)
			{
				auto rect = adventureGroup->GetPlayer(i)->GetRect();
				rect.left -= engine->GetViewPosition().x;
				if (items[currentDraggedItem]->GetGlobalBounds().intersects(rect))
				{
					auto health = g_pObjectProperties->consumableStats[items[currentDraggedItem]->GetItem().id - CONSUMABLE_ITEMS_START].health;
					adventureGroup->GetPlayer(i)->Status().GainHealth(health);
					g_pSounds->PlaySound(soundID::DRINK);
					items[currentDraggedItem]->GetItem().number--;
					break;
				}
			}
		}

		items[currentDraggedItem]->SetPos(currentDraggedItemOldX, currentDraggedItemOldY);
		if (items[currentDraggedItem]->GetItem().number <= 0)
			SAFE_DELETE(items[currentDraggedItem]);
		currentDraggedItem = -1;
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

			if (i->Contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) != Held)
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