#include "Markus.hpp"




void PlayerMarkus::Init(int _id)
{
	combatantObject = g_pModels->modelMarkus->getNewEntityInstance("Markus");

	CombatantAttributes attributes;
	attributes.armour = 1;
	attributes.currentHealth = 20;
	attributes.maxHealth = 20;
	attributes.damage = 5;
	attributes.initiative = 2;

	status.SetAttributes(attributes);

	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 8; i++)
			possibleAbilityAims[j].position[i] = false;

		possibleAbilityAims[j].howMany = 1;
	}

	possibleAbilityAims[0].position[4] = true;
	possibleAbilityAims[0].position[5] = true;
	possibleAbilityAims[0].position[6] = true;
	possibleAbilityAims[0].position[7] = true;

	possibleAbilityAims[1].howMany = 3;
	possibleAbilityAims[1].position[4] = true;
	possibleAbilityAims[1].position[5] = true;
	possibleAbilityAims[1].position[6] = true;
	possibleAbilityAims[1].position[7] = true;

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


bool PlayerMarkus::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	if (status.IsConfused())
	{
		if (rand() % 4 == 0)
			status.LooseHealth(1);
	}

	switch (_id) {
	case 0:
		for (Combatant* t : _targets)
			fistOfRevenge(t);
		break;
	case 1:
		for (Combatant* t : _targets)
			noseRam(t);
		break;
	case 2:
		for (Combatant* t : _targets)
			hardDestruction(t);
		break;
	case 3:
		for (Combatant* t : _targets)
			horribleGrin(t);
		break;
	}
	return true;
}

void PlayerMarkus::fistOfRevenge(Combatant* _target)
{
	std::cout << "FIST OF REVENGE!" << std::endl;
	_target->Status().LooseHealth(status.GetDamage() * 2);
}

void PlayerMarkus::noseRam(Combatant* _target)
{
	std::cout << "Markus setzt seine Nasenramme ein!" << std::endl;
	_target->Status().LooseHealth(status.GetDamage() * 0.3f);
}

void PlayerMarkus::hardDestruction(Combatant* _target)
{
	std::cout << "Markus praktiziert harte Zerstörung!" << std::endl;
	_target->Status().LooseHealth(status.GetDamage());
}

void PlayerMarkus::horribleGrin(Combatant* _target)
{
	std::cout << "Markus grinst fürchterlich!" << std::endl;
	Buff newBuff;
	newBuff.duration = 3;
	newBuff.attributes.armour = 0;
	newBuff.attributes.currentHealth = 0;
	newBuff.attributes.maxHealth = 0;
	newBuff.attributes.damage = -2;
	newBuff.attributes.initiative = -2;

	_target->Status().AddDebuff(newBuff);
	_target->Status().LooseHealth(status.GetDamage());
}
