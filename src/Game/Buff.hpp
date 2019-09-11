#pragma once

#include "CombatantAttributes.hpp"


class Buff
{
public:

	void SetStandardValues();
	void ApplySkill(const Buff &_buff);

	CombatantStats stats;
	int duration;
	bool isPositive;
	bool onSelf;
};