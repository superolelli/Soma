#include "Battle.hpp"



void Battle::Init(int _xView)
{
	enemyObject[3] = g_pModels->modelGesetzloser->getNewEntityInstance("Gesetzloser");
	enemyObject[2] = g_pModels->modelGesetzloser->getNewEntityInstance("Gesetzloser");
	enemyObject[1] = g_pModels->modelAbtruenniger->getNewEntityInstance("Abtruenniger");
	enemyObject[0] = g_pModels->modelAbtruenniger->getNewEntityInstance("Abtruenniger");
	int pos = _xView + 50;

	for (auto e : enemyObject)
	{
		e->setCurrentAnimation("idle");
		e->setScale(SpriterEngine::point(0.6, 0.6));
		e->setPosition(SpriterEngine::point(pos, 700));
		e->reprocessCurrentTime();
		pos += 200;
	}

	isBattleFinished = false;
}


void Battle::Quit()
{
	for (auto e : enemyObject)
		SAFE_DELETE(e);
}

void Battle::Update()
{
	
}


void Battle::Render()
{
	for (auto e : enemyObject)
	{
		e->setTimeElapsed(10);
		e->render();
	}
}