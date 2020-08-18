#pragma once

#include "Lootable.hpp"
#include "LevelSpecs.hpp"

class LootableFactory
{
public:

	void Init(DialogManager *_dialogManager, GameStatus *_gameStatus);
	void SetLevel(int _level);

	Lootable *CreateLootable(LevelType _levelType, int _roomX, int _roomY);

private:

	DialogManager *dialogManager;
	GameStatus *gameStatus;

	int level;
};