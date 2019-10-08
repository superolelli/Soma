#pragma once

#include "Enemy.hpp"


class Abtruenniger : public Enemy
{
public:
	Abtruenniger(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

private:
	void ChooseAbility() override;

};