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

	attributes.armour = 1;
	attributes.currentHealth = 20;
	attributes.maxHealth = 20;
	attributes.damage = 5;
	attributes.initiative = 1;

	healthBar.Load(g_pTextures->healthBar, g_pTextures->healthBarFrame, &attributes.currentHealth, &attributes.maxHealth);
	healthBar.SetPos(GetRect().left + GetRect().width / 2 - healthBar.GetRect().width / 2, GetRect().top + GetRect().height + 30);

	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 8; i++)
			possibleAbilityAims[j].position[i] = true;

		possibleAbilityAims[j].howMany = 1;
	}
}



bool Enemy::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	std::cout << "Der Gegner schießt!" << std::endl;

	_targets[rand() % 4]->LooseHealth(attributes.damage);

	return true;
}

void Enemy::Render()
{
	combatantObject->setTimeElapsed(ENEMY_ANIMATION_SPEED);
	combatantObject->render();
}