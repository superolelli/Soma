#include "Ole.hpp"



void PlayerOle::Init(int _id)
{
	combatantObject = g_pModels->modelOle->getNewEntityInstance("Ole");

	CombatantAttributes attributes;
	attributes.armour = 1;
	attributes.currentHealth = 20;
	attributes.maxHealth = 20;
	attributes.damage = 5;
	attributes.initiative = 2;

	status.SetAttributes(attributes);

	this->Player::Init(_id);
}



bool PlayerOle::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	if (status.IsConfused())
	{
		if (rand() % 4 == 0)
			status.LooseHealth(1);
	}

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
	_target->Status().LooseHealth(status.GetDamage() * 0.3f);
}

void PlayerOle::wholeBodyKick(Combatant* _target)
{
	std::cout << "Ole setzt einen Ganzkörperkick ein!" << std::endl;

	Buff newBuff;
	newBuff.duration = 3;
	newBuff.attributes.armour = 0;
	newBuff.attributes.currentHealth = 0;
	newBuff.attributes.maxHealth = 0;
	newBuff.attributes.damage = -1;
	newBuff.attributes.initiative = -1;

	_target->Status().AddDebuff(newBuff);
	_target->Status().LooseHealth(status.GetDamage());
}

void PlayerOle::hackIntoTheSystem(Combatant* _target)
{
	std::cout << "Ole hackt das System!" << std::endl;

	Buff newBuff;
	newBuff.duration = 3;
	newBuff.attributes.armour = -10;
	newBuff.attributes.currentHealth = 0;
	newBuff.attributes.maxHealth = 0;
	newBuff.attributes.damage = -3;
	newBuff.attributes.initiative = 0;

	_target->Status().AddDebuff(newBuff);
	_target->Status().LooseHealth(status.GetDamage());
}

void PlayerOle::poetrySlam(Combatant* _target)
{
	std::cout << "Ole trägt einen Poetry Slam vor!" << std::endl;

	_target->Status().PutToSleep();
	_target->Status().LooseHealth(status.GetDamage());
}

