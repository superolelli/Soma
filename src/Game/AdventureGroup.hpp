#pragma once

#include "Player.hpp"

enum player {anna, simon, markus, ole};

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