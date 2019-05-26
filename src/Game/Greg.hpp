#pragma once

#include "Enemy.hpp"


class GregDigger : public Enemy
{
public:
	GregDigger(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

	void SetCompanionDiedLastRound(bool _died) { companionDiedLastRound = _died; }

	bool CompanionDiedLastRound() { return companionDiedLastRound; }

private:

	bool companionDiedLastRound;

};