#include "Buff.hpp"





void Buff::SetStandardValues()
{
	stats.armour = 0;
	stats.currentHealth = 0;
	stats.maxHealth = 0;
	stats.damageMin = 0;
	stats.damageMax = 0;
	stats.criticalHit = 0;
	stats.precision = 0;
	stats.dodge = 0;
	stats.initiative = 0;

	isPositive = true;
	onSelf = false;
	duration = 0;
}



void Buff::ApplySkill(const Buff &_buff)
{
	stats.armour += _buff.stats.armour;
	stats.currentHealth += _buff.stats.currentHealth;
	stats.maxHealth += _buff.stats.maxHealth;
	stats.damageMin += _buff.stats.damageMin;
	stats.damageMax += _buff.stats.damageMax;
	stats.criticalHit += _buff.stats.criticalHit;
	stats.precision += _buff.stats.precision;
	stats.dodge += _buff.stats.dodge;
	stats.initiative += _buff.stats.initiative;

	if(duration <= 0)
		isPositive = _buff.isPositive;

	onSelf = _buff.onSelf;
	duration += _buff.duration;
}