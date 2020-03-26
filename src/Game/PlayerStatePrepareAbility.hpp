#pragma once


#include "PlayerState.hpp"

class PlayerStatePrepareAbility : public PlayerState
{
public:
	PlayerStatePrepareAbility(Player *_context);

	abilityPhase GetStateID() override { return ready; };

	void Update() override;
	void Render() override;

	float GetAdditionalDamageForCurrentlyAimedCombatant();
	bool CurrentAbilityCanAimAtCombatant(Combatant* _combatant);

private:

	void PrepareTargets(Combatant *_target);
	bool CombatantClicked(Combatant* _combatant);
	void ChooseCombatantsForConfusionAttack(Combatant *_originallyAttackedCombatant);
	void SelectAdditionalPlayers();
	void SelectAdditionalEnemies();
	void SelectAdditionalTargets();
	int NumberOfTargetsForCurrentAbility();
	bool CurrentAbilityAttacksAll();
	bool CurrentAbilityAttacksAllPlayers();
	void RenderAbilityTargetMarker();
};