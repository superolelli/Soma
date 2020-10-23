#include "Ole.hpp"


PlayerOle::PlayerOle(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Player(_id, _engine, _notificationRenderer)
{
}

void PlayerOle::Init()
{
	combatantObject = g_pModels->combatantModels[CombatantID::Ole]->getNewEntityInstance("Ole");

	this->Player::Init();
}
