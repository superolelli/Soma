#pragma once

#include "Buff.hpp"

#include <string>



struct PossibleAims
{
    bool position[8];
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
};


struct Ability
{
    std::string name;
    PossibleAims possibleAims;
    std::string animation;
    std::string effectAnimationFriendly;
    std::string effectAnimationHostile;
    AbilityEffect effectFriendly;
    AbilityEffect effectHostile;
};