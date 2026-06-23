#include "LootableFactory.hpp"
#include "Resources\ObjectPropertiesManager.hpp"
#include "ItemFactory.hpp"

LootableFactory::LootableFactory(DialogManager * _dialogManager, int _level)
	: dialogManager(_dialogManager)
	, level(_level)
{
}



Lootable * LootableFactory::CreateLootable(LevelType _levelType, int _roomX, int _roomY)
{
	int lootableIndex = LootableID(rand() % g_pObjectProperties->lootablesByLevelType[_levelType].size());
	LootableID lootableId = g_pObjectProperties->lootablesByLevelType[_levelType][lootableIndex];
	Lootable* lootable = new Lootable(lootableId, dialogManager);

	for(int i = 0; i < 9; i++)
	{
		lootable->AddItem(ItemFactory::CreateItemForLootable(_levelType, level, lootableId));

		if (rand() % 4 != 0)
			break;
	}

	lootable->SetPos(_roomX + g_pObjectProperties->lootableProperties[lootableId].position.x, _roomY + g_pObjectProperties->lootableProperties[lootableId].position.y);

	return lootable;
}