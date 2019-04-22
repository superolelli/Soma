#include "Markus.hpp"


PlayerMarkus::PlayerMarkus(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Player(_id, _engine, _notificationRenderer)
{
}

void PlayerMarkus::Init()
{
	combatantObject = g_pModels->modelMarkus->getNewEntityInstance("Markus");

	this->Player::Init();

	for (auto &b : wasAttackedLastRoundBy)
		b = false;
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