#include "Markus.hpp"




void PlayerMarkus::Init(int _id)
{
	combatantObject = g_pModels->modelMarkus->getNewEntityInstance("Markus");

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


bool PlayerMarkus::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
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
}

void PlayerMarkus::noseRam(Combatant* _target)
{
	std::cout << "Markus setzt seine Nasenramme ein!" << std::endl;
}

void PlayerMarkus::hardDestruction(Combatant* _target)
{
	std::cout << "Markus praktiziert harte Zerstörung!" << std::endl;
}

void PlayerMarkus::horribleGrin(Combatant* _target)
{
	std::cout << "Markus grinst fürchterlich!" << std::endl;
}
