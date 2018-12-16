#pragma once

#include "CombatantAttributes.hpp"


class Buff
{
public:

	void SetStandardValues();

	CombatantStats stats;
	int duration;
	bool isPositive;
	bool onSelf;
};