#include "Simon.hpp"


PlayerSimon::PlayerSimon(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Player(_id, _engine, _notificationRenderer)
{
}

void PlayerSimon::Init()
{
	combatantObject = g_pModels->modelSimon->getNewEntityInstance("Simon");

	this->Player::Init();
}
