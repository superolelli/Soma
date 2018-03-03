#pragma once

#include "Ole.hpp"
#include "Anna.hpp"
#include "Markus.hpp"
#include "Simon.hpp"

class AdventureGroup
{
public:

	void Init();
	void Quit();
	void Update(int xMove);
	void Render();

	void RenderWithHealthBars(sf::RenderTarget &_target);

	Player *GetPlayer(int _id) { return adventurer[_id]; }

private:

	Player *adventurer[4];
};