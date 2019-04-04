#pragma once

#include "Buff.hpp"

#include <string>



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
    std::string name;
	std::string description;
    PossibleAims possibleAims;
    std::string animation;
    std::string effectAnimationFriendly;
    std::string effectAnimationHostile;
    AbilityEffect effectFriendly;
    AbilityEffect effectHostile;

	void clear();
};
