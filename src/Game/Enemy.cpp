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

	combatantObject->setCurrentAnimation("idle");
	combatantObject->setScale(SpriterEngine::point(ENEMY_SCALE, ENEMY_SCALE));
	combatantObject->reprocessCurrentTime();

	SpriterEngine::UniversalObjectInterface* hitboxObj = combatantObject->getObjectInstance("bounding_box");

	hitbox.width = hitboxObj->getSize().x * combatantObject->getScale().x;
	hitbox.height = hitboxObj->getSize().y * combatantObject->getScale().y;
	hitbox.left = hitboxObj->getPosition().x;
	hitbox.top = hitboxObj->getPosition().y;

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
		{
			ChooseAbility();
			ChooseTarget();
			abilityAnnouncementTime = 3.0f;
		}
		else
		{
			abilityAnnouncementTime -= g_pTimer->GetElapsedTime().asSeconds();

			if (abilityAnnouncementTime <= 0.0f)
			{
				StartAbilityAnimation();
				StartTargetsAttackedAnimation();
				abilityStatus = executing;
			}
		}
	}

	if (abilityStatus == executing)
	{
		if(combatantObject->animationJustFinished())
		{
			combatantObject->setCurrentAnimation("idle");
			combatantObject->setScale(SpriterEngine::point(0.6, 0.6));
			combatantObject->setPosition(lastPosition);
			StopTargetsAttackedAnimation();

			DoAbility(gui->GetCurrentAbility(), *allCombatants);
			abilityStatus = finished;
		}
	}
}

void Enemy::Render()
{
	if (abilityStatus == executing || abilityStatus == attacked)
		combatantObject->setTimeElapsed(ENEMY_ABILITY_ANIMATION_SPEED);
	else
		combatantObject->setTimeElapsed(ENEMY_IDLE_ANIMATION_SPEED);

	combatantObject->render();

	if (abilityAnnouncementTime > 0.0f)
	{
		g_pSpritePool->abilityAnnouncementBanner.SetPos(engine->GetWindow().getView().getCenter().x + 100.0f, 150.0f);
		g_pSpritePool->abilityAnnouncementBanner.ChangeString(0, GetChosenAbilityName());
		g_pSpritePool->abilityAnnouncementBanner.SetTextPosCentered(0);
		g_pSpritePool->abilityAnnouncementBanner.Render(engine->GetWindow());
	}
}



void Enemy::StartAbilityAnimation()
{
	combatantObject->setCurrentAnimation("bang");
	combatantObject->setCurrentTime(0);

	lastPosition = combatantObject->getPosition();

	combatantObject->setScale(SpriterEngine::point(0.8, 0.8));
	combatantObject->setPosition(SpriterEngine::point(900, 800));
	//combatantObject->setPosition(SpriterEngine::point(int(lastPosition.x) % engine->GetWindowSize().x, 800));
}

