#include "Battle.hpp"



void Battle::Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine, NotificationRenderer *_notificationRenderer)
{
	srand(time(0));
	enemy[3].Init(4, _engine, _notificationRenderer);
	enemy[2].Init(4, _engine, _notificationRenderer);
	enemy[1].Init(5, _engine, _notificationRenderer);
	enemy[0].Init(5, _engine, _notificationRenderer);

	int pos = _xView + ENEMY_X_OFFSET;
	for (auto &e : enemy)
	{
		e.SetPos(pos, ENEMY_Y_POS);
		pos += ENEMY_SPACING;
	}

	players = _adventureGroup;
	gui = _gui;
	engine = _engine;

	combatants.push_back(players->GetPlayer(0));
	combatants.push_back(players->GetPlayer(1));
	combatants.push_back(players->GetPlayer(2));
	combatants.push_back(players->GetPlayer(3));
	combatants.push_back(&enemy[0]);
	combatants.push_back(&enemy[1]);
	combatants.push_back(&enemy[2]);
	combatants.push_back(&enemy[3]);


	for (int i = 0; i < 8; i++)
		combatants[i]->SetBattlePos(i);

	CalculateTurnOrder();

	currentCombatant = 0;

	isBattleFinished = false;

	gui->SetCurrentPlayer(combatants[currentCombatant]->GetID());

	combatants[currentCombatant]->GiveTurnTo(&combatants, gui);
}


void Battle::Quit()
{
	for (Combatant *c : combatants)
		c->ResetAbilityStatus();

	for (Enemy &e : enemy)
		e.Quit();
}



void Battle::Update()
{
	for (Combatant *c : combatants)
		c->Update();

	if (combatants[currentCombatant]->FinishedTurn())
	{
		HandleDeaths();

		if (IsOneGroupDead() == true)
			isBattleFinished = true;

		do {
			ChooseNextCombatant();
		} while (combatants[currentCombatant]->Status().GetCurrentHealth() <= 0);

		gui->SetCurrentPlayer(combatants[currentCombatant]->GetID());

		combatants[currentCombatant]->GiveTurnTo(&combatants, gui);
	}
}



void Battle::HandleDeaths()
{
	std::vector<Combatant*>::iterator i;
	for (i = combatants.begin(); i != combatants.end();)
	{
		if ((*i)->Status().GetCurrentHealth() <= 0)
		{
			if(!(*i)->IsDying())
				(*i)->StartDeathAnimation();
			else if ((*i)->AnimationFinished())
			{
				i = combatants.erase(i);
				continue;
			}
		}

		i++;
	}
}


bool Battle::IsOneGroupDead()
{
	int PlayersAlive = 0;
	int EnemiesAlive = 0;

	for (Combatant* c : combatants)
	{
		if (c->IsPlayer())
			PlayersAlive++;
		else
			EnemiesAlive++;
	}

	return PlayersAlive == 0 || EnemiesAlive == 0;
}



void Battle::ChooseNextCombatant()
{
	currentCombatant++;

	if (currentCombatant >= combatants.size())
	{
		CalculateTurnOrder();
		currentCombatant = 0;
	}
}



void Battle::CalculateTurnOrder()
{
	std::sort(combatants.begin(), combatants.end(), [](Combatant *c1, Combatant *c2) {
			return c1->Status().GetInitiative() > c2->Status().GetInitiative();
	});
}



void Battle::Render()
{
	Combatant::setElapsedTimeForAbilityEffect = false;
	for (Combatant* c : combatants)
	{
		if (c->GetAbilityStatus() != executing && c->GetAbilityStatus() != attacked)
		{
			c->Render();
			c->RenderHealthBar(engine->GetWindow());
		}
	}
}



void Battle::RenderAbilityAnimations()
{
	for (Combatant* c : combatants)
	{
		if(c->GetAbilityStatus() == executing || c->GetAbilityStatus() == attacked)
			c->Render();
	}
}

