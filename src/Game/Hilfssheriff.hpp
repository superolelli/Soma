#pragma once

#include "Enemy.hpp"


class  Hilfssheriff : public Enemy
{
public:
	Hilfssheriff(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

private:
	void ChooseAbility() override;

};