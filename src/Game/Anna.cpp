#include "Anna.hpp"



void PlayerAnna::Init(int _id)
{
	combatantObject = g_pModels->modelAnna->getNewEntityInstance("anna");

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


bool PlayerAnna::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	switch (_id) {
	case 0:
		for (Combatant* t : _targets)
			indomitableHappiness(t);
		break;
	case 1:
		for (Combatant* t : _targets)
			wordGush(t);
		break;
	case 2:
		for (Combatant* t : _targets)
			sideOfNoseBlow(t);
		break;
	case 3:
		for (Combatant* t : _targets)
			deathHug(t);
		break;
	}
	return true;
}

void PlayerAnna::indomitableHappiness(Combatant* _target)
{
	std::cout << "Anna strahlt unbezwingbare Fröhlichkeit aus!" << std::endl;
}

void PlayerAnna::wordGush(Combatant* _target)
{
	std::cout << "Anna setzt einen Wortschwall frei!" << std::endl;
}

void PlayerAnna::sideOfNoseBlow(Combatant* _target)
{
	std::cout << "Anna setzt einen Nasenflügelschlag!" << std::endl;
}

void PlayerAnna::deathHug(Combatant* _target)
{
	std::cout << "Anna setzt eine TODESUMARMUNG ein!" << std::endl;
}
