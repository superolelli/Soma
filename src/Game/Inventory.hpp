#pragma once

#include "GameStatus.hpp"
#include "../Framework/Gui/Button.hpp"
#include "../Framework/Gui/Scrollbar.hpp"
#include "ItemTooltip.hpp"

class Inventory
{
public:

	void Init(GameStatus *_gameStatus, CGameEngine *_engine);
	void Update();
	void Render();

	void Open(int _player = 0);
	bool IsOpen() { return !closed; }

private:
	bool closed;

	GameStatus *gameStatus;
	CGameEngine *engine;

	int currentPlayer;
	int currentUpperRow;

	CSprite inventoryPanel;
	VerticalScrollbar scrollbar;

	sf::Text currentPlayerName;
	sf::Text panelTitle;

	CButton buttonNext;
	CButton buttonPrevious;
	CButton buttonClose;

	std::vector<std::pair<Item, CSprite>> items;

	ItemTooltip tooltip;

	void CheckButtonsForPlayerChoosing();
	void UpdateGUIForChosenPlayer();

	void OnItemAdded(Item _item);

	void RecalculatePositionsOfItems();
};