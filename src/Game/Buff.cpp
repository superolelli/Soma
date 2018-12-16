#include "Buff.hpp"





void Buff::SetStandardValues()
{
	stats.armour = 0;
	stats.currentHealth = 0;
	stats.maxHealth = 0;
	stats.damage = 0;
	stats.criticalHit = 0;
	stats.precision = 0;
	stats.dodge = 0;
	stats.initiative = 0;
	stats.attributes.constitution = 0;
	stats.attributes.dexterity = 0;
	stats.attributes.speed = 0;
	stats.attributes.strength = 0;

	isPositive = true;
	onSelf = false;
	duration = 0;
}
