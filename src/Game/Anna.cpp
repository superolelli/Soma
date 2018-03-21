#include "Anna.hpp"



void PlayerAnna::Init(int _id)
{
	combatantObject = g_pModels->modelAnna->getNewEntityInstance("anna");

	CombatantAttributes attributes;
	attributes.armour = 1;
	attributes.currentHealth = 20;
	attributes.maxHealth = 20;
	attributes.damage = 5;
	attributes.initiative = 2;

	status.SetAttributes(attributes);

	possibleAbilityAims[0] = g_pObjectProperties->playerAbilities[Anna][0];
	possibleAbilityAims[1] = g_pObjectProperties->playerAbilities[Anna][1];
	possibleAbilityAims[2] = g_pObjectProperties->playerAbilities[Anna][2];
	possibleAbilityAims[3] = g_pObjectProperties->playerAbilities[Anna][3];

	this->Player::Init(_id);
}


bool PlayerAnna::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	if (status.IsConfused())
	{
		if (rand() % 4 == 0)
			status.LooseHealth(1);
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
	std::cout << "Anna strahlt unbezwingbare Fr�hlichkeit aus!" << std::endl;
	if(rand()%3 == 0)
		_target->Status().Confuse(3);

	_target->Status().GainHealth(3);
}

void PlayerAnna::wordGush(Combatant* _target)
{
	std::cout << "Anna setzt einen Wortschwall frei!" << std::endl;
	if (rand() % 4 == 0)
		_target->Status().PutToSleep();

	_target->Status().LooseHealth(status.GetDamage());
}

void PlayerAnna::sideOfNoseBlow(Combatant* _target)
{
	std::cout << "Anna setzt einen Nasenfl�gelschlag!" << std::endl;
	_target->Status().LooseHealth(status.GetDamage() * 0.4f);
}

void PlayerAnna::deathHug(Combatant* _target)
{
	std::cout << "Anna setzt eine TODESUMARMUNG ein!" << std::endl;
	_target->Status().LooseHealth(status.GetDamage());

	status.GainHealth(1);
}
