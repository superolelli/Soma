#include "Ability.hpp"


void AbilityEffect::clear()
{
	damageFactor = 0.0f;
	heal = 0;
	healSelf = 0;
	confusion = 0;
	confusionProbability = 0;
	mark = 0;
	putToSleepProbability = 0.0f;
	removeBuffs = false;
	removeDebuffs = false;
	buff.SetStandardValues();
}


void Ability::clear()
{
	name.clear();
	animation.clear();
	effectAnimationFriendly.clear();
	effectAnimationHostile.clear();
	effectFriendly.clear();
	effectHostile.clear();
}
