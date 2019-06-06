#include "Battle.hpp"
#include "Greg.hpp"


void Battle::Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine, NotificationRenderer *_notificationRenderer, int enemyIDs[4], bool _boss)
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
			if(enemyIDs[i] == CombatantID::Greg)
				enemy[i] = new GregDigger(enemyIDs[i], _engine, _notificationRenderer);
			else
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
			combatants[currentCombatant]->GiveTurnTo(&combatants, gui);

			if (isBossBattle)
				g_pVideos->PlayVideo(videoId::introGreg);
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
				if(!(*i)->IsPlayer())
					InformGregAboutDeath();
			}
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

void Battle::InformGregAboutDeath()
{
	for (auto c : combatants)
	{
		if (c->GetID() == CombatantID::Greg)
			dynamic_cast<GregDigger*>(c)->SetCompanionDiedLastRound(true);
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

