#include "LevelBuilder.hpp"


Level *LevelBuilder::buildLevel(int _levelID)
{
	srand(time(0));
	Level *newLevel = new Level;

	int position = 0;
	for (int i = 0; i < g_pObjectProperties->levelSpecs.numberOfRooms - 1; i++)
	{
		Room *newRoom = new Room;
		newRoom->battle = rand() % 100 < g_pObjectProperties->levelSpecs.battleProbability * 100.0f;
		if (newRoom->battle)
		{
			int enemyGroup = rand() % g_pObjectProperties->levelSpecs.possibleEnemyGroups.size();
			for (int j = 0; j < 4; j++)
				newRoom->enemyIds[j] = g_pObjectProperties->levelSpecs.possibleEnemyGroups[enemyGroup][j];
		}

		auto backgroundID = rand() % g_pObjectProperties->levelSpecs.possibleBackgrounds.size();
		newRoom->background.Load(g_pTextures->bangBackgrounds[g_pObjectProperties->levelSpecs.possibleBackgrounds[backgroundID]]);
		newRoom->background.SetPos(position, 0);
		newRoom->boss = false;
		position += newRoom->background.GetRect().width;

		newLevel->AddRoom(newRoom);
	}

	//add end room
	Room *newRoom = new Room;
	newRoom->battle = true;
	newRoom->background.Load(g_pTextures->bangBackgrounds[g_pObjectProperties->levelSpecs.endBackground]);
	newRoom->background.SetPos(position, 0);
	newRoom->boss = true;
	for(int i = 0; i < 4; i++)
		newRoom->enemyIds[i] = g_pObjectProperties->levelSpecs.bossGroup[i];
	newLevel->AddRoom(newRoom);

	return newLevel;
}
