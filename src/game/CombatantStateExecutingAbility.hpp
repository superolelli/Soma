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
	const float COMBATANT_SCALE = 1.0 + COMBATANT_ABILITY_SCALE - COMBATANT_NORMAL_SCALE;

	Ability *ability;
	sf::Vector2i combatantAnimationPositions[8];

	void CalculateCombatantAnimationPositions();
	int CalculateSelectedTargetsTotalWidth(int _padding);
	void CalculateSelectedTargetsPositions(int _startingPosition, int _padding);

	void ExecuteAbility();
	void ApplyAbilityEffect();
	void StartAbilityAnimation();
	void AttackTargets();
	void StartAttackedAnimation(Combatant *_combatant, sf::Vector2i _position);
	bool CheckForDodging(Combatant * _attacker, Combatant * _defender);
	void StopTargetsAttackedAnimation();
};