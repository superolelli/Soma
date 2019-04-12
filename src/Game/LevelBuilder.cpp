#include "LevelBuilder.hpp"


Level *LevelBuilder::buildLevel(int _levelID)
{
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
		position += newRoom->background.GetRect().width;

		newLevel->AddRoom(newRoom);
	}

	//add end room
	Room *newRoom = new Room;
	newRoom->battle = false;
	newRoom->background.Load(g_pTextures->bangBackgrounds[g_pObjectProperties->levelSpecs.endBackground]);
	newRoom->background.SetPos(position, 0);
	newLevel->AddRoom(newRoom);

	return newLevel;
}
