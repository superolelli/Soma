#pragma once

#include "PlayerState.hpp"

class PlayerStateExecutingAbility : public PlayerState
{
public:
	PlayerStateExecutingAbility(Player *_context);

	abilityPhase GetStateID() override { return executing; };

	void Update() override;
	void Render() override;

private:
	void StartAbilityAnimation(int _ability);
	void ExecuteAbility();
	void ApplyAbilityEffect();
};