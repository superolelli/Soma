#pragma once

#include "Ole.hpp"
#include "Anna.hpp"
#include "Markus.hpp"
#include "Simon.hpp"

#include "GameStatus.hpp"

const int PLAYER_SPACING = 160;
const int GROUP_Y_POS = 750;
const int GROUP_OFFSET_LEFT = 100;


class AdventureGroup
{
public:

	void Init(CGameEngine *_engine, NotificationRenderer *_notificationRenderer, GameStatus *_gameStatus);
	void Quit();
	void Update(int xMove);
	void Render();

	bool IsDead();
	Player *GetPlayer(int _id) { return adventurer[_id]; }

private:

	Player *adventurer[4];
};