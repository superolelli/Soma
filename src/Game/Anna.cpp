#include "Anna.hpp"

PlayerAnna::PlayerAnna(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Player(_id, _engine, _notificationRenderer)
{
}

void PlayerAnna::Init()
{
	combatantObject = g_pModels->GetNewCombatantModelInstance(CombatantID::Anna);
	this->Player::Init();
}
