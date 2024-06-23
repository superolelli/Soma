#pragma once

#include "GameStatus.hpp"
#include "Player.hpp"

const int PLAYER_SPACING = 160;
const int GROUP_Y_POS = 750;
const int GROUP_OFFSET_LEFT = 100;


class AdventureGroup
{
public:

	AdventureGroup(CGameEngine *_engine, NotificationRenderer *_notificationRenderer);
	~AdventureGroup();

	void Update(int xMove);
	void Render();

	void SetFatigueLevelOfAllPlayers(CombatantStatus::FatigueLevel _level);

	bool IsDead();
	Player *GetPlayer(int _id) { return adventurer[_id]; }

private:

	Player *adventurer[4];
};