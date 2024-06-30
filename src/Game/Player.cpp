#include "Player.hpp"
#include <numeric>
#include "CombatantStateAttacked.hpp"
#include "CombatantStateIdle.hpp"
#include "CombatantStateUpdateStatus.hpp"
#include "CombatantStateDying.hpp"
#include "PlayerStatePrepareAbility.hpp"

Player::Player(CombatantID _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer, SpriterEngine::EntityInstance* _combatantObject)
	: Combatant(_id, _engine, _notificationRenderer, _combatantObject)
	, is_walking(false)
{
	status.SetStats(g_pObjectProperties->playerStats[GetID()]);
}


void Player::SetEquipment(CombatantAttributes &_equipmentStats)
{
	status.AddStats(_equipmentStats);
}

float Player::GetAdditionalDamageForCurrentlyAimedCombatant()
{
	if (auto state = dynamic_cast<PlayerStatePrepareAbility*>(currentState))
		return state->GetAdditionalDamageForCurrentlyAimedCombatant();
	
	return 0.0f;
}


void Player::Update(int _xMove, bool _is_walking)
{
	Combatant::Update();
	SetPos(combatantObject->getPosition().x + _xMove, combatantObject->getPosition().y);

	if (GetAbilityStatus() != executing && GetAbilityStatus() != attacked)
	{
		if (is_walking == false && _is_walking == true)
		{
			is_walking = true;
			SetAnimation("walk", WALKING_ANIMATION_SPEED);
		}
		else if (is_walking == true && _is_walking == false)
		{
			is_walking = false;
			SetAnimation("idle", IDLE_ANIMATION_SPEED);
		}
	}
}


void Player::SetAbilityStatus(abilityPhase _status)
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
		currentState = new PlayerStatePrepareAbility(this);
		break;
	case handlingStatus:
		currentState = new CombatantStateUpdateStatus(this);
		break;
	case dying:
		currentState = new CombatantStateDying(this);
	}
}

bool Player::CurrentAbilityCanAimAtCombatant(Combatant * _combatant)
{
	if (auto state = dynamic_cast<PlayerStatePrepareAbility*>(currentState))
		return state->CurrentAbilityCanAimAtCombatant(_combatant);

	return false;
}
