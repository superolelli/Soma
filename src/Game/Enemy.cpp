#include "Enemy.hpp"




void Enemy::Init(int _id)
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
}



void Enemy::ChooseAbility(std::vector<Combatant*> &_targets)
{
	chosenAbility = enemyAbilities::bang;

	//check for marked players
	for (int i = 0; i < 8; i++)
	{
		if (_targets[i] != nullptr && _targets[i]->IsPlayer())
		{
			if (_targets[i]->Status().IsMarked())
			{
				chosenTarget = _targets[i];
				return;
			}
		}
	}


	int target = rand() % 4;
	do {
		for (int i = 0; i < 8; i++)
		{
			if (_targets[i] != nullptr && _targets[i]->IsPlayer())
			{
				if (target == 0)
				{
					chosenTarget = _targets[i];
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



void Enemy::Render()
{
	combatantObject->setTimeElapsed(ENEMY_ANIMATION_SPEED);
	combatantObject->render();
}