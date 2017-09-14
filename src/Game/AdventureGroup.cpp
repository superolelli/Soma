#include "AdventureGroup.hpp"




void AdventureGroup::Init()
{
	int x = 100;
	for (auto &a : adventurer)
	{
		a.Init();
		a.SetPos(x, 700);

		x += 150;
	}
}


void AdventureGroup::Quit()
{
	for (auto &a : adventurer)
		a.Quit();
}

void AdventureGroup::Update(int xMove)
{
	for (auto &a : adventurer)
	{
		a.Update(xMove, xMove != 0);
	}
	
}


void AdventureGroup::Render()
{
	for (auto &a : adventurer)
		a.Render();
}

