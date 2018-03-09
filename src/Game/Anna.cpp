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
			possibleAbilityAims[j].position[i] = false;

		possibleAbilityAims[j].howMany = 1;
	}


	possibleAbilityAims[0].position[0] = true;
	possibleAbilityAims[0].position[1] = true;
	possibleAbilityAims[0].position[2] = true;
	possibleAbilityAims[0].position[3] = true;

	possibleAbilityAims[1].position[4] = true;
	possibleAbilityAims[1].position[5] = true;
	possibleAbilityAims[1].position[6] = true;
	possibleAbilityAims[1].position[7] = true;

	possibleAbilityAims[2].howMany = 2;
	possibleAbilityAims[2].position[4] = true;
	possibleAbilityAims[2].position[5] = true;
	possibleAbilityAims[2].position[6] = true;
	possibleAbilityAims[2].position[7] = true;

	possibleAbilityAims[3].position[4] = true;
	possibleAbilityAims[3].position[5] = true;
	possibleAbilityAims[3].position[6] = true;
	possibleAbilityAims[3].position[7] = true;

	this->Player::Init(_id);
}


bool PlayerAnna::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	if (status.confused > 0)
	{
		if (rand() % 4 == 0)
			LooseHealth(1);
	}

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
	if(rand()%3 == 0)
		_target->Confuse(3);

	_target->GainHealth(3);
}

void PlayerAnna::wordGush(Combatant* _target)
{
	std::cout << "Anna setzt einen Wortschwall frei!" << std::endl;
	if (rand() % 4 == 0)
		_target->PutToSleep();

	_target->LooseHealth(attributes.damage);
}

void PlayerAnna::sideOfNoseBlow(Combatant* _target)
{
	std::cout << "Anna setzt einen Nasenflügelschlag!" << std::endl;
	_target->LooseHealth(attributes.damage * 0.4f);
}

void PlayerAnna::deathHug(Combatant* _target)
{
	std::cout << "Anna setzt eine TODESUMARMUNG ein!" << std::endl;
	_target->LooseHealth(attributes.damage);

	GainHealth(1);
}
