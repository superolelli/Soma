#pragma once

#include "InventoryItemWrapper.hpp"
#include "GameStatus.hpp"
#include "AdventureGroup.hpp"
#include "ItemRowPanel.hpp"
#include "ItemPanelDragAndDropDecorator.hpp"

#include "../Framework/Patterns/Observer.hpp"

class ConsumablePanel : CObserver
{
public:

	ConsumablePanel(CGameEngine *_engine, AdventureGroup *_adventureGroup);
	~ConsumablePanel();

	void Update();
	void Render();

	void AddItem(Item _item);

	void SetPos(int _x, int _y);

	void OnNotify(ObserverNotification const &_notification) override;

private:
	CGameEngine *engine;
	AdventureGroup *adventureGroup;

	CSprite consumablePanel;
	ItemPanelDragAndDropDecorator itemRowPanel;

	InventoryItemWrapper* OnItemFromItemPanelReceived(InventoryItemWrapper* _receivedItem);
};