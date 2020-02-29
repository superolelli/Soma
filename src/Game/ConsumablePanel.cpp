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

	itemRowPanel.Init(engine, [&](InventoryItemWrapper* _item) {return OnItemFromItemPanelReceived(_item); });
	itemRowPanel.SetPos(consumablePanel.GetRect().left + 49, consumablePanel.GetRect().top + 44);

	isOpen = true;
}


void ConsumablePanel::Quit()
{
	itemRowPanel.Quit();
}


void ConsumablePanel::SetPos(int _x, int _y)
{
	consumablePanel.SetPos(_x, _y);
	itemRowPanel.SetPos(_x + 49, _y + 44);
}

void ConsumablePanel::Update()
{
	if (isOpen)
	{
		itemRowPanel.Update();
	}
}


void ConsumablePanel::Render()
{
	if (isOpen)
	{
		consumablePanel.Render(engine->GetWindow());
		itemRowPanel.Render();
		itemRowPanel.RenderCurrentlyDraggedItem();
	}
}


InventoryItemWrapper * ConsumablePanel::OnItemFromItemPanelReceived(InventoryItemWrapper * _receivedItem)
{
	if (adventureGroup != nullptr)
	{
		for (int i = 0; i < 4; i++)
		{
			auto rect = adventureGroup->GetPlayer(i)->GetRect();
			rect.left -= engine->GetViewPosition().x;
			if (_receivedItem->GetGlobalBounds().intersects(rect))
			{
				auto health = g_pObjectProperties->consumableStats[_receivedItem->GetItem().id - CONSUMABLE_ITEMS_START].health;
				adventureGroup->GetPlayer(i)->Status().GainHealth(health);
				g_pSounds->PlaySound(soundID::DRINK);
				_receivedItem->SetItemAmount(_receivedItem->GetItem().number-1);
				gameStatus->RemoveItem(_receivedItem->GetItem());
				break;
			}
		}
	}

	if (_receivedItem->GetItem().number <= 0)
	{
		SAFE_DELETE(_receivedItem);
	}

	return _receivedItem;
}


void ConsumablePanel::OnItemAdded(Item _item, bool _onlyAmountChanged)
{
	itemRowPanel.AddItem(_item, _onlyAmountChanged);
}