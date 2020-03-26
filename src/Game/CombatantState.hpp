#pragma once

#include "Combatant.hpp"

class CombatantState
{
public:

	CombatantState(Combatant *_context);

	virtual abilityPhase GetStateID() = 0;

	virtual void Update() = 0;
	virtual void Render() = 0;

protected:
	Combatant *context;
};