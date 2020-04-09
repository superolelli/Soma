#include "CombatantAttributes.hpp"






void CombatantAttributes::operator+=(const CombatantAttributes & _stats)
{
	maxHealth += _stats.maxHealth;
	currentHealth += _stats.currentHealth;
	damageMin += _stats.damageMin;
	damageMax += _stats.damageMax;
	armour += _stats.armour;
	criticalHit += _stats.criticalHit;
	dodge += _stats.dodge;
	initiative += _stats.initiative;
	precision += _stats.precision;
}


void CombatantAttributes::operator-=(const CombatantAttributes & _stats)
{
	maxHealth -= _stats.maxHealth;
	currentHealth -= _stats.currentHealth;
	damageMin -= _stats.damageMin;
	damageMax -= _stats.damageMax;
	armour -= _stats.armour;
	criticalHit -= _stats.criticalHit;
	dodge -= _stats.dodge;
	initiative -= _stats.initiative;
	precision -= _stats.precision;
}


void CombatantAttributes::Reset()
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
}

CombatantAttributes operator+(CombatantAttributes _lhs, const CombatantAttributes & _rhs)
{
	_lhs += _rhs;
	return _lhs;
}
