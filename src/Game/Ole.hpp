#pragma once

#include "Player.hpp"


class PlayerOle : public Player
{
public:
	PlayerOle(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);
	virtual void Init() override;

	virtual int GetID() override { return CombatantID::Ole; }

private:
};

