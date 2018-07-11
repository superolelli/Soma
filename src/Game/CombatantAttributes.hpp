#pragma once




struct CombatantAttributes
{
	int maxHealth;
	int currentHealth;
	int damage;
	int armour;
	int initiative;
	int dexterity;

	void operator +=(const CombatantAttributes &_attributes);
	void operator -=(const CombatantAttributes &_attributes);
};