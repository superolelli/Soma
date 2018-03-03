#include "Battle.hpp"



void Battle::Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine)
{
	srand(time(0));
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

	combatants.push_back(&enemy[0]);
	combatants.push_back(&enemy[1]);
	combatants.push_back(&enemy[2]);
	combatants.push_back(&enemy[3]);
	combatants.push_back(players->GetPlayer(0));
	combatants.push_back(players->GetPlayer(1));
	combatants.push_back(players->GetPlayer(2));
	combatants.push_back(players->GetPlayer(3));

	CalculateTurnOrder();

	currentCombatant = combatants.begin();

	isBattleFinished = false;

	gui->SetCurrentPlayer((*currentCombatant)->GetID());

	abilityStatus = ready;
	selectedTarget = nullptr;
}


void Battle::Quit()
{
	for (Enemy &e : enemy)
		e.Quit();
}



void Battle::Update()
{
	if (abilityStatus == ready)
	{
		if ((*currentCombatant)->IsPlayer())
		{
			if (AimChosen())
				abilityStatus = aimed;
		}
		else
			abilityStatus = aimed;
	}
	else if (abilityStatus == aimed)
	{
		//do the ability
		std::vector<Combatant*> targets;
		if ((*currentCombatant)->IsPlayer())
		{
			targets.push_back(selectedTarget);
		}
		else
		{
			targets.push_back(*combatants.begin());
		}		
		
		(*currentCombatant)->DoAbility(gui->GetCurrentAbility(), targets);

		abilityStatus = finished;
	}
	else if (abilityStatus == finished)
	{
		currentCombatant++;

		if (currentCombatant == combatants.end())
		{
			CalculateTurnOrder();
			currentCombatant = combatants.begin();
		}

		gui->SetCurrentPlayer((*currentCombatant)->GetID());

		abilityStatus = ready;
	}
	
}

void Battle::CalculateTurnOrder()
{
	combatants.sort([](Combatant *c1, Combatant *c2) {return c1->GetAttributes().initiative > c2->GetAttributes().initiative;});
}



bool Battle::AimChosen()
{
	if (engine->GetButtonstates(ButtonID::Left) == Keystates::Released)
	{
		for (int i = 0; i < 8; i++)
		{
			if ((*currentCombatant)->possibleAbilityAims[gui->GetCurrentAbility()].position[i] == true)
				if (CombatantClicked(i))
				{
					selectedTarget = &enemy[i];
					return true;
				}
				
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

	return false;
}



void Battle::Render()
{
	for (auto &e : enemy)
	{
		e.Render();
		e.RenderHealthBar(engine->GetWindow());
	}
}