#pragma once

#include "Player.hpp"


class PlayerOle : public Player
{
public:
	PlayerOle(CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

	virtual int GetID() override { return CombatantID::Ole; }

private:
};

