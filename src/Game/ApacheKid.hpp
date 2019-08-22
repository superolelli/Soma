#pragma once

#include "Enemy.hpp"


class ApacheKid : public Enemy
{
public:

	ApacheKid(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);
	void SetCallbackForAddingEnemy(std::function<void(int)> func) { addEnemy = func; }

private:
	void ChooseAbility() override;
	void ApplyAbilityEffectToTarget(Combatant *_target, AbilityEffect &_effect) override;

	bool OnlyOneCompanionLeft();

	std::function<void(int)> addEnemy;
};