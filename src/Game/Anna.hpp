#pragma once

#include "Player.hpp"


class PlayerAnna: public Player
{
public:
	PlayerAnna(CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

	virtual int GetID() override { return CombatantID::Anna; }

private:

};