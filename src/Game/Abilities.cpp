#include "Abilities.hpp"


Abilities::Abilities()
{
	for(int j = 0; j < 4; j++)
		for(int i = 0; i < 8; i++)
			abilityAims[0][j].position[i] = true;

	abilityAims[0][0].howMany = 1;
	abilityAims[0][1].howMany = 1;
	abilityAims[0][2].howMany = 1;
	abilityAims[0][3].howMany = 1;

	for (int j = 0; j < 4; j++)
		for (int i = 0; i < 8; i++)
			abilityAims[1][j].position[i] = true;

	abilityAims[1][0].howMany = 1;
	abilityAims[1][1].howMany = 1;
	abilityAims[1][2].howMany = 1;
	abilityAims[1][3].howMany = 1;

	for (int j = 0; j < 4; j++)
		for (int i = 0; i < 8; i++)
			abilityAims[2][j].position[i] = true;

	abilityAims[2][0].howMany = 1;
	abilityAims[2][1].howMany = 1;
	abilityAims[2][2].howMany = 1;
	abilityAims[2][3].howMany = 1;

	for (int j = 0; j < 4; j++)
		for (int i = 0; i < 8; i++)
			abilityAims[3][j].position[i] = true;

	abilityAims[3][0].howMany = 1;
	abilityAims[3][1].howMany = 1;
	abilityAims[3][2].howMany = 1;
	abilityAims[3][3].howMany = 1;

	abilitySimon[0] = &Abilities::repair;
	abilitySimon[1] = &Abilities::messAround;
	abilitySimon[2] = &Abilities::marshmallowFlash;
	abilitySimon[3] = &Abilities::allOnOne;

	abilityOle[0] = &Abilities::repair;
	abilityOle[1] = &Abilities::messAround;
	abilityOle[2] = &Abilities::marshmallowFlash;
	abilityOle[3] = &Abilities::allOnOne;

	abilityAnna[0] = &Abilities::repair;
	abilityAnna[1] = &Abilities::messAround;
	abilityAnna[2] = &Abilities::marshmallowFlash;
	abilityAnna[3] = &Abilities::allOnOne;

	abilityMarkus[0] = &Abilities::repair;
	abilityMarkus[1] = &Abilities::messAround;
	abilityMarkus[2] = &Abilities::marshmallowFlash;
	abilityMarkus[3] = &Abilities::allOnOne;
}




bool Abilities::repair()
{
	return true;
}

bool Abilities::messAround()
{
	return true;
}

bool Abilities::marshmallowFlash()
{
	return true;
}

bool Abilities::allOnOne()
{
	return true;
}
