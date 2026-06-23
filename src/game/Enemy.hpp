#pragma once

#include "Combatant.hpp"
#include "Resources\Resources.hpp"

#include <numeric>

class Battle;

class Enemy : public Combatant
{
public:

	Enemy(CombatantID _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

	bool IsPlayer() override { return false; }

	void Update() override;

	abilityPhase GetAbilityStatus() override;

	void SetAbilityStatus(abilityPhase _status) override;

protected:
};