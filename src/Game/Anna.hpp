#pragma once

#include "Player.hpp"


class PlayerAnna: public Player
{
public:
	PlayerAnna(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);
	virtual void Init() override;

	virtual int GetID() override { return CombatantID::Anna; }

private:

};