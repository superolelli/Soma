#include "Battle.hpp"



void Battle::Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine)
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
	gui = _gui;
	engine = _engine;

	isBattleFinished = false;

	currentPlayer = Simon;
	gui->SetCurrentPlayer((PlayerID)currentPlayer);
	abilityStatus = ready;
}


void Battle::Quit()
{
	for (auto e : enemy)
		SAFE_DELETE(e);
}



void Battle::Update()
{
	if (abilityStatus == finished)
	{
		currentPlayer++;
		if (currentPlayer > 3)
			currentPlayer = 0;
		gui->SetCurrentPlayer((PlayerID)currentPlayer);

		abilityStatus = ready;
	}
	else if (abilityStatus == aimed)
	{
		   //do the ability
			abilityStatus = finished;
	}
	else
	{
		if(AimChosen())
			abilityStatus = aimed; 
	}
	
}



bool Battle::AimChosen()
{
	if (engine->GetButtonstates(ButtonID::Left) == Keystates::Released)
	{
		for (int i = 0; i < 8; i++)
		{
			if (g_pAbilities->abilityAims[currentPlayer][gui->GetCurrentAbility()].position[i] == true)
				if (CompetitantClicked(i))
					return true;
		}
	}

	return false;
}



bool Battle::CompetitantClicked(int _id)
{
	if (_id >= 0 && _id < 4)
	{
		sf::IntRect rect;
		rect.left = enemy[_id]->getPosition().x;
		rect.top = enemy[_id]->getPosition().y - 200;
		rect.width = 150;
		rect.height = 400;		

		if (rect.contains(engine->GetWorldMousePos()))
			return true;
	}
	else
		return false;

	return false;
}



void Battle::Render()
{
	for (auto e : enemy)
	{
		e->setTimeElapsed(10);
		e->render();
	}
}