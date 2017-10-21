#pragma once

#include "../Framework/Patterns/Singleton.hpp"

struct PossibleAims
{
	bool position[8];
	int howMany;
};




#define g_pAbilities Abilities::Get()
class Abilities : public TSingleton<Abilities>
{
public:

	Abilities();

	bool(Abilities::*abilitySimon[4])();
	bool(Abilities::*abilityOle[4])();
	bool(Abilities::*abilityAnna[4])();
	bool(Abilities::*abilityMarkus[4])();

	PossibleAims abilityAims[4][4];

private:

	bool repair();
	bool messAround();
	bool marshmallowFlash();
	bool allOnOne();
};
