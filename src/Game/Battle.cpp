#include "Battle.hpp"



void Battle::Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine)
{
	srand(time(0));
	enemy[3].Init(4);
	enemy[2].Init(4);
	enemy[1].Init(5);
	enemy[0].Init(5);

	int pos = _xView + ENEMY_X_OFFSET;
	for (auto &e : enemy)
	{
		e.SetPos(pos, ENEMY_Y_POS);
		pos += ENEMY_SPACING;
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

	currentCombatant = 0;

	isBattleFinished = false;

	gui->SetCurrentPlayer(combatants[currentCombatant]->GetID());

	abilityStatus = ready;
	selectedTarget = 0;
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
		if (combatants[currentCombatant]->IsPlayer())
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
		if (combatants[currentCombatant]->IsPlayer())
		{
			std::vector<Combatant*> targets;
			for (int i = selectedTarget; i < combatants[currentCombatant]->possibleAbilityAims[gui->GetCurrentAbility()].howMany + selectedTarget && i < 8; i++)
			{
				targets.push_back(combatants[i]);
			}
			combatants[currentCombatant]->DoAbility(gui->GetCurrentAbility(), targets);
		}
		else
		{
			combatants[currentCombatant]->DoAbility(gui->GetCurrentAbility(), combatants);
		}		

		abilityStatus = finished;
	}
	else if (abilityStatus == finished)
	{
		currentCombatant++;

		if (currentCombatant == 7)
		{
			CalculateTurnOrder();
			currentCombatant = 0;
		}

		gui->SetCurrentPlayer(combatants[currentCombatant]->GetID());

		abilityStatus = ready;
	}
	
}

void Battle::CalculateTurnOrder()
{
	std::sort(combatants.begin(), combatants.end(), [](Combatant *c1, Combatant *c2) {return c1->GetAttributes().initiative > c2->GetAttributes().initiative; });
}



bool Battle::AimChosen()
{
	if (engine->GetButtonstates(ButtonID::Left) == Keystates::Released)
	{
		for (int i = 0; i < 8; i++)
		{
			if (combatants[currentCombatant]->possibleAbilityAims[gui->GetCurrentAbility()].position[i] == true)
				if (CombatantClicked(i))
				{
					selectedTarget = i;
					return true;
				}
				
		}
	}

	return false;
}



bool Battle::CombatantClicked(int _id)
{
	return combatants[_id]->GetRect().contains(engine->GetWorldMousePos());
}



void Battle::Render()
{
	for (auto &e : enemy)
	{
		e.Render();
		e.RenderHealthBar(engine->GetWindow());
	}
}