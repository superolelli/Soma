#include "Level.hpp"


Level::Level()
{	
	int position = 0;
	battle = false;
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

	if (rooms[_playerPos / (rooms[0]->background.GetRect().width)]->battle == 1)
	{
		battle = true;
		rooms[_playerPos / (rooms[0]->background.GetRect().width)]->battle = 0;
		currentEnemyIDs[0] = rooms[_playerPos / (rooms[0]->background.GetRect().width)]->enemyIds[0];
		currentEnemyIDs[1] = rooms[_playerPos / (rooms[0]->background.GetRect().width)]->enemyIds[1];
		currentEnemyIDs[2] = rooms[_playerPos / (rooms[0]->background.GetRect().width)]->enemyIds[2];
		currentEnemyIDs[3] = rooms[_playerPos / (rooms[0]->background.GetRect().width)]->enemyIds[3];
	}
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