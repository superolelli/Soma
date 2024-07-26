#include "Enemy.hpp"
#include "CombatantStateIdle.hpp"
#include "EnemyStatePrepareAbility.hpp"
#include "CombatantStateUpdateStatus.hpp"
#include "CombatantStateAttacked.hpp"
#include "CombatantStateDying.hpp"
#include "Battle.hpp"


Enemy::Enemy(CombatantID _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Combatant(_id, _engine, _notificationRenderer, g_pModels->GetNewCombatantModelInstance(CombatantID(_id)), g_pObjectProperties->enemyStats[_id])
{
	if (combatantID == CombatantID::BillNoface)
		status.SetNofaceBuffLevel(0);
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
	switch (_status)
	{
	case finished:
		ChangeState(new CombatantStateIdle(this));
		break;
	case dodging:
		ChangeState(new CombatantStateAttacked(this, true));
		break;
	case attacked:
		ChangeState(new CombatantStateAttacked(this, false));
		break;
	case ready:
		ChangeState(new EnemyStatePrepareAbility(this));
		break;
	case handlingStatus:
		ChangeState(new CombatantStateUpdateStatus(this));
		break;
	case dying:
		ChangeState(new CombatantStateDying(this));
	}
}

