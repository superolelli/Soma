#pragma once

#include "Enemy.hpp"
#include "CombatantState.hpp"

class EnemyState : public CombatantState
{
public:

	EnemyState(Enemy *_context);

protected:
	Enemy *enemyContext;
};