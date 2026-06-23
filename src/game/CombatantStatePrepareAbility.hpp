#pragma once

#include "CombatantState.hpp"

class CombatantStatePrepareAbility : public CombatantState
{
public:

	CombatantStatePrepareAbility(Combatant *_context);

	abilityPhase GetStateID() override { return ready; };

	virtual void Update() override {};
	virtual void Render() override {};

protected:
	void ChooseRandomAlly(bool _canChooseSelf = false);
	void ChooseRandomOpponent();
};