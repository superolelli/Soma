#pragma once

#include "GameStatus.hpp"
#include "../Framework/Gui/Button.hpp"
#include "InventoryItemWrapper.hpp"



class VendingMachinePanel
{
public:

	void Init(GameStatus *_gameStatus, CGameEngine *_engine);
	void Update();
	void Render();
	void Quit();

	void Open();
	bool IsOpen() { return !closed; }

private:
	GameStatus *gameStatus;
	CGameEngine *engine;

	CSprite vendingMachinePanel;
	CButton buttonClose;

	bool closed;
};