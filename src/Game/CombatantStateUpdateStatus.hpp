#pragma once


#include "CombatantState.hpp"

class CombatantStateUpdateStatus : public CombatantState
{
public:

	CombatantStateUpdateStatus(Combatant *_context);

	abilityPhase GetStateID() override { return handlingStatus; };

	void Update() override;
	void Render() override;

private:
};