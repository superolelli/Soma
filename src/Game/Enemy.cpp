#include "Enemy.hpp"




void Enemy::Init(int _id, CGameEngine * _engine)
{
	engine = _engine;

	switch (_id)
	{
	case 4:
		combatantObject = g_pModels->modelGesetzloser->getNewEntityInstance("Gesetzloser");
		break;
	case 5:
		combatantObject = g_pModels->modelAbtruenniger->getNewEntityInstance("Abtruenniger");
		break;
	}

	SetAnimation("idle", IDLE_ANIMATION_SPEED);
	Scale(ENEMY_SCALE, ENEMY_SCALE);
	combatantObject->reprocessCurrentTime();

	ReloadHitbox();

	status.Reset();

	CombatantAttributes attributes;
	attributes.armour = 1;
	attributes.currentHealth = 20;
	attributes.maxHealth = 20;
	attributes.damage = 5;
	attributes.initiative = 1;
	attributes.dexterity = 5;

	status.SetAttributes(attributes);
	
	healthBar.Load(g_pTextures->healthBar, g_pTextures->healthBarFrame, status.GetCurrentHealthPointer(), status.GetMaxHealthPointer());
	healthBar.SetPos(GetRect().left + GetRect().width / 2 - healthBar.GetRect().width / 2, GetRect().top + GetRect().height + 30);
	healthBar.SetSmoothTransformationTime(0.7f);

	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 8; i++)
			possibleAbilityAims[j].position[i] = true;

		possibleAbilityAims[j].howMany = 1;
	}


	abilityAnnouncementTime = 0.0f;
	abilityStatus = finished;
}



void Enemy::ChooseAbility()
{
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
		if (c->IsPlayer() && c->Status().IsMarked())
		{
			selectedTargets.push_back(c);
			return;
		}
	}
}



void Enemy::ChooseRandomPlayer()
{
	int numberOfPlayers = std::accumulate((*allCombatants).begin(), (*allCombatants).end(), 0, [](int sum, Combatant* c) {if (c->IsPlayer()) return sum + 1; else return sum; });

	int target = rand() % numberOfPlayers;
	for (Combatant* c : (*allCombatants))
	{
		if (c->IsPlayer())
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


bool Enemy::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	//check for confusion
	if (status.IsConfused())
	{
		if (rand() % 4 == 0)
			status.LooseHealth(1);
	}

	selectedTargets[0]->Status().LooseHealth(status.GetDamage());

	return true;
}



void Enemy::Update()
{
	Combatant::Update();

	if (abilityStatus == ready)
	{
		if (abilityAnnouncementTime <= 0.0f)
			PrepareAbility();
		else
			AnnounceAndStartAbilityAnimation();
	}

	if (abilityStatus == executing)
	{
		if(!combatantObject->animationIsPlaying())
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
		StartAbilityAnimation();
		StartTargetsAttackedAnimation();
		abilityStatus = executing;
	}
}


void Enemy::ExecuteAbility()
{
	SetAnimation("idle", IDLE_ANIMATION_SPEED);
	ReverseScaleForAbilityAnimation();
	StopTargetsAttackedAnimation();

	DoAbility(gui->GetCurrentAbility(), *allCombatants);
	abilityStatus = finished;
}


void Enemy::Render()
{
	combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	combatantObject->render();

	if (abilityAnnouncementTime > 0.0f && abilityAnnouncementTime < 1.0f)
		RenderAbilityAnnouncement();

	if (abilityStatus == attacked)
		RenderAbilityEffects();
}



void Enemy::RenderAbilityAnnouncement()
{
	g_pSpritePool->abilityAnnouncementBanner.SetPos(engine->GetWindow().getView().getCenter().x + 100.0f, 150.0f);
	g_pSpritePool->abilityAnnouncementBanner.ChangeString(0, GetChosenAbilityName());
	g_pSpritePool->abilityAnnouncementBanner.SetTextPosCentered(0);
	g_pSpritePool->abilityAnnouncementBanner.Render(engine->GetWindow());
}



void Enemy::StartAbilityAnimation()
{
	ScaleForAbilityAnimation();                           //order is relevant because "bang" has no hitbox (needed for animation calculation)
	SetAnimation("bang", ABILITY_ANIMATION_SPEED);

	g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation("bang");
	g_pSpritePool->abilityEffectsAnimation->setCurrentTime(0);
}

