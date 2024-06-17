#pragma once

#include "../Framework/Gui/Button.hpp"
#include "InventoryItemWrapper.hpp"
#include "GameStatus.hpp"

class LootableDialog
{
public:

	LootableDialog(CGameEngine *_engine,  int _lootableID);
	~LootableDialog();

	void Update();
	void Render();

	void AddItem(Item _item);

	void SetTitle(std::string const &_str);
	void SetPos(int _x, int _y);

	bool IsOpen() { return isOpen; }

private:
	CGameEngine *engine;

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