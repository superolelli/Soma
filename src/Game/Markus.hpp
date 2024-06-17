#pragma once

#include "Player.hpp"


class PlayerMarkus : public Player
{
public:
	PlayerMarkus(CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

	virtual int GetID() override { return CombatantID::Markus; }

	void AttackedBy(int _battlePos);
	void ResetFistOfRevenge();
	bool CanAimFistOfRevengeAt(int _battlePos);

private:

	bool wasAttackedLastRoundBy[4];

};