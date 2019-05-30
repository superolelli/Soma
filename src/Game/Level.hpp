#pragma once

#include "../Framework/Graphics/Sprite.hpp"
#include "../Game/Resources/Resources.hpp"
#include "CombatantID.hpp"

struct Room {
	CSprite background;
	bool battle;
	bool boss;
	int enemyIds[4];
};

class Level
{
public:

	Level();
	~Level();

	void Update(int _playerPos);
	void Render(sf::RenderTarget &_target, int _viewX);

	void AddRoom(Room *_room);

	bool InBattle() { return battle; }

	int *GetEnemyIDs();
	int IsAtEnd(int _playerPos);

private:

	std::vector<Room*> rooms;

	void RenderBackground(sf::RenderTarget &_target, int _viewX);

	bool battle;
	int currentEnemyIDs[4];
};