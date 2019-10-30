#pragma once


const int CONSTITUTION_TO_HEALTH = 5;
const int DEXTERITY_TO_PRECISION = 1;
const int STRENGTH_TO_DAMAGE = 2;
const int DEXTERITY_TO_CRITICAL = 1;
const int SPEED_TO_DODGE = 3;
const int SPEED_TO_INITIATIVE = 1;


struct CombatantAttributes
{
	int dexterity;
	int speed;
	int constitution;
	int strength;

	void operator +=(const CombatantAttributes &_attributes);
	void operator -=(const CombatantAttributes &_attributes);
};

struct CombatantStats
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

	CombatantAttributes attributes;

	void operator +=(const CombatantStats &_stats);
	void operator -=(const CombatantStats &_stats);

	void Reset();
};

