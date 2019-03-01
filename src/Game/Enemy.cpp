#include "Enemy.hpp"




void Enemy::Init(int _id, CGameEngine * _engine, NotificationRenderer *_notificationRenderer)
{
	switch (_id)
	{
	case 4:
		combatantObject = g_pModels->modelGesetzloser->getNewEntityInstance("Gesetzloser");
		break;
	case 5:
		combatantObject = g_pModels->modelAbtruenniger->getNewEntityInstance("Abtruenniger");
		break;
	}

	Combatant::Init(_id, _engine, _notificationRenderer);

	enemyID = _id;
	
	status.SetStats(g_pObjectProperties->enemyStats[enemyID]);

	abilityAnnouncementTime = 0.0f;
	confusionChecked = false;
}



void Enemy::ChooseAbility()
{
	if (enemyID == 4)
		chosenAbility = enemyAbilities::springfield;
	else
		chosenAbility = enemyAbilities::bang;
}



void Enemy::ChooseTarget()
{
	selectedTargets.clear();

	CheckForMarkedPlayers();

	if (selectedTargets.empty())
	{
		ChooseRandomPlayer();
	}
}




void Enemy::CheckForMarkedPlayers()
{
	for (Combatant* c : (*allCombatants))
	{
		if (c->IsPlayer() && c->Status().IsMarked() && CanAimAtCombatant(c))
		{
            if (rand() % 10 < 8)
            {
                selectedTargets.push_back(c);
                return;
            }
		}
	}
}


bool Enemy::CanAimAtCombatant(Combatant *_combatant)
{
	return g_pObjectProperties->enemyAbilities[chosenAbility].possibleAims.position[_combatant->GetBattlePos()];
}



void Enemy::ChooseRandomPlayer()
{
	int numberOfPlayers = std::accumulate((*allCombatants).begin(), (*allCombatants).end(), 0, [&](int sum, Combatant* c) {if (c->IsPlayer() && CanAimAtCombatant(c)) return sum + 1; else return sum; });

	if (numberOfPlayers > 0)
	{
		int target = rand() % numberOfPlayers;
		for (Combatant* c : (*allCombatants))
		{
			if (c->IsPlayer() && CanAimAtCombatant(c))
			{
				if (target == 0)
				{
					selectedTargets.push_back(c);
					return;
				}
				target--;
			}
		}
	}
	else
	{
		abilityStatus = finished;
	}
}


void Enemy::ChooseRandomEnemy()
{
	int numberOfEnemies = std::accumulate((*allCombatants).begin(), (*allCombatants).end(), 0, [&](int sum, Combatant *c) {if (!c->IsPlayer())return sum + 1; else return sum; });

    if (numberOfEnemies > 0)
    {
		int target = rand() % (numberOfEnemies-1);
		for (Combatant *c : (*allCombatants))
		{
			if (!c->IsPlayer() && c != this)
			{
				if (target == 0)
				{
					selectedTargets.push_back(c);
					return;
				}
				target--;
			}
		}
    }
	else
	{
		abilityStatus = finished;
	}
}


bool Enemy::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	for (Combatant *t : selectedTargets)
	{
		if (!t->IsPlayer() || actsInConfusion)
			ApplyAbilityEffectToTarget(t, g_pObjectProperties->enemyAbilities[chosenAbility].effectFriendly);
		else if(t->GetAbilityStatus() != dodging)
			ApplyAbilityEffectToTarget(t, g_pObjectProperties->enemyAbilities[chosenAbility].effectHostile);
	}

	return true;
}





void Enemy::Update()
{
	Combatant::Update();

	if (abilityStatus == ready)
	{
		if (confusionChecked == false)
		{
			if (status.IsConfused() && rand() % 10 < 5)
			{
				selectedTargets.clear();
				ChooseAbility();
				ChooseRandomEnemy();
				notificationRenderer->AddNotification("Verwirrt!", g_pFonts->f_kingArthur, sf::Vector2f(GetRect().left - GetRect().width/2.0f, GetRect().top - 20.0f), 1.0f);
				abilityAnnouncementTime = 2.0f;
				actsInConfusion = true;
			}

			confusionChecked = true;
		}
		else
		{
			if (abilityAnnouncementTime <= 0.0f)
				PrepareAbility();
			
			AnnounceAndStartAbilityAnimation();
		}
	}

	if (abilityStatus == executing)
	{
		if(!combatantObject->animationIsPlaying() && !AbilityEffectIsPlaying())
			ExecuteAbility();
	}
}



void Enemy::PrepareAbility()
{
	ChooseAbility();
	ChooseTarget();
	abilityAnnouncementTime = 2.0f;
}


void Enemy::AnnounceAndStartAbilityAnimation()
{
	abilityAnnouncementTime -= g_pTimer->GetElapsedTime().asSeconds();

	if (abilityAnnouncementTime <= 0.0f)
	{
		StartAbilityAnimation(chosenAbility);
		StartTargetsAttackedAnimation();
		abilityStatus = executing;
	}
}


void Enemy::ExecuteAbility()
{
	DoAbility(gui->GetCurrentAbility(), *allCombatants);

	SetAnimation("idle", IDLE_ANIMATION_SPEED);
	ReverseScaleForAbilityAnimation();
	StopTargetsAttackedAnimation();
	abilityStatus = finished;
	confusionChecked = false;
	actsInConfusion = false;
}


void Enemy::Render()
{
	combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	combatantObject->render();
	combatantObject->playSoundTriggers();

	if (abilityAnnouncementTime > 0.0f && abilityAnnouncementTime < 1.0f && !actsInConfusion)
		RenderAbilityAnnouncement();

	if (abilityStatus == ready)
	{
		RenderTurnMarker();

		if(abilityAnnouncementTime >= 0.0f)
			RenderAbilityTargetMarker();
	}

	if (abilityStatus == attacked || abilityStatus == dodging)
		RenderAbilityEffects();

	if (abilityStatus != executing && abilityStatus != attacked && abilityStatus != dodging)
		statusBar.Render();
}



void Enemy::RenderAbilityAnnouncement()
{
	g_pSpritePool->abilityAnnouncementBanner.SetPos(engine->GetWindow().getView().getCenter().x + 100.0f, 150.0f);
	g_pSpritePool->abilityAnnouncementBanner.ChangeString(0, GetChosenAbilityName());
	g_pSpritePool->abilityAnnouncementBanner.SetTextPosCentered(0);
	g_pSpritePool->abilityAnnouncementBanner.Render(engine->GetWindow());
}


void Enemy::RenderAbilityTargetMarker()
{
	for (Combatant *c : selectedTargets)
		c->RenderAbilityTargetMarker();	
}


void Enemy::StartAbilityAnimation(int _ability)
{
	ScaleForAbilityAnimation();                           //order is relevant because "bang" has no hitbox (needed for animation calculation)

	SetAnimation(g_pObjectProperties->enemyAbilities[_ability].animation, ABILITY_ANIMATION_SPEED);

	if (!selectedTargets[0]->IsPlayer())
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(g_pObjectProperties->enemyAbilities[_ability].effectAnimationFriendly);
	else
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(g_pObjectProperties->enemyAbilities[_ability].effectAnimationHostile);

	g_pSpritePool->abilityEffectsAnimation->setCurrentTime(0);
}

