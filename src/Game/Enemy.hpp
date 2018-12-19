#pragma once

#include "Combatant.hpp"
#include "Resources\Resources.hpp"

#include <numeric>

const float ENEMY_SCALE = 0.6f;

class Enemy : public Combatant
{
public:

	virtual void Init(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer) override;

	virtual int GetID() override { return enemyID; }

	virtual bool DoAbility(int _id, std::vector<Combatant*> &_targets);

	void Update() override;

	virtual void Render() override;

	std::string GetChosenAbilityName() { return g_pObjectProperties->enemyAbilities[chosenAbility].name; }

private:

	int enemyID;

	enemyAbilities chosenAbility;

	float abilityAnnouncementTime;

	void PrepareAbility();
	void ChooseAbility();

	void ChooseTarget();
	void CheckForMarkedPlayers();
	void ChooseRandomPlayer();

	bool CanAimAtCombatant(Combatant *_combatant);

	void AnnounceAndStartAbilityAnimation();
	void StartAbilityAnimation(int _ability);

	void ExecuteAbility();

	void RenderAbilityAnnouncement();
	void RenderAbilityTargetMarker();
};