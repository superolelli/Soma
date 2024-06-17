#pragma once

#include "Lootable.hpp"
#include "LevelSpecs.hpp"

class LootableFactory
{
public:

	LootableFactory(DialogManager* _dialogManager, int _level);
	Lootable *CreateLootable(LevelType _levelType, int _roomX, int _roomY);

private:

	DialogManager *dialogManager;
	int level;
};