#pragma once


struct CombatantAttributes
{
	int maxHealth;
	int currentHealth;
	int armour;
	int damageMin;
	int damageMax;
	int criticalHit;
	int dodge;
	int precision;
	int initiative;

	void operator +=(const CombatantAttributes &_stats);
	void operator -=(const CombatantAttributes &_stats);

	void Reset();
};

