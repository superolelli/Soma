#pragma once

#include "Ole.hpp"
#include "Anna.hpp"
#include "Markus.hpp"
#include "Simon.hpp"

const int PLAYER_SPACING = 150;
const int GROUP_Y_POS = 700;
const int GROUP_OFFSET_LEFT = 100;


class AdventureGroup
{
public:

	void Init(CGameEngine *_engine);
	void Quit();
	void Update(int xMove);
	void Render();

	void RenderWithHealthBars(sf::RenderTarget &_target);

	Player *GetPlayer(int _id) { return adventurer[_id]; }

private:

	Player *adventurer[4];
};