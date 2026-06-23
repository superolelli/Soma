#pragma once

#include "CombatantState.hpp"

class CombatantStateIdle : public CombatantState
{
public:

	CombatantStateIdle(Combatant *_context);

	abilityPhase GetStateID() override { return finished; };

	void Update() override;
	void Render() override;

private:
};