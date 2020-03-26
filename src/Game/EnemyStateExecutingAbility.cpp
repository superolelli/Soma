#include "EnemyStateExecutingAbility.hpp"
#include "CombatantStateIdle.hpp"
#include "Battle.hpp"



EnemyStateExecutingAbility::EnemyStateExecutingAbility(Enemy *_context, enemyAbilities _chosenAbility)
	:EnemyState(_context)
{
	chosenAbility = static_cast<int>(_chosenAbility);
	StartAbilityAnimation();
	enemyContext->StartTargetsAttackedAnimation(g_pObjectProperties->enemyAbilities[chosenAbility].precisionModificator);
}

void EnemyStateExecutingAbility::Update()
{
	if (!enemyContext->combatantObject->animationIsPlaying() && !enemyContext->AbilityEffectIsPlaying())
		ExecuteAbility();
}

void EnemyStateExecutingAbility::ExecuteAbility()
{

	if (chosenAbility == int(enemyAbilities::indians))
	{
		for (int i = 0; i < 3; i++)
			enemyContext->battle->AddEnemy(CombatantID::Indianer);
	}
	else
		ApplyAbilityEffect();

	enemyContext->ReverseScaleForAbilityAnimation();
	enemyContext->StopTargetsAttackedAnimation();

	CombatantStateIdle *newState = new CombatantStateIdle(enemyContext);
	enemyContext->ChangeState(newState);
}


void EnemyStateExecutingAbility::ApplyAbilityEffect()
{
	auto &ability = g_pObjectProperties->enemyAbilities[chosenAbility];
	for (Combatant *t : enemyContext->selectedTargets)
	{
		if (!t->IsPlayer() || enemyContext->actsInConfusion)
		{
			float additionalDamage = ability.effectFriendly.lessTargetsMoreDamage * (ability.possibleAims.howMany - enemyContext->selectedTargets.size());
			enemyContext->ApplyAbilityEffectToTarget(t, ability.effectFriendly, additionalDamage);
		}
		else if (t->GetAbilityStatus() != dodging)
		{
			float additionalDamage = ability.effectHostile.lessTargetsMoreDamage * (ability.possibleAims.howMany - enemyContext->selectedTargets.size());
			enemyContext->ApplyAbilityEffectToTarget(t, ability.effectHostile, additionalDamage);
		}
	}
}

void EnemyStateExecutingAbility::Render()
{
	enemyContext->combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	enemyContext->combatantObject->render();
	enemyContext->combatantObject->playSoundTriggers();
}

void EnemyStateExecutingAbility::StartAbilityAnimation()
{
	enemyContext->ScaleForAbilityAnimation();                           //order is relevant because "bang" has no hitbox (needed for animation calculation)

	enemyContext->SetAnimation(g_pObjectProperties->enemyAbilities[chosenAbility].animation, ABILITY_ANIMATION_SPEED);

	if (!enemyContext->selectedTargets[0]->IsPlayer())
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(g_pObjectProperties->enemyAbilities[chosenAbility].effectAnimationFriendly);
	else
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(g_pObjectProperties->enemyAbilities[chosenAbility].effectAnimationHostile);

	g_pSpritePool->abilityEffectsAnimation->setCurrentTime(0);
}
