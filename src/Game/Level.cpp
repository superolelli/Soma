#include "Level.hpp"


Level::Level()
{	
	int position = 0;
	battle = false;
	bossBattle = false;
}

Level::~Level()
{
	for (auto room : rooms)
	{
		SAFE_DELETE(room);
	}
}



void Level::Update(int _playerPos)
{
	battle = false;
	int roomNumber = (_playerPos - rooms[0]->background.GetRect().width / 2) / rooms[0]->background.GetRect().width;

	if (rooms[roomNumber]->battle == true)
	{
		battle = true;
		rooms[roomNumber]->battle = false;
		currentEnemyIDs[0] = rooms[roomNumber]->enemyIds[0];
		currentEnemyIDs[1] = rooms[roomNumber]->enemyIds[1];
		currentEnemyIDs[2] = rooms[roomNumber]->enemyIds[2];
		currentEnemyIDs[3] = rooms[roomNumber]->enemyIds[3];
	}
	if (rooms[roomNumber]->boss)
		bossBattle = true;
	else
		bossBattle = false;
}


int *Level::GetEnemyIDs()
{
	return currentEnemyIDs;
}


void Level::Render(sf::RenderTarget &_target, int _viewX)
{
	RenderBackground(_target, _viewX);
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
	int roomNumber = _viewX / rooms[0]->background.GetRect().width;

	if (rooms.size() > roomNumber)
	{
		rooms[roomNumber]->background.Render(_target);

		if (rooms.size() > roomNumber + 1)
			rooms[roomNumber + 1]->background.Render(_target);
	}
}