#include "ApacheKid.hpp"
#include "ApacheKid.hpp"

ApacheKid::ApacheKid(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Enemy(_id, _engine, _notificationRenderer)
{
}


void ApacheKid::ChooseAbility()
{
	if (OnlyOneCompanionLeft())
		chosenAbility = enemyAbilities::indians;
	else
		chosenAbility = enemyAbilities::knife;
}


void ApacheKid::ApplyAbilityEffectToTarget(Combatant * _target, AbilityEffect & _effect, float _additionalDamageFactor)
{
	if (chosenAbility == enemyAbilities::indians)
	{
		for(int i = 0; i < 3; i++)
			addEnemy(CombatantID::Indianer);
	}
	else
	{
		Combatant::ApplyAbilityEffectToTarget(_target, _effect, _additionalDamageFactor);
	}
}



bool ApacheKid::OnlyOneCompanionLeft()
{
	return std::count_if(allCombatants->begin(), allCombatants->end(), [&](Combatant *c) {return !c->IsPlayer(); }) <= 2;
}