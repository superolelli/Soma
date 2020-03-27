#pragma once

#include "CombatantState.hpp"

class CombatantStateAttacked : public CombatantState
{
public:

	CombatantStateAttacked(Combatant *_context, bool _dodge);

	abilityPhase GetStateID() override;

	void Update() override;
	void Render() override;

private:

	bool dodged;
	SpriterEngine::point abilityEffectPoint;

	void RenderAbilityEffects();
	void ReloadAbilityEffectPoint();
};