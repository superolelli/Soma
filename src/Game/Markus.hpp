#pragma once


#include "Player.hpp"
#include "PlayerEnum.hpp"


class PlayerMarkus : public Player
{
public:
	virtual void Init(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer) override;

	virtual int GetID() override { return PlayerID::Markus; }

private:

};