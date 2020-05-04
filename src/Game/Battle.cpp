#include "Battle.hpp"


void Battle::Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine, NotificationRenderer *_notificationRenderer, int enemyIDs[4], bool _boss)
{
	g_pMusic->StopMusic();

	players = _adventureGroup;
	gui = _gui;
	engine = _engine;
	notificationRenderer = _notificationRenderer;
	isBattleFinished = false;
	turnsSinceLastEnemyDied = -1;
	isBossBattle = _boss;

	InitCombatants(_xView, enemyIDs);

	InitNewRound();

	gui->SetCombatantToDisplay(combatants[currentCombatant]);

	if (combatants[currentCombatant]->IsPlayer())
		gui->SetCurrentPlayer(combatants[currentCombatant]);

	combatants[currentCombatant]->GiveTurnTo(&combatants, gui);
}



void Battle::InitCombatants(int _xView, int enemyIDs[4])
{
	for (int i = 0; i < 4; i++)
	{
		if (players->GetPlayer(i)->Status().GetCurrentHealth() > 0)
		{
			combatants.push_back(players->GetPlayer(i));
			combatants.back()->SetBattlePos(i);
			combatants.back()->SetBattlePtr(this);
		}
	}

	int pos = _xView + ENEMY_X_OFFSET;
	for (int i = 0; i < 4; i++)
	{
		enemies[i] = nullptr;
		if (enemyIDs[i] != CombatantID::Undefined) {
			enemies[i] = CreateEnemy(enemyIDs[i]);
			enemies[i]->SetPos(pos - enemies[i]->GetLocalPosition().x, ENEMY_Y_POS);
			pos += enemies[i]->GetRect().width + ENEMY_SPACING;
			enemies[i]->SetBattlePos(i + 4);
			enemies[i]->Update();
			combatants.push_back(enemies[i]);
			combatants.back()->SetBattlePtr(this);
		}
	}
}


void Battle::Quit()
{
	for (Combatant *c : combatants)
	{
		c->ResetAbilityStatus();
		c->SetTurnPending(false);
		c->SetBattlePtr(nullptr);

		if (!c->IsPlayer()) {
			c->Quit();
			SAFE_DELETE(c);
		}
	}
}


void Battle::AddEnemy(int enemyID)
{
	int battlePosition = GetEmptyEnemyBattlePosition();
	if (battlePosition != -1)
	{
		enemies[battlePosition - 4] = CreateEnemy(enemyID);
		enemies[battlePosition - 4]->SetBattlePos(battlePosition);
		combatants.insert(combatants.begin(), enemies[battlePosition - 4]);
		currentCombatant++;
		RecalculateEnemyPositions();
		enemies[battlePosition - 4]->Combatant::Update();
	}
}


Enemy *Battle::CreateEnemy(int _enemyID)
{
	Enemy *enemy;
	enemy = new Enemy(_enemyID, engine, notificationRenderer);
	enemy->Init();
	return enemy;
}


int Battle::GetEmptyEnemyBattlePosition()
{
	for (int i = 0; i < 4; i++)
	{
		if (enemies[i] == nullptr)
			return i + 4;
	}

	return -1;
}


void Battle::RecalculateEnemyPositions()
{
	int xPos = engine->GetWindow().getView().getCenter().x + ENEMY_X_OFFSET;
	int width = 100;
	for (auto e : enemies)
	{
		if (e != nullptr)
		{
			e->SetPos(xPos - e->GetLocalPosition().x, ENEMY_Y_POS);
			width = e->GetRect().width;
		}

		xPos += width + ENEMY_SPACING;
	}
}


void Battle::Update()
{
	for (int i = 0; i < 4; i++)
	{
		if(enemies[i] != nullptr)
			enemies[i]->Update();
	}

	HandleDeaths();

	if (IsOneGroupDead() == true)
	{
		isBattleFinished = true;
		return;
	}

	if (combatants[currentCombatant]->FinishedTurn() && !IsOneGroupDying())
		GiveTurnToNextCombatant();

	SetCombatantToDisplayForGUI();
}



void Battle::GiveTurnToNextCombatant()
{
	do {
		ChooseNextCombatant();
	} while (combatants[currentCombatant]->Status().GetCurrentHealth() <= 0);

	combatants[currentCombatant]->GiveTurnTo(&combatants, gui);
	if (combatants[currentCombatant]->IsPlayer())
		gui->SetCurrentPlayer(combatants[currentCombatant]);
	else
		gui->SetCurrentPlayer(nullptr);
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
			if (!(*i)->IsDying()) 
			{
				(*i)->SetAbilityStatus(dying);
				if (!(*i)->IsPlayer())
					turnsSinceLastEnemyDied = 0;
			}
			else if ((*i)->AnimationFinished())
			{
				int battlePos = (*i)->GetBattlePos();
				if (combatantNumber < currentCombatant)
					currentCombatant--;

				if (!(*i)->IsPlayer())
				{
					(*i)->Quit();
					SAFE_DELETE(enemies[battlePos - 4]);
				}

				i = combatants.erase(i);
				continue;
			}
		}

		i++;
	}
}


bool Battle::EnemyDiedLastRound()
{
	return turnsSinceLastEnemyDied >= 0 && turnsSinceLastEnemyDied <= 1;
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


bool Battle::IsOneGroupDying()
{
	int PlayersAlive = 0;
	int EnemiesAlive = 0;

	for (int i = 0; i < 4; i++)
	{
		if (players->GetPlayer(i)->Status().GetCurrentHealth() > 0)
			PlayersAlive++;
	}

	for (Enemy *e : enemies)
	{
		if (e != nullptr && e->Status().GetCurrentHealth() > 0)
			EnemiesAlive++;
	}

	return PlayersAlive == 0 || EnemiesAlive == 0;
}


void Battle::ChooseNextCombatant()
{
	currentCombatant++;

	if (currentCombatant >= combatants.size())
	{
		if(turnsSinceLastEnemyDied >= 0)
			turnsSinceLastEnemyDied++;
		InitNewRound();
	}
}

void Battle::InitNewRound()
{
	CalculateTurnOrder();
	currentCombatant = 0;

	for (auto c : combatants)
		c->SetTurnPending(true);
}

void Battle::CalculateTurnOrder()
{
	std::sort(combatants.begin(), combatants.end(), [](Combatant *c1, Combatant *c2) {
		return c1->Status().GetInitiative() > c2->Status().GetInitiative();
	});

	auto startIt = combatants.begin();
	auto endIt = combatants.begin();

	do
	{
		while (endIt != combatants.end() && (*startIt)->Status().GetInitiative() == (*endIt)->Status().GetInitiative())
			endIt++;

		std::random_shuffle(startIt, endIt);

		startIt = endIt;
	} while (endIt != combatants.end());
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