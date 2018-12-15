#pragma once

#include "CombatantAttributes.hpp"


class Buff
{
public:

	void SetStandardValues();

	CombatantAttributes attributes;
	int duration;
	bool isPositive;
	bool onSelf;
};