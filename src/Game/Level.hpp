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

	void SetReward(LevelReward _reward) { reward = _reward; };
	LevelReward &GetReward() { return reward; }

	bool InBattle() { return battle; }
	bool IsBossBattle() { return bossBattle; }

	int *GetEnemyIDs();
	int IsAtEnd(int _playerPos);

private:

	std::vector<Room*> rooms;

	void RenderBackground(sf::RenderTarget &_target, int _viewX);

	bool battle;
	bool bossBattle;
	int currentEnemyIDs[4];

	LevelReward reward;
};