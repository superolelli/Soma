#include "AdventureGroup.hpp"




void AdventureGroup::Init()
{
	int x = 100;
	int id = 0;
	for (auto &a : adventurer)
	{
		a.Init(id);
		a.SetPos(x, 700);

		x += 150;
		id += 1;
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

