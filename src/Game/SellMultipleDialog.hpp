#pragma once

#include "../Framework/Gui/Slider.hpp"
#include "../Framework/Gameengine.hpp"
#include "../Framework/Gui/Button.hpp"
#include "Item.hpp"

class SellMultipleDialog
{
public:

	void Init(CGameEngine *_engine);
	void Update();
	void Render();
	void Quit();

	void SetItem(Item const &_item);

	void SetPos(int _x, int _y);

	int GetAmount() { return amount; }

	bool IsOpen() { return isOpen; }

private:
	CGameEngine *engine;

	bool isOpen;

	int amount;

	CSprite sellMultiplePanel;
	Slider slider;
	CButton buttonSell;
	CButton buttonCancel;
};