#include "CombatantAttributes.hpp"





void CombatantAttributes::operator+=(const CombatantAttributes & _attributes)
{
	maxHealth += _attributes.maxHealth;
	currentHealth += _attributes.currentHealth;
	damage += _attributes.damage;
	armour += _attributes.armour;
	initiative += _attributes.initiative;
}


void CombatantAttributes::operator-=(const CombatantAttributes & _attributes)
{
	maxHealth -= _attributes.maxHealth;
	currentHealth -= _attributes.currentHealth;
	damage -= _attributes.damage;
	armour -= _attributes.armour;
	initiative -= _attributes.initiative;
}
