#include "Enemy.hpp"
#include "CombatantStateIdle.hpp"
#include "EnemyStatePrepareAbility.hpp"
#include "CombatantStateUpdateStatus.hpp"
#include "CombatantStateDodging.hpp"
#include "CombatantStateAttacked.hpp"
#include "Battle.hpp"


Enemy::Enemy(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer, Battle *_battle)
	: Combatant(_id, _engine, _notificationRenderer)
{
	enemyID = _id;
	battle = _battle;
}

void Enemy::Init()
{
	switch (enemyID)
	{
	case CombatantID::Gesetzloser:
		combatantObject = g_pModels->modelGesetzloser->getNewEntityInstance("Gesetzloser");
		break;
	case CombatantID::Abtruenniger:
		combatantObject = g_pModels->modelAbtruenniger->getNewEntityInstance("Abtruenniger");
		break;
	case CombatantID::Indianer:
		combatantObject = g_pModels->modelIndianer->getNewEntityInstance("Indianer");
		break;
	case CombatantID::Hilfssheriff:
		combatantObject = g_pModels->modelHilfssheriff->getNewEntityInstance("Hilfssheriff");
		break;
	case CombatantID::Greg:
		combatantObject = g_pModels->modelGreg->getNewEntityInstance("Greg");
		break;
	case CombatantID::Apachekid:
		combatantObject = g_pModels->modelApacheKid->getNewEntityInstance("ApacheKid");
		break;
	case CombatantID::BigSpencer:
		combatantObject = g_pModels->modelBigSpencer->getNewEntityInstance("BigSpencer");
		break;
	}
	
	status.SetStats(g_pObjectProperties->enemyStats[enemyID]);

	Combatant::Init();
}


void Enemy::Update()
{
	Combatant::Update();
}


void Enemy::Render()
{
	currentState->Render();
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
		currentState = new CombatantStateDodging(this);
		break;
	case attacked:
		currentState = new CombatantStateAttacked(this);
		break;
	case ready:
		currentState = new EnemyStatePrepareAbility(this);
		break;
	case handlingStatus:
		currentState = new CombatantStateUpdateStatus(this);
		break;
	}
}

