#pragma once

#include "Buff.hpp"

#include <string>
#include <SFML/System.hpp>



struct PossibleAims
{
    bool position[8];
	bool attackAll;
    int howMany;
};


struct AbilityEffect
{
    float damageFactor;
    int heal;
    int healSelf;
    int confusion;
	float confusionProbability;
    int mark;
	float putToSleepProbability;
    bool removeBuffs;
    bool removeDebuffs;
    Buff buff;

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

	int precisionModificator;
	int criticalHitModificator;
	float lessTargetsMoreDamage;

	void clear();
};