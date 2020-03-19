#include "Level.hpp"


Level::Level()
{	
	battle = false;
	bossBattle = false;
	currentRoomNumber = 0;
	currentThirdLayer1 = rand() % NUMBER_OF_THIRD_LAYER_BACKGROUNDS;
	currentThirdLayer2 = rand() % NUMBER_OF_THIRD_LAYER_BACKGROUNDS;
	currentSecondLayer1 = rand() % NUMBER_OF_SECOND_LAYER_BACKGROUNDS;
	currentSecondLayer2 = rand() % NUMBER_OF_SECOND_LAYER_BACKGROUNDS;
}

Level::~Level()
{
	for (auto room : rooms)
	{
		SAFE_DELETE(room->lootable);
		SAFE_DELETE(room);
	}
}



void Level::Update(int _playerPos, CGameEngine *_engine)
{
	battle = false;
	currentRoomNumber = (_playerPos - rooms[0]->background.GetRect().width / 2) / rooms[0]->background.GetRect().width;

	if (rooms[currentRoomNumber]->battle == true)
	{
		battle = true;
		rooms[currentRoomNumber]->battle = false;
		currentEnemyIDs[0] = rooms[currentRoomNumber]->enemyIds[0];
		currentEnemyIDs[1] = rooms[currentRoomNumber]->enemyIds[1];
		currentEnemyIDs[2] = rooms[currentRoomNumber]->enemyIds[2];
		currentEnemyIDs[3] = rooms[currentRoomNumber]->enemyIds[3];
	}
	if (rooms[currentRoomNumber]->boss)
		bossBattle = true;
	else
		bossBattle = false;

	UpdateLootables(_engine);
}


void Level::UpdateLootables(CGameEngine *_engine)
{
	if (rooms[currentRoomNumber]->lootable != nullptr)
		rooms[currentRoomNumber]->lootable->Update(_engine);

	if (rooms.size() > currentRoomNumber + 1)
	{
		if (rooms[currentRoomNumber + 1]->lootable != nullptr)
			rooms[currentRoomNumber + 1]->lootable->Update(_engine);
	}
}


int *Level::GetEnemyIDs()
{
	return currentEnemyIDs;
}


void Level::Render(sf::RenderTarget &_target, int _viewX)
{
	RenderBackground(_target, _viewX);
	RenderLootables(_target);
}


void Level::RenderLootables(sf::RenderTarget &_target)
{
	if (rooms[currentRoomNumber]->lootable != nullptr)
		rooms[currentRoomNumber]->lootable->Render(_target);

	if (rooms.size() > currentRoomNumber + 1)
	{
		if (rooms[currentRoomNumber + 1]->lootable != nullptr)
			rooms[currentRoomNumber + 1]->lootable->Render(_target);
	}
}


void Level::AddRoom(Room *_room)
{
	rooms.push_back(_room);
}


int Level::IsAtEnd(int _playerPos)
{
	return _playerPos >= rooms.size() * rooms[0]->background.GetRect().width - rooms[0]->background.GetRect().width / 2;
}


void Level::RenderBackground(sf::RenderTarget &_target, int _viewX)
{
	//Render fourth Layer
	g_pSpritePool->bangBackgroundFourthLayer.SetPos(_viewX - (_viewX / 9) % g_pSpritePool->bangBackgroundFourthLayer.GetRect().width, 0);
	g_pSpritePool->bangBackgroundFourthLayer.Render(_target);
	g_pSpritePool->bangBackgroundFourthLayer.SetPos(g_pSpritePool->bangBackgroundFourthLayer.GetRect().left + g_pSpritePool->bangBackgroundFourthLayer.GetRect().width, 0);
	g_pSpritePool->bangBackgroundFourthLayer.Render(_target);

	//Render third Layer
	int newX = _viewX - (_viewX / 6) % g_pSpritePool->bangBackgroundThirdLayers[currentThirdLayer1].GetRect().width;
	if (newX >= g_pSpritePool->bangBackgroundThirdLayers[currentThirdLayer1].GetRect().left + g_pSpritePool->bangBackgroundThirdLayers[currentThirdLayer1].GetRect().width)
	{
		currentThirdLayer1 = currentThirdLayer2;
		currentThirdLayer2 = rand() % NUMBER_OF_THIRD_LAYER_BACKGROUNDS;
	}

	g_pSpritePool->bangBackgroundThirdLayers[currentThirdLayer2].SetPos(newX + g_pSpritePool->bangBackgroundThirdLayers[currentThirdLayer1].GetRect().width, 0);
	g_pSpritePool->bangBackgroundThirdLayers[currentThirdLayer2].Render(_target);
	g_pSpritePool->bangBackgroundThirdLayers[currentThirdLayer1].SetPos(newX, 0);
	g_pSpritePool->bangBackgroundThirdLayers[currentThirdLayer1].Render(_target);


	//Render second Layer
	newX = _viewX - (_viewX / 3) % g_pSpritePool->bangBackgroundSecondLayers[currentSecondLayer1].GetRect().width;
	if (newX >= g_pSpritePool->bangBackgroundSecondLayers[currentSecondLayer1].GetRect().left + g_pSpritePool->bangBackgroundSecondLayers[currentSecondLayer1].GetRect().width)
	{
		currentSecondLayer1 = currentSecondLayer2;
		currentSecondLayer2 = rand() % NUMBER_OF_SECOND_LAYER_BACKGROUNDS;
	}

	g_pSpritePool->bangBackgroundSecondLayers[currentSecondLayer2].SetPos(newX + g_pSpritePool->bangBackgroundSecondLayers[currentSecondLayer1].GetRect().width, 0);
	g_pSpritePool->bangBackgroundSecondLayers[currentSecondLayer2].Render(_target);
	g_pSpritePool->bangBackgroundSecondLayers[currentSecondLayer1].SetPos(newX, 0);
	g_pSpritePool->bangBackgroundSecondLayers[currentSecondLayer1].Render(_target);


	// Render first Layer
	if (rooms.size() > currentRoomNumber)
	{
		rooms[currentRoomNumber]->background.Render(_target);

		if (rooms.size() > currentRoomNumber + 1)
			rooms[currentRoomNumber + 1]->background.Render(_target);
	}
}