#include "AdventureGroup.hpp"




void AdventureGroup::Init()
{
	adventurer[PlayerID::Ole] = new PlayerOle;
	adventurer[PlayerID::Ole]->Init(PlayerID::Ole);

	adventurer[PlayerID::Anna] = new PlayerAnna;
	adventurer[PlayerID::Anna]->Init(PlayerID::Anna);

	adventurer[PlayerID::Simon] = new PlayerSimon;
	adventurer[PlayerID::Simon]->Init(PlayerID::Simon);

	adventurer[PlayerID::Markus] = new PlayerMarkus;
	adventurer[PlayerID::Markus]->Init(PlayerID::Markus);


	int x = 100;
	for (auto a : adventurer)
	{
		a->SetPos(x, 700);
		x += 150;
	}
}


void AdventureGroup::Quit()
{
	for (auto a : adventurer)
	{
		a->Quit();
		SAFE_DELETE(a);
	}
}

void AdventureGroup::Update(int xMove)
{
	for (auto a : adventurer)
	{
		a->Update(xMove, xMove != 0);
	}
	
}


void AdventureGroup::Render()
{
	for (auto a : adventurer)
		a->Render();
}

