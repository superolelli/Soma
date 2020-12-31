#pragma once


#include "CombatantStatePrepareAbility.hpp"

class Enemy;

class EnemyStatePrepareAbility : public CombatantStatePrepareAbility
{
public:
	EnemyStatePrepareAbility(Enemy *_context);

	void Update() override;
	void Render() override;

private:

	Enemy *enemyContext;

	enemyAbilities chosenAbility;

	bool EnemyShouldBeAddedAsTarget(Combatant * _combatant, int _targetPosition);
	bool PlayerShouldBeAddedAsTarget(Combatant * _combatant, int _targetPosition);
	void SelectAdditionalTargets(bool _selectPlayers);
	bool ChosenAbilityHitsPlayer();

    bool ChosenAbilityHitsSelf();
	bool ChosenAbilityHitsOnlySelf();

	void ChooseAbility();
	void ChooseAbilityGreg();
	void ChooseAbilityGesetzloser();
	void ChooseAbilityIndianer();
	void ChooseAbilityAbtruenniger();
	void ChooseAbilityApacheKid();
	void ChooseAbilityHilfssherrif();
	void ChooseAbilityTequilaJoe();
	void ChooseAbilityBigSpencer();
	void ChooseAbilityBillNoface();
	void ChooseAbilityJuanTirador();
	void ChooseAbilityMarieSaunier();
	void ChooseAbilityBruderZacharias();

	void ChooseTargetForDoctor();
	void ChooseTargetForKnockOut();
	void ChooseTargetForBounty();

	void ChooseTarget();
    void ChooseRandomEnemy();
    void ChooseRandomPlayer();
	void CheckForMarkedPlayers();
	bool CanAimAtCombatant(Combatant *_combatant);

	void HandleConfusion();


	void RenderAbilityAnnouncement();
	void RenderAbilityTargetMarker();
	sf::String &GetChosenAbilityName();

	bool OnlyOneCompanionLeft();

	double abilityAnnouncementTime;
};