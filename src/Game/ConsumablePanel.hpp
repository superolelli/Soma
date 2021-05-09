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

	void Init(CGameEngine *_engine, GameStatus *_gameStatus, AdventureGroup *_adventureGroup);
	void Update();
	void Render();
	void Quit();

	void AddItem(Item _item);

	void SetPos(int _x, int _y);

	void OnNotify(ObserverNotification const &_notification) override;

private:
	CGameEngine *engine;
	GameStatus *gameStatus;
	AdventureGroup *adventureGroup;

	CSprite consumablePanel;
	ItemPanelDragAndDropDecorator itemRowPanel;

	InventoryItemWrapper* OnItemFromItemPanelReceived(InventoryItemWrapper* _receivedItem);
};