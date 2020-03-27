#pragma once

#include "CombatantState.hpp"

class CombatantStateDying : public CombatantState
{
public:

	CombatantStateDying(Combatant *_context);

	abilityPhase GetStateID() override { return dying; };

	void Update() override;
	void Render() override;

private:
};