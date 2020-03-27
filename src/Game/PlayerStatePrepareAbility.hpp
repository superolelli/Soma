#pragma once


#include "CombatantStatePrepareAbility.hpp"

class Player;

class PlayerStatePrepareAbility : public CombatantStatePrepareAbility
{
public:
	PlayerStatePrepareAbility(Player *_context);

	void Update() override;
	void Render() override;

	float GetAdditionalDamageForCurrentlyAimedCombatant();
	bool CurrentAbilityCanAimAtCombatant(Combatant* _combatant);

private:
	Player *playerContext;

	void HandleConfusion();
	bool PlayerShouldBeAddedAsTarget(Combatant *_combatant, int _targetPosition);
	bool EnemyShouldBeAddedAsTarget(Combatant *_combatant, int _targetPosition);
	void SelectAdditionalTargets(bool _selectPlayers);
	int NumberOfTargetsForCurrentAbility();
	bool CurrentAbilityAttacksAll();
	bool CurrentAbilityAttacksAllPlayers();
	void RenderAbilityTargetMarker();

	void UpdateSelectedTargets();
};