#include "LootableFactory.hpp"
#include "Resources\ObjectPropertiesManager.hpp"
#include "ItemFactory.hpp"

void LootableFactory::Init(DialogManager * _dialogManager, GameStatus * _gameStatus)
{
	dialogManager = _dialogManager;
	gameStatus = _gameStatus;
}

void LootableFactory::SetLevel(int _level)
{
	level = _level;
}


Lootable * LootableFactory::CreateLootable(LevelType _levelType, int _roomX, int _roomY)
{
	Lootable *lootable = new Lootable;
	int lootableIndex = LootableID(rand() % g_pObjectProperties->lootablesByLevelType[_levelType].size());
	LootableID lootableId = g_pObjectProperties->lootablesByLevelType[_levelType][lootableIndex];

	lootable->Init(lootableId, dialogManager, gameStatus);

	for(int i = 0; i < 9; i++)
	{
		lootable->AddItem(ItemFactory::CreateItemForLootable(_levelType, level, lootableId));

		if (rand() % 4 != 0)
			break;
	}

	lootable->SetPos(_roomX + g_pObjectProperties->lootableProperties[lootableId].position.x, _roomY + g_pObjectProperties->lootableProperties[lootableId].position.y);

	return lootable;
}