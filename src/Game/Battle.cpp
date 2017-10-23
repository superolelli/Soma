#include "Battle.hpp"



void Battle::Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine)
{
	enemy[3].Init(4);
	enemy[2].Init(4);
	enemy[1].Init(5);
	enemy[0].Init(5);

	int pos = _xView + 50;
	for (auto &e : enemy)
	{
		e.SetPos(pos, 700);
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
				if (CombatantClicked(i))
					return true;
		}
	}

	return false;
}



bool Battle::CombatantClicked(int _id)
{
	if (_id >= 0 && _id < 4)
	{
		if (enemy[_id].GetRect().contains(engine->GetWorldMousePos()))
			return true;
	}
	else
		return false;

	return false;
}



void Battle::Render()
{
	for (auto &e : enemy)
		e.Render();
}