#include "Ability.hpp"


void AbilityEffect::clear()
{
	criticalHitModificator = 0;
	lessTargetsMoreDamage = 0.0f;

	damageFactor = 0.0f;
	heal = 0;
	healSelf = 0;
	confusion = 0;
	confusionProbability = 0;
	mark = 0;
	putToSleepProbability = 0.0f;
	decay = 0;
	decayRounds = 0;
	dynamite = false;
	addMiss = 0;
	addMissSelf = 0;
	removeBuffs = false;
	removeDebuffs = false;
	buff.SetStandardValues();
}

void AbilityEffect::applySkill(const AbilityEffect &_effect)
{
	criticalHitModificator += _effect.criticalHitModificator;
	lessTargetsMoreDamage += _effect.lessTargetsMoreDamage;

	damageFactor += _effect.damageFactor;
	heal += _effect.heal;
	healSelf += _effect.healSelf;
	confusion += _effect.confusion;
	confusionProbability += _effect.confusionProbability;
	mark += _effect.mark;
	putToSleepProbability += _effect.putToSleepProbability;
	decay += _effect.decay;
	decayRounds += _effect.decayRounds;
	removeBuffs = removeBuffs || _effect.removeBuffs;
	removeDebuffs = removeDebuffs || _effect.removeDebuffs;
	buff.ApplySkill(_effect.buff);
}



void Ability::applySkill(const Ability & _ability)
{
	precisionModificator += _ability.precisionModificator;
	fatigue += _ability.fatigue;

	possibleAims.applySkill(_ability.possibleAims);
	effectFriendly.applySkill(_ability.effectFriendly);
	effectHostile.applySkill(_ability.effectHostile);
}


void Ability::clear()
{
	name.clear();
	description.clear();
	animation.clear();
	effectAnimationFriendly.clear();
	effectAnimationHostile.clear();
	effectFriendly.clear();
	effectHostile.clear();

	canTargetEnemiesOrFriends = false;

	precisionModificator = 0;
	fatigue = 0;
}

void PossibleAims::applySkill(const PossibleAims & _aims)
{
	attackAll = attackAll || _aims.attackAll;
	howMany += _aims.howMany;
	
	for (int i = 0; i < 8; i++)
	{
		position[i] = position[i] || _aims.position[i];
	}
}
