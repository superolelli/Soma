#include "Simon.hpp"



void PlayerSimon::Init(int _id, CGameEngine *_engine)
{
	combatantObject = g_pModels->modelSimon->getNewEntityInstance("Simon");

	this->Player::Init(_id, _engine);
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
	_target->Status().Confuse(3);
	_target->Status().LooseHealth(status.GetDamage() * 0.5f);
}

void PlayerSimon::allOnOne(Combatant* _target)
{
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

void PlayerSimon::StartAbilityAnimation(int _ability)
{
	combatantObject->setCurrentAnimation("marshmallow_flash");
	combatantObject->setCurrentTime(0);

	lastPosition = combatantObject->getPosition();

	combatantObject->setScale(SpriterEngine::point(0.8, 0.8));

	combatantObject->setPosition(SpriterEngine::point(int(lastPosition.x) - (engine->GetWindow().getView().getCenter().x - engine->GetWindow().getView().getSize().x / 2), 800));
}
