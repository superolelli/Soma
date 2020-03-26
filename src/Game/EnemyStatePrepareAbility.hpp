#pragma once


#include "EnemyState.hpp"

class EnemyStatePrepareAbility : public EnemyState
{
public:
	EnemyStatePrepareAbility(Enemy *_context);

	abilityPhase GetStateID() override { return ready; };

	void Update() override;
	void Render() override;

private:

	enemyAbilities chosenAbility;

	void ChooseAbility();
	void ChooseAbilityGreg();
	void ChooseAbilityAbtruenniger();
	void ChooseAbilityApacheKid();
	void ChooseAbilityHilfssherrif();
	void ChooseAbilityBigSpencer();

	void ChooseTarget();
	void SelectAdditionalEnemies();
	void SelectAdditionalPlayers();
	void ChooseRandomPlayer();
	void CheckForMarkedPlayers();
	bool CanAimAtCombatant(Combatant *_combatant);


	void RenderAbilityAnnouncement();
	void RenderAbilityTargetMarker();
	sf::String &GetChosenAbilityName();

	bool OnlyOneCompanionLeft();

	float abilityAnnouncementTime;
};