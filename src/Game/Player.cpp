#include "Player.hpp"
#include "Markus.hpp"
#include <numeric>
#include "CombatantStateAttacked.hpp"
#include "CombatantStateDodging.hpp"
#include "CombatantStateIdle.hpp"
#include "CombatantStateUpdateStatus.hpp"
#include "PlayerStatePrepareAbility.hpp"

Player::Player(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Combatant(_id, _engine, _notificationRenderer)
{
}

void Player::Init()
{
	status.SetStats(g_pObjectProperties->playerStats[GetID()]);
	Combatant::Init();
	is_walking = false;
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



void Player::Render()
{
	currentState->Render();
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
		currentState = new CombatantStateDodging(this);
		break;
	case attacked:
		currentState = new CombatantStateAttacked(this);
		break;
	case ready:
		currentState = new PlayerStatePrepareAbility(this);
		break;
	case handlingStatus:
		currentState = new CombatantStateUpdateStatus(this);
		break;
	}
}

bool Player::CurrentAbilityCanAimAtCombatant(Combatant * _combatant)
{
	if (auto state = dynamic_cast<PlayerStatePrepareAbility*>(currentState))
		return state->CurrentAbilityCanAimAtCombatant(_combatant);

	return false;
}
