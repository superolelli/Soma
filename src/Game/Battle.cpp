#include "Battle.hpp"



void Battle::Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine)
{
	srand(time(0));
	enemy[3].Init(4, _engine);
	enemy[2].Init(4, _engine);
	enemy[1].Init(5, _engine);
	enemy[0].Init(5, _engine);

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

	for (int i = 0; i < 8; i++)
		combatants[i]->SetBattlePos(i);

	currentCombatant = 0;

	isBattleFinished = false;

	gui->SetCurrentPlayer(combatants[currentCombatant]->GetID());

	combatants[currentCombatant]->GiveTurnTo(&combatants, gui);
}


void Battle::Quit()
{
	for (Enemy &e : enemy)
		e.Quit();
}



void Battle::Update()
{
	for (Combatant *c : combatants)
	{
		if (c != nullptr)
			c->Update();
	}

	if (combatants[currentCombatant]->FinishedTurn())
	{
		HandleDeaths();

		if (IsOneGroupDead() == true)
			isBattleFinished = true;

		ChooseNextCombatant();

		gui->SetCurrentPlayer(combatants[currentCombatant]->GetID());

		combatants[currentCombatant]->GiveTurnTo(&combatants, gui);
	}
}



void Battle::HandleDeaths()
{
	for (int i = 0; i < 8; i++)
	{
		if (combatants[i] != nullptr && combatants[i]->Status().GetCurrentHealth() <= 0)
			combatants[i] = nullptr;
	}
}


bool Battle::IsOneGroupDead()
{
	int PlayersAlive = 0;
	int EnemiesAlive = 0;
	for (int i = 0; i < 8; i++)
	{
		if (combatants[i] != nullptr)
		{
			if (combatants[i]->IsPlayer())
				PlayersAlive++;
			else
				EnemiesAlive++;
		}
	}

	if (PlayersAlive == 0 || EnemiesAlive == 0)
		return true;	

	return false;
}



void Battle::ChooseNextCombatant()
{
	do {
		currentCombatant++;

		if (currentCombatant == 8)
		{
			CalculateTurnOrder();
			currentCombatant = 0;
		}
	} while (combatants[currentCombatant] == nullptr);
}



void Battle::CalculateTurnOrder()
{
	std::sort(combatants.begin(), combatants.end(), [](Combatant *c1, Combatant *c2) {
		if (c1 == nullptr)
			return false;
		else if (c2 == nullptr)
			return true;
		else
			return c1->Status().GetInitiative() > c2->Status().GetInitiative();
	});
}



void Battle::Render()
{
	for (int i = 0; i < 8; i++)
	{
		if (combatants[i] != nullptr)
		{
			combatants[i]->Render();
			combatants[i]->RenderHealthBar(engine->GetWindow());
		}
	}
}

