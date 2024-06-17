#include "LevelBuilder.hpp"
#include "LootableFactory.hpp"


Level *LevelBuilder::buildLevel(LevelType _levelType, int _levelID, DialogManager *_dialogManager)
{
	LootableFactory lootableFactory(_dialogManager, _levelID);

	srand(time(0));
	Level *newLevel = new Level(_levelType, _levelID);

	int position = 0;
	for (int i = 0; i < g_pObjectProperties->levelSpecs[_levelType][_levelID - 1].numberOfRooms - 1; i++)
	{
		//battle
		auto backgroundID = rand() % g_pObjectProperties->levelSpecs[_levelType][_levelID - 1].possibleBackgrounds.size();
		Room* newRoom = new Room(g_pTextures->firstLayerBackgrounds[_levelType][g_pObjectProperties->levelSpecs[_levelType][_levelID - 1].possibleBackgrounds[backgroundID]]);
		newRoom->battle = rand() % 100 < g_pObjectProperties->levelSpecs[_levelType][_levelID - 1].battleProbability * 100.0f;
		if (newRoom->battle)
		{
			int enemyGroup = rand() % g_pObjectProperties->levelSpecs[_levelType][_levelID - 1].possibleEnemyGroups.size();
			for (int j = 0; j < 4; j++)
				newRoom->enemyIds[j] = g_pObjectProperties->levelSpecs[_levelType][_levelID - 1].possibleEnemyGroups[enemyGroup][j];
		}

		//background
		newRoom->background.SetPos(position, 0);
		newRoom->boss = false;
		position += newRoom->background.GetRect().width;

		//lootable
		if (rand() % 100 < g_pObjectProperties->levelSpecs[_levelType][_levelID - 1].lootableProbability * 100.0f)
			newRoom->lootable = lootableFactory.CreateLootable(_levelType, newRoom->background.GetGlobalRect().left, newRoom->background.GetGlobalRect().top);
		else
			newRoom->lootable = nullptr;

		newLevel->AddRoom(newRoom);
	}

	//add end room
	Room *newRoom = new Room(g_pTextures->firstLayerBackgrounds[_levelType][g_pObjectProperties->levelSpecs[_levelType][_levelID - 1].endBackground]);
	newRoom->battle = true;
	newRoom->background.SetPos(position, 0);
	newRoom->boss = true;
	newRoom->lootable = nullptr;
	for(int i = 0; i < 4; i++)
		newRoom->enemyIds[i] = g_pObjectProperties->levelSpecs[_levelType][_levelID - 1].bossGroup[i];
	newLevel->AddRoom(newRoom);

	return newLevel;
}
