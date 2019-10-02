#include "Battle.hpp"
#include "Greg.hpp"
#include "ObserverNotificationBattle.h"
#include "ApacheKid.hpp"


void Battle::Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine, NotificationRenderer *_notificationRenderer, int enemyIDs[4], bool _boss)
{
	g_pMusic->StopMusic();

	players = _adventureGroup;
	gui = _gui;
	engine = _engine;
	notificationRenderer = _notificationRenderer;

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
		enemies[i] = nullptr;
		if (enemyIDs[i] != CombatantID::Undefined) {
			Enemy *enemy;
			if (enemyIDs[i] == CombatantID::Greg) {
				enemy = new GregDigger(enemyIDs[i], _engine, _notificationRenderer);
				AddObserver(enemy);
			}
			else if (enemyIDs[i] == CombatantID::Apachekid) {
				enemy = new ApacheKid(enemyIDs[i], _engine, _notificationRenderer);
				dynamic_cast<ApacheKid*>(enemy)->SetCallbackForAddingEnemy([&](int id) {AddEnemy(id); });
			}
			else
				enemy = new Enemy(enemyIDs[i], _engine, _notificationRenderer);

			enemies[i] = enemy;
			enemy->Init();
			enemy->SetPos(pos - enemy->GetLocalPosition().x, ENEMY_Y_POS);
			pos += enemy->GetRect().width + ENEMY_SPACING;
			enemy->SetBattlePos(i + 4);
			combatants.push_back(enemy);
		}
	}

	CalculateTurnOrder();

	currentCombatant = 0;

	isBattleFinished = false;
	isPlayingIntro = true;
	isBossBattle = _boss;
	finishedCycles = 0;
	afterIntroWaitingTime = 2.0f;

	gui->SetCombatantToDisplay(combatants[currentCombatant]);

	if (combatants[currentCombatant]->IsPlayer())
		gui->SetCurrentPlayer(combatants[currentCombatant]);

	g_pSpritePool->newBattleAnimation->setCurrentAnimation("new_battle");
	g_pSpritePool->newBattleAnimation->setCurrentTime(0);
	g_pSpritePool->newBattleAnimation->reprocessCurrentTime();
	g_pSpritePool->newBattleAnimation->setPosition(SpriterEngine::point(_xView - 300, engine->GetWindowSize().y / 2 - 30));
}


void Battle::Quit()
{
	for (Combatant *c : combatants)
	{
		c->ResetAbilityStatus();

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
		Enemy *enemy;
		if (enemyID == CombatantID::Greg) {
			enemy = new GregDigger(enemyID, engine, notificationRenderer);
			AddObserver(enemy);
		}
		else if (enemyID == CombatantID::Apachekid) {
			enemy = new ApacheKid(enemyID, engine, notificationRenderer);
			dynamic_cast<ApacheKid*>(enemy)->SetCallbackForAddingEnemy([&](int id) {AddEnemy(id); });
		}
		else
			enemy = new Enemy(enemyID, engine, notificationRenderer);

		enemies[battlePosition - 4] = enemy;
		enemy->Init();
		enemy->SetBattlePos(battlePosition);
		combatants.insert(combatants.begin(), enemy);
		currentCombatant++;
		RecalculateEnemyPositions();
		enemy->Combatant::Update();
	}
}


int Battle::GetEmptyEnemyBattlePosition()
{
	int battlePosition;

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

	if (!isPlayingIntro)
	{
		HandleDeaths();

		if (IsOneGroupDead() == true)
			isBattleFinished = true;

		if (combatants[currentCombatant]->FinishedTurn())
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

		SetCombatantToDisplayForGUI();
	}
	else
		HandleIntro();
}

void Battle::HandleIntro()
{
	if (g_pSpritePool->newBattleAnimation->animationIsPlaying() == false)
	{
		if (afterIntroWaitingTime > 0.0f)
			afterIntroWaitingTime -= g_pTimer->GetElapsedTime().asSeconds();
		else
		{
			isPlayingIntro = false;
			g_pMusic->SetBattleStarted();
			combatants[currentCombatant]->GiveTurnTo(&combatants, gui);

			if (isBossBattle)
			{
				if(enemies[3]->GetID() == CombatantID::Greg)
					g_pVideos->PlayVideo(videoId::introGreg);
				else if(enemies[3]->GetID() == CombatantID::Apachekid)
					g_pVideos->PlayVideo(videoId::introApacheKid);
			}
		}
	} 
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
			if (!(*i)->IsDying()) {
				(*i)->StartDeathAnimation();
				if (!(*i)->IsPlayer())
				{
					Notify(ObserverNotificationBattle{battleEvents::enemyDied});
				}
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

	if (isPlayingIntro)
	{
		if (finishedCycles >= 2)
		{
			g_pSpritePool->newBattleAnimation->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
			g_pSpritePool->newBattleAnimation->render();
			g_pSpritePool->newBattleAnimation->playSoundTriggers();
		}
		else
			finishedCycles++;
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

