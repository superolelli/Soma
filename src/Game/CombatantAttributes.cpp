#include "CombatantAttributes.hpp"





void CombatantAttributes::operator+=(const CombatantAttributes & _attributes)
{
	strength += _attributes.strength;
	constitution += _attributes.constitution;
	dexterity += _attributes.dexterity;
	speed += _attributes.speed;
}


void CombatantAttributes::operator-=(const CombatantAttributes & _attributes)
{
	strength -= _attributes.strength;
	constitution -= _attributes.constitution;
	dexterity -= _attributes.dexterity;
	speed -= _attributes.speed;
}


void CombatantStats::operator+=(const CombatantStats & _stats)
{
	maxHealth += _stats.maxHealth + _stats.attributes.constitution * CONSTITUTION_TO_HEALTH;
	currentHealth += _stats.currentHealth + _stats.attributes.constitution * CONSTITUTION_TO_HEALTH;
	damageMin += _stats.damageMin + _stats.attributes.strength * STRENGTH_TO_DAMAGE;
	damageMax += _stats.damageMax + _stats.attributes.strength * STRENGTH_TO_DAMAGE;
	armour += _stats.armour;
	criticalHit += _stats.criticalHit + _stats.attributes.dexterity * DEXTERITY_TO_CRITICAL;
	dodge += _stats.dodge + _stats.attributes.speed * SPEED_TO_DODGE;
	initiative += _stats.initiative + _stats.attributes.speed * SPEED_TO_INITIATIVE;
	precision += _stats.precision + _stats.attributes.dexterity * DEXTERITY_TO_PRECISION;
	attributes += _stats.attributes;
}


void CombatantStats::operator-=(const CombatantStats & _stats)
{
	maxHealth -= _stats.maxHealth + _stats.attributes.constitution * CONSTITUTION_TO_HEALTH;
	currentHealth -= _stats.currentHealth + _stats.attributes.constitution * CONSTITUTION_TO_HEALTH;
	damageMin -= _stats.damageMin + _stats.attributes.strength * STRENGTH_TO_DAMAGE;
	damageMax -= _stats.damageMax + _stats.attributes.strength * STRENGTH_TO_DAMAGE;
	armour -= _stats.armour;
	criticalHit -= _stats.criticalHit + _stats.attributes.dexterity * DEXTERITY_TO_CRITICAL;
	dodge -= _stats.dodge + _stats.attributes.speed * SPEED_TO_DODGE;
	initiative -= _stats.initiative + _stats.attributes.speed * SPEED_TO_INITIATIVE;
	precision -= _stats.precision + _stats.attributes.dexterity * DEXTERITY_TO_PRECISION;
	attributes -= _stats.attributes;
}


void CombatantStats::Reset()
{
	maxHealth = 0;
	currentHealth = 0;
	damageMin = 0;
	damageMax = 0;
	armour = 0;
	criticalHit = 0;
	dodge = 0;
	initiative = 0;
	precision = 0;
	attributes.constitution = 0;
	attributes.dexterity = 0;
	attributes.speed = 0;
	attributes.strength = 0;
}