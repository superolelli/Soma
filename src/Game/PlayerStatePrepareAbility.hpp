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

	double notificationWaitingTime;

	void ChangeState();
	void HandleConfusion();
    void HandleStupidness();
	void SelectRandomAbilityAndTargets();
	void SelectConfusionTargets();
	bool PlayerShouldBeAddedAsTarget(Combatant *_combatant, int _targetPosition);
	bool EnemyShouldBeAddedAsTarget(Combatant *_combatant, int _targetPosition);
	void SelectAdditionalConfusionTargets(bool _selectPlayers);
	void SelectAdditionalTargets();
	int NumberOfTargetsForCurrentAbility();
	bool CurrentAbilityAttacksAll();
	bool CurrentAbilityAttacksAllPlayers();
	void RenderAbilityTargetMarker();

	void UpdateSelectedTargets();
};