#include "Ole.hpp"



void PlayerOle::Init(int _id)
{
	combatantObject = g_pModels->modelOle->getNewEntityInstance("Ole");

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



bool PlayerOle::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	switch (_id) {
	case 0:
		for(Combatant* t : _targets)
			gummiArm(t);
		break;
	case 1:
		for (Combatant* t : _targets)
			wholeBodyKick(t);
		break;
	case 2:
		for (Combatant* t : _targets)
			hackIntoTheSystem(t);
		break;
	case 3:
		for (Combatant* t : _targets)
			poetrySlam(t);
		break;
	}
	return true;
}


void PlayerOle::gummiArm(Combatant* _target)
{
	std::cout << "Ole setzt seinen Gummiarm ein!" << std::endl;
}

void PlayerOle::wholeBodyKick(Combatant* _target)
{
	std::cout << "Ole setzt einen Ganzkörperkick ein!" << std::endl;
}

void PlayerOle::hackIntoTheSystem(Combatant* _target)
{
	std::cout << "Ole hackt das System!" << std::endl;
}

void PlayerOle::poetrySlam(Combatant* _target)
{
	std::cout << "Ole trägt einen Poetry Slam vor!" << std::endl;
}

