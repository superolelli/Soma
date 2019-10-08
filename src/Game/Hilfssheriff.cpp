#include "Hilfssheriff.hpp"

Hilfssheriff::Hilfssheriff(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Enemy(_id, _engine, _notificationRenderer)
{
}




void Hilfssheriff::ChooseAbility()
{
	for (auto *c : *allCombatants)
	{
		if (c->IsPlayer() && c->Status().IsMarked())
		{
			chosenAbility = enemyAbilities::bang;
			return;
		}
	}


	if (rand() % 2 == 0)
		chosenAbility = enemyAbilities::bang;
	else
		chosenAbility = enemyAbilities::bounty;
}
