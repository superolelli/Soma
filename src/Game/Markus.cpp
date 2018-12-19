#include "Markus.hpp"




void PlayerMarkus::Init(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer)
{
	combatantObject = g_pModels->modelMarkus->getNewEntityInstance("Markus");

	this->Player::Init(_id, _engine, _notificationRenderer);
}
