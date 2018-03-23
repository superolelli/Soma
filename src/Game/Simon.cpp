#include "Simon.hpp"



void PlayerSimon::Init(int _id)
{
	combatantObject = g_pModels->modelSimon->getNewEntityInstance("Simon");

	this->Player::Init(_id);
}



bool PlayerSimon::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	if (status.IsConfused())
	{
		if (rand() % 4 == 0)
			status.LooseHealth(1);
	}

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

	_target->Status().Confuse(3);
	_target->Status().LooseHealth(status.GetDamage() * 0.5f);
}

void PlayerSimon::allOnOne(Combatant* _target)
{
	std::cout << "Simon hetzt alle auf einen!" << std::endl;
	Buff newBuff;
	newBuff.duration = 2;
	newBuff.attributes.armour = 30;
	newBuff.attributes.currentHealth = 0;
	newBuff.attributes.maxHealth = 0;
	newBuff.attributes.damage = 0;
	newBuff.attributes.initiative = 0;

	_target->Status().Mark(2);
	_target->Status().AddBuff(newBuff);
}

void PlayerSimon::marshmallowFlash()
{
	std::cout << "Simon hat einen Marshmallow-Flash!" << std::endl;
	Buff newBuff;
	newBuff.duration = 3;
	newBuff.attributes.armour = 10;
	newBuff.attributes.currentHealth = 0;
	newBuff.attributes.maxHealth = 0;
	newBuff.attributes.damage = 3;
	newBuff.attributes.initiative = 1;

	status.AddBuff(newBuff);
	status.GainHealth(5);
}

void PlayerSimon::repair(Combatant* _target)
{
	std::cout << "Simon repariert!" << std::endl;

	if (_target->IsPlayer())
	{
		_target->Status().GainHealth(3);
		_target->Status().RemoveAllDebuffs();
	}
	else
	{
		_target->Status().LooseHealth(status.GetDamage());
		_target->Status().RemoveAllBuffs();
	}
}
