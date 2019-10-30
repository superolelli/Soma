#pragma once

#include "../Framework/Gui/Button.hpp"
#include "InventoryItemWrapper.hpp"
#include "GameStatus.hpp"

class LootableDialog
{
public:

	void Init(CGameEngine *_engine, GameStatus *_gameStatus,  int _lootableID);
	void Update();
	void Render();
	void Quit();

	void AddItem(Item &_item);

	void SetTitle(std::string &_str);
	void SetPos(int _x, int _y);

	bool IsOpen() { return isOpen; }

private:
	CGameEngine *engine;
	GameStatus *gameStatus;

	bool isOpen;

	CSprite lootablePanel;
	CButton buttonTakeAll;
	CButton buttonClose;

	sf::Text lootableNameText;

	InventoryItemWrapper *items[9];

	ItemTooltip tooltip;

	void TakeAllItems();
	void TakeItem(int _id);
	int GetFreeItemSlot();
};