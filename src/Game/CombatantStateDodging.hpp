#pragma once

#include "CombatantState.hpp"

class CombatantStateDodging : public CombatantState
{
public:

	CombatantStateDodging(Combatant *_context);

	abilityPhase GetStateID() override { return dodging; };

	void Update() override;
	void Render() override;

private:
};