#pragma once


#include "Player.hpp"
#include "PlayerEnum.hpp"


class PlayerMarkus : public Player
{
public:
	virtual void Init(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer) override;

	virtual int GetID() override { return PlayerID::Markus; }

	void AttackedBy(int _battlePos);
	void ResetFistOfRevenge();
	bool CanAimFistOfRevengeAt(int _battlePos);

private:

	bool wasAttackedLastRoundBy[4];

};