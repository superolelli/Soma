#include "ConsumablePanel.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\ObjectPropertiesManager.hpp"
#include "../Framework/Patterns/Subject.hpp"

ConsumablePanel::ConsumablePanel(CGameEngine* _engine, AdventureGroup* _adventureGroup)
	: engine(_engine)
	, adventureGroup(_adventureGroup)
	, consumablePanel(g_pTextures->consumablePanel)
	, itemRowPanel(_engine, new ItemRowPanel(_engine))
{
	consumablePanel.SetPos(1200, 855);

	itemRowPanel.SetOnItemDroppedCallback([&](InventoryItemWrapper* _item) {return OnItemFromItemPanelReceived(_item); });
	itemRowPanel.SetPos(consumablePanel.GetRect().left + 49, consumablePanel.GetRect().top + 44);

	for (auto &c : g_pGameStatus->GetConsumables())
		AddItem(c);

	g_pGameStatus->AddObserver(this);
}


ConsumablePanel::~ConsumablePanel()
{
	g_pGameStatus->RemoveObserver(this);
}


void ConsumablePanel::SetPos(int _x, int _y)
{
	consumablePanel.SetPos(_x, _y);
	itemRowPanel.SetPos(_x + 49, _y + 44);
}

void ConsumablePanel::Update()
{
	itemRowPanel.Update();
}


void ConsumablePanel::Render()
{
	consumablePanel.Render(engine->GetRenderTarget());
	itemRowPanel.Render();
	itemRowPanel.RenderCurrentlyDraggedItem();
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
				if (g_pObjectProperties->consumableStats[_receivedItem->GetItem().id - CONSUMABLE_ITEMS_START].heal > 0)
				{
					auto health = g_pObjectProperties->consumableStats[_receivedItem->GetItem().id - CONSUMABLE_ITEMS_START].heal;
					adventureGroup->GetPlayer(i)->Status().GainHealth(health);
				}

				if (g_pObjectProperties->consumableStats[_receivedItem->GetItem().id - CONSUMABLE_ITEMS_START].buff.duration > 0)
				{
					auto buff = g_pObjectProperties->consumableStats[_receivedItem->GetItem().id - CONSUMABLE_ITEMS_START].buff;
					adventureGroup->GetPlayer(i)->Status().AddBuff(buff);
				}

				g_pSounds->PlaySound(soundID::DRINK);
				_receivedItem->SetItemAmount(_receivedItem->GetItem().number-1);
				g_pGameStatus->RemoveItem(_receivedItem->GetItem());
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


void ConsumablePanel::OnNotify(ObserverNotification const &_notification)
{
	auto notification = dynamic_cast<ObserverNotificationGameStatus const*>(&_notification);

	if (notification->event == gameStatusEvents::consumableAdded)
		AddItem(notification->item);
}

void ConsumablePanel::AddItem(Item _item)
{
	itemRowPanel.AddItem(_item);
}