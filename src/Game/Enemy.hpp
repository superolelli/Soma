#pragma once

#include "Combatant.hpp"
#include "Resources\Resources.hpp"

#include <numeric>

class Battle;

class Enemy : public Combatant
{
public:
	Enemy(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

	virtual void Init() override;

	virtual int GetID() override { return enemyID; }
	bool IsPlayer() override { return false; }

	void Update() override;

	virtual void Render() override;

	abilityPhase GetAbilityStatus() override;

	void SetAbilityStatus(abilityPhase _status) override;

protected:

	int enemyID;
};