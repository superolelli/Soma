#include "Level.hpp"


Level::Level()
{	
	battle = false;
	bossBattle = false;
	currentRoomNumber = 0;
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
	if (rooms.size() > currentRoomNumber)
	{
		rooms[currentRoomNumber]->background.Render(_target);

		if (rooms.size() > currentRoomNumber + 1)
			rooms[currentRoomNumber + 1]->background.Render(_target);
	}
}