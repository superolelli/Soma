#include "Simon.hpp"



void PlayerSimon::Init(int _id)
{
	combatantObject = g_pModels->modelSimon->getNewEntityInstance("Simon");

	attributes.armour = 1;
	attributes.currentHealth = 20;
	attributes.maxHealth = 20;
	attributes.damage = 5;
	attributes.initiative = 2;

	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 8; i++)
			possibleAbilityAims[j].position[i] = true;

		possibleAbilityAims[j].howMany = 1;
	}

	this->Player::Init(_id);
}



bool PlayerSimon::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	switch (_id) {
	case 0:
		marshmallowFlash();
		break;
	case 1:
		for (Combatant* t : _targets)
			messAround(t);
		break;
	case 2:
		for (Combatant* t : _targets)
			repair(t);
		break;
	case 3:
		for (Combatant* t : _targets)
			allOnOne(t);
		break;
	}

	return true;
}


void PlayerSimon::messAround(Combatant* _target)
{
	std::cout << "Simon verarscht den Gegner!" << std::endl;
}

void PlayerSimon::allOnOne(Combatant* _target)
{
	std::cout << "Simon hetzt alle auf einen!" << std::endl;
}

void PlayerSimon::marshmallowFlash()
{
	std::cout << "Simon hat einen Marshmallow-Flash!" << std::endl;
}

void PlayerSimon::repair(Combatant* _target)
{
	std::cout << "Simon repariert!" << std::endl;
}
