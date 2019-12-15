#include "Enemy.hpp"
#include "Greg.hpp"


Enemy::Enemy(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Combatant(_id, _engine, _notificationRenderer)
{
	enemyID = _id;
}

void Enemy::Init()
{
	switch (enemyID)
	{
	case CombatantID::Gesetzloser:
		combatantObject = g_pModels->modelGesetzloser->getNewEntityInstance("Gesetzloser");
		break;
	case CombatantID::Abtruenniger:
		combatantObject = g_pModels->modelAbtruenniger->getNewEntityInstance("Abtruenniger");
		break;
	case CombatantID::Indianer:
		combatantObject = g_pModels->modelIndianer->getNewEntityInstance("Indianer");
		break;
	case CombatantID::Hilfssheriff:
		combatantObject = g_pModels->modelHilfssheriff->getNewEntityInstance("Hilfssheriff");
		break;
	case CombatantID::Greg:
		combatantObject = g_pModels->modelGreg->getNewEntityInstance("Greg");
		break;
	case CombatantID::Apachekid:
		combatantObject = g_pModels->modelApacheKid->getNewEntityInstance("ApacheKid");
		break;
	}
	
	status.SetStats(g_pObjectProperties->enemyStats[enemyID]);

	Combatant::Init();

	abilityAnnouncementTime = 0.0f;
	confusionChecked = false;
}



void Enemy::ChooseAbility()
{
	switch (enemyID) 
	{
	case CombatantID::Gesetzloser:
		chosenAbility = enemyAbilities::springfield;
		break;
	case CombatantID::Indianer:
		chosenAbility = enemyAbilities::tomahawk;
		break;
	}
}



void Enemy::ChooseTarget()
{
	selectedTargets.clear();

	if (chosenAbility == enemyAbilities::bury_the_dead || chosenAbility == enemyAbilities::indians)
		selectedTargets.push_back(this);
	else {
		CheckForMarkedPlayers();

		if (selectedTargets.empty())
		{
			ChooseRandomPlayer();
		}
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
	return g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.position[_combatant->GetBattlePos()] && !_combatant->IsDying();
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
					SelectAdditionalPlayers();
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


void Enemy::SelectAdditionalPlayers()
{
	int targetPosition = selectedTargets[0]->GetBattlePos();

	for (Combatant* c : (*allCombatants))
	{
		if (g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackAll && this != c  && selectedTargets[0] != c || c->GetBattlePos() < targetPosition && c->GetBattlePos() > targetPosition - g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.howMany)
			selectedTargets.push_back(c);
	}
}


void Enemy::SelectAdditionalEnemies()
{
	int targetPosition = selectedTargets[0]->GetBattlePos();

	for (Combatant* c : (*allCombatants))
	{
		if (g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackAll && this != c  && selectedTargets[0] != c || c->GetBattlePos() > targetPosition && c->GetBattlePos() < targetPosition + g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.howMany)
			selectedTargets.push_back(c);
	}
}

void Enemy::ChooseRandomEnemy()
{
	int numberOfEnemies = std::accumulate((*allCombatants).begin(), (*allCombatants).end(), 0, [&](int sum, Combatant *c) {if (!c->IsPlayer() && c->Status().GetCurrentHealth() > 0)return sum + 1; else return sum; });

    if (numberOfEnemies > 0)
    {
		int target = rand() % (numberOfEnemies-1);
		for (Combatant *c : (*allCombatants))
		{
			if (!c->IsPlayer() && c != this && !c->IsDying())
			{
				if (target == 0)
				{
					selectedTargets.push_back(c);
					SelectAdditionalEnemies();
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
	auto &ability = g_pObjectProperties->enemyAbilities[int(chosenAbility)];
	for (Combatant *t : selectedTargets)
	{
		if (!t->IsPlayer() || actsInConfusion)
		{
			float additionalDamage = ability.effectFriendly.lessTargetsMoreDamage * (ability.possibleAims.howMany - selectedTargets.size());
			ApplyAbilityEffectToTarget(t, ability.effectFriendly, additionalDamage);
		}
		else if (t->GetAbilityStatus() != dodging)
		{
			float additionalDamage = ability.effectHostile.lessTargetsMoreDamage * (ability.possibleAims.howMany - selectedTargets.size());
			ApplyAbilityEffectToTarget(t, ability.effectHostile, additionalDamage);
		}
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
		StartAbilityAnimation(int(chosenAbility));
		StartTargetsAttackedAnimation(g_pObjectProperties->playerAbilities[GetID()][int(chosenAbility)].precisionModificator);
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
	if (abilityStatus != executing && abilityStatus != attacked && abilityStatus != dodging)
		RenderShadow();

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

	if (abilityStatus != executing && abilityStatus != attacked && abilityStatus != dodging && !IsDying())
		statusBar.Render();
}



void Enemy::RenderAbilityAnnouncement()
{
	g_pSpritePool->abilityAnnouncementBanner.SetPos(engine->GetWindow().getView().getCenter().x + 100.0f, 150.0f);
	g_pSpritePool->abilityAnnouncementBanner.ChangeString(0, GetChosenAbilityName());
	g_pSpritePool->abilityAnnouncementBanner.SetTextPosCentered(0);
	g_pSpritePool->abilityAnnouncementBanner.MoveText(0, 0, -20);
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

