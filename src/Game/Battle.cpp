#include "Battle.hpp"
#include <algorithm>
#include <random>

Battle::Battle(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine, NotificationRenderer *_notificationRenderer, CombatantID enemyIDs[4], bool _boss, LevelStatus *_levelStatus)
	: players(_adventureGroup)
	, gui(_gui)
	, engine(_engine)
	, notificationRenderer(_notificationRenderer)
	, isBattleFinished(false)
	, turnsSinceLastEnemyDied(-1)
	, isBossBattle(_boss)
	, levelStatus(_levelStatus)
{
	g_pMusic->StopMusic();

	InitCombatants(_xView, enemyIDs);
	InitNewRound();

	gui->SetCombatantToDisplay(combatants[currentCombatant]);

	if (combatants[currentCombatant]->IsPlayer())
		gui->SetCurrentPlayer(combatants[currentCombatant]);

	combatants[currentCombatant]->GiveTurnTo(&combatants, gui);
}



void Battle::InitCombatants(int _xView, CombatantID enemyIDs[4])
{
	for (int i = 0; i < 4; i++)
	{
		if (players->GetPlayer(i)->Status().GetAttribute("currentHealth") > 0)
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


Battle::~Battle()
{
	for (Combatant *c : combatants)
	{
		c->ResetAbilityStatus();
		c->SetTurnPending(false);
		c->SetBattlePtr(nullptr);

		if (!c->IsPlayer()) {
			SAFE_DELETE(c);
		}
	}
}


void Battle::AddEnemy(CombatantID enemyID)
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


Enemy *Battle::CreateEnemy(CombatantID _enemyID)
{
	return new Enemy(_enemyID, engine, notificationRenderer);
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
	int xPos = engine->GetRenderTarget().getView().getCenter().x + ENEMY_X_OFFSET;
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
	} while (combatants[currentCombatant]->Status().GetAttribute("currentHealth") <= 0);

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
		if ((*i)->Status().GetAttribute("currentHealth") <= 0)
		{
			if (!(*i)->IsDying()) 
			{
				(*i)->SetAbilityStatus(dying);
				if ((*i)->IsPlayer())
					levelStatus->PlayerDied();
				else
				{
					levelStatus->OpponentKilled();
					turnsSinceLastEnemyDied = 0;
				}
			}
			else if ((*i)->AnimationFinished())
			{
				int battlePos = (*i)->GetBattlePos();
				if (combatantNumber < currentCombatant)
					currentCombatant--;

				/*if (!(*i)->IsPlayer())
				{
					(*i)->Quit();
					SAFE_DELETE(enemies[battlePos - 4]);
				}*/

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
		if (players->GetPlayer(i)->Status().GetAttribute("currentHealth") > 0)
			PlayersAlive++;
	}

	for (Enemy *e : enemies)
	{
		if (e != nullptr && e->Status().GetAttribute("currentHealth") > 0)
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
		return c1->Status().GetAttribute("initiative") > c2->Status().GetAttribute("initiative");
	});

	auto startIt = combatants.begin();
	auto endIt = combatants.begin();

	std::random_device rd;
	std::mt19937 g(rd());
	do
	{
		while (endIt != combatants.end() && (*startIt)->Status().GetAttribute("initiative") == (*endIt)->Status().GetAttribute("initiative"))
			endIt++;

		std::shuffle(startIt, endIt, g);

		startIt = endIt;
	} while (endIt != combatants.end());
}



void Battle::Render()
{
	Combatant::setElapsedTimeForAbilityEffect = false;
	for (Combatant *c : combatants)
	{
		if (c->GetAbilityStatus() != executing && c->GetAbilityStatus() != attacked && c->GetAbilityStatus() != dodging)
			c->Render();
	}

	for (Combatant* c : combatants)
	{
		if (c->GetAbilityStatus() != executing && c->GetAbilityStatus() != attacked && c->GetAbilityStatus() != dodging && c->GetAbilityStatus() != dying)
			c->RenderStatusBar();
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

bool Battle::CurrentlyExecutingAbility()
{
	for (Combatant* c : combatants)
	{
		if (c->GetAbilityStatus() == executing || c->GetAbilityStatus() == attacked || c->GetAbilityStatus() == dodging)
			return true;
	}

	return false;
}
