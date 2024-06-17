#pragma once

#include "Player.hpp"


class PlayerSimon : public Player
{
public:
	PlayerSimon(CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

	virtual int GetID() override { return CombatantID::Simon; }

private:
};