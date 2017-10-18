#pragma once

#include "Player.hpp"
#include "PlayerEnum.hpp"

class AdventureGroup
{
public:

	void Init();
	void Quit();
	void Update(int xMove);
	void Render();

private:

	Player adventurer[4];
};