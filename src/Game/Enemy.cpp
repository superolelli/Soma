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
	combatantObject->setScale(SpriterEngine::point(0.6, 0.6));
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

	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 8; i++)
			possibleAbilityAims[j].position[i] = true;

		possibleAbilityAims[j].howMany = 1;
	}
}



bool Enemy::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	for (Combatant* t : _targets)
		std::cout << "Der Gegner schießt!" << std::endl;

	return true;
}

void Enemy::Render()
{
	combatantObject->setTimeElapsed(10);
	combatantObject->render();
}