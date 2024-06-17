#pragma once

#include "Combatant.hpp"
#include "Resources\Resources.hpp"

#include <numeric>

class Battle;

class Enemy : public Combatant
{
public:

	Enemy(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

	virtual int GetID() override { return enemyID; }
	bool IsPlayer() override { return false; }

	void Update() override;

	abilityPhase GetAbilityStatus() override;

	void SetAbilityStatus(abilityPhase _status) override;

protected:

	int enemyID;
};