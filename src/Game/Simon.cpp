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
	ScaleForAbilityAnimation();

	switch (_ability) {
	case 0:
		SetAnimation("marshmallow_flash", ABILITY_ANIMATION_SPEED);
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation("empty");
		break;
	case 1:
		SetAnimation("mess_around", ABILITY_ANIMATION_SPEED);
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation("mess_around");
		break;
	case 2:
		SetAnimation("repair", ABILITY_ANIMATION_SPEED);

		if(selectedTargets[0]->IsPlayer())
			g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation("repair_friendly");
		else
			g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation("repair_hostile");

		break;
	case 3:
		SetAnimation("all_on_one", ABILITY_ANIMATION_SPEED);
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation("all_on_one");
		break;
	}

	
	g_pSpritePool->abilityEffectsAnimation->setCurrentTime(0);
}
