#include "Anna.hpp"



void PlayerAnna::Init(int _id, CGameEngine *_engine)
{
	combatantObject = g_pModels->modelAnna->getNewEntityInstance("anna");

	this->Player::Init(_id, _engine);
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
	if (_target->IsPlayer())
	{
		Buff newBuff;
		newBuff.duration = 3;
		newBuff.attributes.armour = 0;
		newBuff.attributes.currentHealth = 0;
		newBuff.attributes.maxHealth = 0;
		newBuff.attributes.damage = 5;
		newBuff.attributes.initiative = 2;
		_target->Status().AddBuff(newBuff);
	}
	else
	{
		if (rand() % 3 == 0)
			_target->Status().Confuse(3);

		Status().GainHealth(3);
	}
}

void PlayerAnna::wordGush(Combatant* _target)
{
	if (rand() % 4 == 0)
		_target->Status().PutToSleep();

	_target->Status().LooseHealth(status.GetDamage());
}

void PlayerAnna::sideOfNoseBlow(Combatant* _target)
{
	_target->Status().LooseHealth(status.GetDamage() * 0.4f);
}

void PlayerAnna::deathHug(Combatant* _target)
{
	_target->Status().LooseHealth(status.GetDamage());

	status.GainHealth(1);
}

void PlayerAnna::StartAbilityAnimation(int _ability)
{
	ScaleForAbilityAnimation();

	switch (_ability)
	{
	case 0:
		SetAnimation("indomitable_happiness", ABILITY_ANIMATION_SPEED);

		if (selectedTargets[0]->IsPlayer())
			g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation("indomitable_happiness_friendly");
		else
			g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation("indomitable_happiness_hostile");
		break;
	case 1:
		SetAnimation("word_gush", ABILITY_ANIMATION_SPEED);
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation("word_gush");
		break;
	case 2:
		SetAnimation("side_of_nose_blow", ABILITY_ANIMATION_SPEED);
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation("side_of_nose_blow");
		break;
	case 3:
		SetAnimation("death_hug", ABILITY_ANIMATION_SPEED);
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation("death_hug");
		break;
	}

	g_pSpritePool->abilityEffectsAnimation->setCurrentTime(0);
}
