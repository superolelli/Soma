#include "PlayerStateExecutingAbility.hpp"
#include "Markus.hpp"
#include "CombatantStateIdle.hpp"

PlayerStateExecutingAbility::PlayerStateExecutingAbility(Player * _context)
	:PlayerState(_context)
{
	StartAbilityAnimation(playerContext->gui->GetCurrentAbility());
	playerContext->StartTargetsAttackedAnimation(g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()].precisionModificator);
}

void PlayerStateExecutingAbility::Update()
{
	if (!playerContext->combatantObject->animationIsPlaying() && !playerContext->AbilityEffectIsPlaying())
		ExecuteAbility();
}


void PlayerStateExecutingAbility::ExecuteAbility()
{
	ApplyAbilityEffect();

	playerContext->ReverseScaleForAbilityAnimation();
	playerContext->StopTargetsAttackedAnimation();

	if (dynamic_cast<PlayerMarkus*>(this))
		dynamic_cast<PlayerMarkus*>(this)->ResetFistOfRevenge();

	CombatantStateIdle *newState = new CombatantStateIdle(playerContext);
	playerContext->ChangeState(newState);
}


void PlayerStateExecutingAbility::ApplyAbilityEffect()
{
	auto &ability = g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()];

	for (Combatant *t : playerContext->selectedTargets)
	{
		if (t->IsPlayer() && !playerContext->actsInConfusion || !t->IsPlayer() && playerContext->actsInConfusion)
		{
			float additionalDamage = ability.effectFriendly.lessTargetsMoreDamage * (ability.possibleAims.howMany - playerContext->selectedTargets.size());
			playerContext->ApplyAbilityEffectToTarget(t, ability.effectFriendly, additionalDamage);
		}
		else if (t->GetAbilityStatus() != dodging)
		{
			float additionalDamage = ability.effectHostile.lessTargetsMoreDamage * (ability.possibleAims.howMany - playerContext->selectedTargets.size());
			playerContext->ApplyAbilityEffectToTarget(t, ability.effectHostile, additionalDamage);
		}
	}
}


void PlayerStateExecutingAbility::Render()
{
	playerContext->combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	playerContext->combatantObject->render();
	playerContext->combatantObject->playSoundTriggers();
}


void PlayerStateExecutingAbility::StartAbilityAnimation(int _ability)
{
	playerContext->ScaleForAbilityAnimation();

	playerContext->SetAnimation(g_pObjectProperties->playerAbilities[playerContext->GetID()][_ability].animation, ABILITY_ANIMATION_SPEED);

	if (playerContext->selectedTargets[0]->IsPlayer() && !playerContext->actsInConfusion || !playerContext->selectedTargets[0]->IsPlayer() && playerContext->actsInConfusion)
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(g_pObjectProperties->playerAbilities[playerContext->GetID()][_ability].effectAnimationFriendly);
	else
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(g_pObjectProperties->playerAbilities[playerContext->GetID()][_ability].effectAnimationHostile);

	g_pSpritePool->abilityEffectsAnimation->setCurrentTime(0);
}
