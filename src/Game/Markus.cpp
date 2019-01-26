#include "Markus.hpp"




void PlayerMarkus::Init(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer)
{
	combatantObject = g_pModels->modelMarkus->getNewEntityInstance("Markus");

	this->Player::Init(_id, _engine, _notificationRenderer);

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