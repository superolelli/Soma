#include "Greg.hpp"
#include "ObserverNotificationBattle.h"

GregDigger::GregDigger(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: companionDiedLastRound{false}
	, Enemy(_id, _engine, _notificationRenderer)
{
}


void GregDigger::OnNotify(ObserverNotification &_notification)
{
	auto notification = dynamic_cast<ObserverNotificationBattle*>(&_notification);

	if(notification->event == battleEvents::enemyDied)
		companionDiedLastRound = true;
}


void GregDigger::ChooseAbility()
{
	if (CompanionDiedLastRound() && status.GetCurrentHealth() < status.GetMaxHealth())
	{
		chosenAbility = enemyAbilities::bury_the_dead;
	}
	else
		if (rand() % 2 == 0)
			chosenAbility = enemyAbilities::shovel_hit;
		else
			chosenAbility = enemyAbilities::gravedigger_gaze;

	SetCompanionDiedLastRound(false);
}
