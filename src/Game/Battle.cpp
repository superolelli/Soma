#include "Battle.hpp"



void Battle::Init(int _xView, AdventureGroup *_adventureGroup)
{
	enemy[3] = g_pModels->modelGesetzloser->getNewEntityInstance("Gesetzloser");
	enemy[2] = g_pModels->modelGesetzloser->getNewEntityInstance("Gesetzloser");
	enemy[1] = g_pModels->modelAbtruenniger->getNewEntityInstance("Abtruenniger");
	enemy[0] = g_pModels->modelAbtruenniger->getNewEntityInstance("Abtruenniger");
	int pos = _xView + 50;

	for (auto e : enemy)
	{
		e->setCurrentAnimation("idle");
		e->setScale(SpriterEngine::point(0.6, 0.6));
		e->setPosition(SpriterEngine::point(pos, 700));
		e->reprocessCurrentTime();
		pos += 200;
	}

	players = _adventureGroup;
	//gui = _gui;

	isBattleFinished = false;
}


void Battle::Quit()
{
	for (auto e : enemy)
		SAFE_DELETE(e);
}

void Battle::Update()
{
	
}


void Battle::Render()
{
	for (auto e : enemy)
	{
		e->setTimeElapsed(10);
		e->render();
	}
}