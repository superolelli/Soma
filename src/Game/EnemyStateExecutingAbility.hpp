#pragma once


#include "EnemyState.hpp"

class EnemyStateExecutingAbility : public EnemyState
{
public:
	EnemyStateExecutingAbility(Enemy *_context, enemyAbilities _chosenAbility);

	abilityPhase GetStateID() override { return executing; };

	void Update() override;
	void Render() override;

private:
	int chosenAbility;

	void ExecuteAbility();
	void ApplyAbilityEffect();
	void StartAbilityAnimation();
};