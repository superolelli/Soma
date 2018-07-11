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

	//check for marked players
	for (int i = 0; i < 8; i++)
	{
		if ((*allCombatants)[i] != nullptr && (*allCombatants)[i]->IsPlayer())
		{
			if ((*allCombatants)[i]->Status().IsMarked())
			{
				chosenTarget = (*allCombatants)[i];
				return;
			}
		}
	}


	int target = rand() % 4;
	do {
		for (int i = 0; i < 8; i++)
		{
			if ((*allCombatants)[i] != nullptr && (*allCombatants)[i]->IsPlayer())
			{
				if (target == 0)
				{
					chosenTarget = (*allCombatants)[i];
					return;
				}
				target--;
			}
		}
	} while (target >= 0);
}




bool Enemy::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	std::cout << "Der Gegner schießt!" << std::endl;

	//check for confusion
	if (status.IsConfused())
	{
		if (rand() % 4 == 0)
			status.LooseHealth(1);
	}

	chosenTarget->Status().LooseHealth(status.GetDamage());

	return true;
}



void Enemy::Update()
{
	Combatant::Update();

	if (abilityStatus == ready)
	{
		ChooseAbility();
		abilityStatus = executing;
		abilityAnnouncementTime = 3.0f;
	}

	if (abilityStatus == executing)
	{
		if (abilityAnnouncementTime > 0.0f)
		{
			abilityAnnouncementTime -= g_pTimer->GetElapsedTime().asSeconds();
		}
		else
		{
			DoAbility(gui->GetCurrentAbility(), *allCombatants);
			abilityStatus = finished;
		}
	}
}

void Enemy::Render()
{
	combatantObject->setTimeElapsed(ENEMY_ANIMATION_SPEED);
	combatantObject->render();

	if (abilityAnnouncementTime > 0.0f)
	{
		g_pSpritePool->abilityAnnouncementBanner.SetPos(engine->GetWindow().getView().getCenter().x + 100.0f, 150.0f);
		g_pSpritePool->abilityAnnouncementBanner.ChangeString(0, GetChosenAbilityName());
		g_pSpritePool->abilityAnnouncementBanner.SetTextPosCentered(0);
		g_pSpritePool->abilityAnnouncementBanner.Render(engine->GetWindow());
	}
}


