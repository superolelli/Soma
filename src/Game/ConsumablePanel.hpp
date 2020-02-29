#pragma once

#include "InventoryItemWrapper.hpp"
#include "GameStatus.hpp"
#include "AdventureGroup.hpp"
#include "ItemRowPanel.hpp"

class ConsumablePanel
{
public:

	void Init(CGameEngine *_engine, GameStatus *_gameStatus, AdventureGroup *_adventureGroup);
	void Update();
	void Render();
	void Quit();

	void SetPos(int _x, int _y);
	void SetAdventureGroup(AdventureGroup *_adventureGroup) { adventureGroup = _adventureGroup; }

	bool IsOpen() { return isOpen; }

private:
	CGameEngine *engine;
	GameStatus *gameStatus;
	AdventureGroup *adventureGroup;

	bool isOpen;

	CSprite consumablePanel;
	ItemRowPanel itemRowPanel;

	InventoryItemWrapper* OnItemFromItemPanelReceived(InventoryItemWrapper* _receivedItem);
	void OnItemAdded(Item _item, bool _onlyAmountChanged);
};