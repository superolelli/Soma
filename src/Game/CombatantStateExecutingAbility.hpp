#pragma once

#include "CombatantState.hpp"

class CombatantStateExecutingAbility : public CombatantState
{
public:
	CombatantStateExecutingAbility(Combatant *_context, Ability *_ability);

	abilityPhase GetStateID() override { return executing; };

	void Update() override;
	void Render() override;

private:
	Ability *ability;

	void ExecuteAbility();
	void ApplyAbilityEffect();
	void StartAbilityAnimation();
	void AttackTargets();
	void StartAttackedAnimation(Combatant *_combatant);
	bool CheckForDodging(Combatant * _attacker, Combatant * _defender);
	void StopTargetsAttackedAnimation();
};