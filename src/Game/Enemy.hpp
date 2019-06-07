#pragma once

#include "Combatant.hpp"
#include "Resources\Resources.hpp"

#include <numeric>

class Enemy : public Combatant
{
public:

	Enemy(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

	virtual void Init() override;

	virtual int GetID() override { return enemyID; }

	virtual bool DoAbility(int _id, std::vector<Combatant*> &_targets);

	void Update() override;

	virtual void Render() override;

	sf::String GetChosenAbilityName() { return g_pObjectProperties->enemyAbilities[chosenAbility].name; }

protected:

	int enemyID;

	enemyAbilities chosenAbility;

	float abilityAnnouncementTime;

	bool confusionChecked;

	void PrepareAbility();
	virtual void ChooseAbility();

	void ChooseTarget();
	void CheckForMarkedPlayers();
	void ChooseRandomPlayer();
    void ChooseRandomEnemy();

	bool CanAimAtCombatant(Combatant *_combatant);

	void AnnounceAndStartAbilityAnimation();
	void StartAbilityAnimation(int _ability);

	void ExecuteAbility();

	void RenderAbilityAnnouncement();
	void RenderAbilityTargetMarker();
};