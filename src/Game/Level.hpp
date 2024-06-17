#pragma once

#include "../Framework/Graphics/Sprite.hpp"
#include "../Game/Resources/Resources.hpp"
#include "CombatantID.hpp"
#include "Lootable.hpp"

#include "LevelStatus.hpp"

struct Room {
	Room(sf::Texture const &_backgroundTexture) : background(_backgroundTexture){}
	CSprite background;
	bool battle;
	bool boss;
	int enemyIds[4];
	Lootable *lootable;
};

class Level
{
public:

	Level(LevelType _levelType, int _levelID);
	~Level();

	void Update(int _playerPos, CGameEngine *_engine);
	void Render(sf::RenderTarget &_target, int _viewX);

	LevelStatus& GetLevelStatus() { return levelStatus; }

	void AddRoom(Room *_room);

	bool InBattle() { return battle; }
	bool IsBossBattle() { return bossBattle; }

	int *GetEnemyIDs();
	int IsAtEnd(int _playerPos);

private:

	std::vector<Room*> rooms;
	LevelType levelType;

	void RenderBackground(sf::RenderTarget &_target, int _viewX);
	void RenderLootables(sf::RenderTarget &_target);
	void UpdateLootables(CGameEngine *_engine);

	bool battle;
	bool bossBattle;
	int currentEnemyIDs[4];

	int currentRoomNumber;

	int currentThirdLayer1;
	int currentThirdLayer2;
	int currentSecondLayer1;
	int currentSecondLayer2;

	LevelStatus levelStatus;
};