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

	void ChooseAbility();
	void ChooseAbilityGreg();
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