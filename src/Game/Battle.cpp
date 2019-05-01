#include "Battle.hpp"



void Battle::Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine, NotificationRenderer *_notificationRenderer, int enemyIDs[4])
{
	players = _adventureGroup;
	gui = _gui;
	engine = _engine;

	for(int i = 0; i < 4; i++)
	{
		if (players->GetPlayer(i)->Status().GetCurrentHealth() > 0)
		{
			combatants.push_back(players->GetPlayer(i));
			combatants.back()->SetBattlePos(i);
		}
	}

	int pos = _xView + ENEMY_X_OFFSET;
	for (int i = 0; i < 4; i++)
	{
		if (enemyIDs[i] != CombatantID::Undefined) {
			enemy[i] = new Enemy(enemyIDs[i], _engine, _notificationRenderer);
			enemy[i]->Init();
			enemy[i]->SetPos(pos - enemy[i]->GetLocalPosition().x, ENEMY_Y_POS);
			pos += enemy[i]->GetRect().width + ENEMY_SPACING;
			enemy[i]->SetBattlePos(i + 4);
			combatants.push_back(enemy[i]);
		}
	}

	CalculateTurnOrder();

	currentCombatant = 0;

	isBattleFinished = false;

	gui->SetCombatantToDisplay(combatants[currentCombatant]);

	combatants[currentCombatant]->GiveTurnTo(&combatants, gui);
}


void Battle::Quit()
{
	for (Combatant *c : combatants)
		c->ResetAbilityStatus();

	for (Enemy *e : enemy)
	{
		e->Quit();
		SAFE_DELETE(e);
	}
}



void Battle::Update()
{
	for (Combatant *c : combatants)
		c->Update();

	HandleDeaths();

	if (IsOneGroupDead() == true)
		isBattleFinished = true;

	if (combatants[currentCombatant]->FinishedTurn())
	{
		do {
			ChooseNextCombatant();
		} while (combatants[currentCombatant]->Status().GetCurrentHealth() <= 0);

		combatants[currentCombatant]->GiveTurnTo(&combatants, gui);
	}

	SetCombatantToDisplayForGUI();
}


void Battle::SetCombatantToDisplayForGUI()
{
	bool combatantSet = false;
	for (auto c : combatants)
	{
		if (c->GetRect().contains(engine->GetWorldMousePos()))
		{
			gui->SetCombatantToDisplay(c);
			combatantSet = true;
			break;
		}
	}

	if (!combatantSet)
		gui->SetCombatantToDisplay(combatants[currentCombatant]);
}


void Battle::HandleDeaths()
{
	int combatantNumber = 0;
	std::vector<Combatant*>::iterator i;
	for (i = combatants.begin(); i != combatants.end(); combatantNumber++)
	{
		if ((*i)->Status().GetCurrentHealth() <= 0)
		{
			if(!(*i)->IsDying())
				(*i)->StartDeathAnimation();
			else if ((*i)->AnimationFinished())
			{
				if (combatantNumber < currentCombatant)
					currentCombatant--;
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
		if (c->GetAbilityStatus() != executing && c->GetAbilityStatus() != attacked && c->GetAbilityStatus() != dodging)
			c->Render();
	}
}



void Battle::RenderAbilityAnimations()
{
	for (Combatant* c : combatants)
	{
		if(c->GetAbilityStatus() == executing || c->GetAbilityStatus() == attacked || c->GetAbilityStatus() == dodging)
			c->Render();
	}
}

