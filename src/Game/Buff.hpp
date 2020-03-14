#pragma once

#include "CombatantAttributes.hpp"


class Buff
{
public:

	void SetStandardValues();
	void ApplySkill(const Buff &_buff);

	CombatantAttributes stats;
	int duration;
	bool isPositive;
	bool onSelf;
};