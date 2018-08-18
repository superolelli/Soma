#include "Ole.hpp"



void PlayerOle::Init(int _id, CGameEngine *_engine)
{
	combatantObject = g_pModels->modelOle->getNewEntityInstance("Ole");

	this->Player::Init(_id, _engine);
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
	_target->Status().LooseHealth(status.GetDamage() * 0.3f);
}

void PlayerOle::wholeBodyKick(Combatant* _target)
{
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
	_target->Status().PutToSleep();
	_target->Status().LooseHealth(status.GetDamage());
}

void PlayerOle::StartAbilityAnimation(int _ability)
{
	ScaleForAbilityAnimation();
	SetAnimation("hack_into_the_system", ABILITY_ANIMATION_SPEED);

	g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation("empty");
	g_pSpritePool->abilityEffectsAnimation->setCurrentTime(0);
}

