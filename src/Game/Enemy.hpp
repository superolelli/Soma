#pragma once

#include "Combatant.hpp"
#include "Resources\Resources.hpp"

#include <numeric>

class Battle;

class Enemy : public Combatant
{
	friend class EnemyStatePrepareAbility;
	friend class EnemyStateExecutingAbility;

public:
	Enemy(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer, Battle *_battle);

	virtual void Init() override;

	virtual int GetID() override { return enemyID; }

	void Update() override;

	virtual void Render() override;

	abilityPhase GetAbilityStatus() override;

	void SetAbilityStatus(abilityPhase _status) override;

protected:
	Battle *battle;

	int enemyID;
};