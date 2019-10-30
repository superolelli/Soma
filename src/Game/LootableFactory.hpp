#pragma once

#include "Lootable.hpp"

class LootableFactory
{
public:

	void Init(DialogManager *_dialogManager, GameStatus *_gameStatus);
	void SetLevel(int _level);

	Lootable *CreateLootable(int _roomX, int _roomY);

private:

	DialogManager *dialogManager;
	GameStatus *gameStatus;

	int level;
	int numberOfPossibleItems;

	ItemID GetRandomItemID(LootableID _lootableID);
	sf::Color GetRandomColor();
	int GetRandomItemNumber(ItemID _id);
};