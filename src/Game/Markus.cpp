#include "Markus.hpp"


PlayerMarkus::PlayerMarkus(CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Player(CombatantID::Markus, _engine, _notificationRenderer, g_pModels->GetNewCombatantModelInstance(CombatantID::Markus))
	, wasAttackedLastRoundBy{false, false, false, false}
{
}


void PlayerMarkus::ResetFistOfRevenge()
{
	for (auto &b : wasAttackedLastRoundBy)
		b = false;
}


void PlayerMarkus::AttackedBy(int _battlePos)
{
	if(_battlePos > 3 && _battlePos < 8)
		wasAttackedLastRoundBy[_battlePos - 4] = true;
}


bool PlayerMarkus::CanAimFistOfRevengeAt(int _battlePos)
{
	if (_battlePos > 3 && _battlePos < 8)
		return wasAttackedLastRoundBy[_battlePos - 4];
	else
		return false;
}