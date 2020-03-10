#pragma once

#include "InventoryItemWrapper.hpp"
#include "GameStatus.hpp"
#include "AdventureGroup.hpp"
#include "ItemRowPanelDragAndDrop.hpp"

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

	void OnNotify(ObserverNotification &_notification);

private:
	CGameEngine *engine;
	GameStatus *gameStatus;
	AdventureGroup *adventureGroup;

	CSprite consumablePanel;
	ItemRowPanelDragAndDrop itemRowPanel;

	InventoryItemWrapper* OnItemFromItemPanelReceived(InventoryItemWrapper* _receivedItem);
};