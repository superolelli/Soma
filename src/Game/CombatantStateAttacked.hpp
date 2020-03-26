#pragma once

#include "CombatantState.hpp"

class CombatantStateAttacked : public CombatantState
{
public:

	CombatantStateAttacked(Combatant *_context);

	abilityPhase GetStateID() override { return attacked; };

	void Update() override;
	void Render() override;

private:
};