#pragma once

#include "Player.hpp"


class PlayerSimon : public Player
{
public:
	PlayerSimon(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);
	virtual void Init() override;

	virtual int GetID() override { return CombatantID::Simon; }

private:
};