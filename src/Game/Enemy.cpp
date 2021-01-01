#include "Enemy.hpp"
#include "CombatantStateIdle.hpp"
#include "EnemyStatePrepareAbility.hpp"
#include "CombatantStateUpdateStatus.hpp"
#include "CombatantStateAttacked.hpp"
#include "CombatantStateDying.hpp"
#include "Battle.hpp"


Enemy::Enemy(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Combatant(_id, _engine, _notificationRenderer)
{
	enemyID = _id;
}

void Enemy::Init()
{
    combatantObject = g_pModels->GetNewCombatantModelInstance(CombatantID(enemyID));
	status.SetStats(g_pObjectProperties->enemyStats[enemyID]);

    if (enemyID == CombatantID::BillNoface)
        status.SetNofaceBuffLevel(0);

	Combatant::Init();
}


void Enemy::Update()
{
	Combatant::Update();
}


abilityPhase Enemy::GetAbilityStatus()
{
	return currentState->GetStateID();
}

void Enemy::SetAbilityStatus(abilityPhase _status)
{
	SAFE_DELETE(currentState);

	switch (_status)
	{
	case finished:
		currentState = new CombatantStateIdle(this);
		break;
	case dodging:
		currentState = new CombatantStateAttacked(this, true);
		break;
	case attacked:
		currentState = new CombatantStateAttacked(this, false);
		break;
	case ready:
		currentState = new EnemyStatePrepareAbility(this);
		break;
	case handlingStatus:
		currentState = new CombatantStateUpdateStatus(this);
		break;
	case dying:
		currentState = new CombatantStateDying(this);
	}
}

