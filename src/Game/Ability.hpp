#pragma once

#include "Buff.hpp"

#include <string>
#include <SFML/System.hpp>



struct PossibleAims
{
    bool position[8];
	bool attackAll;
	bool attackAllPlayers;
	bool attackAllEnemies;
    int howMany;

	void applySkill(const PossibleAims &_aims);
};


struct AbilityEffect
{
	int criticalHitModificator;
	float lessTargetsMoreDamage;

    float damageFactor;
    int heal;
    int healSelf;
    int confusion;
	float confusionProbability;
	int damageOverTime;
	int damageOverTimeRounds;
    int mark;
	float putToSleepProbability;
    bool removeBuffs;
    bool removeDebuffs;
    Buff buff;

	void applySkill(const AbilityEffect &_effect);
	void clear();
};


struct Ability
{
    sf::String name;
	sf::String description;
    PossibleAims possibleAims;
    std::string animation;
    std::string effectAnimationFriendly;
    std::string effectAnimationHostile;
    AbilityEffect effectFriendly;
    AbilityEffect effectHostile;

	bool canTargetEnemiesOrFriends;

    int fatigue;

	int precisionModificator;

	void applySkill (const Ability &_ability);
	void clear();
};