#include "CombatantStateExecutingAbility.hpp"
#include "CombatantStateIdle.hpp"
#include "Battle.hpp"


CombatantStateExecutingAbility::CombatantStateExecutingAbility(Combatant * _context, Ability * _ability)
	:CombatantState(_context)
{
	ability = _ability;
	StartAbilityAnimation();
	AttackTargets();
}


void CombatantStateExecutingAbility::Update()
{
	if (!context->combatantObject->animationIsPlaying() && !g_pSpritePool->AbilityEffectIsPlaying())
		ExecuteAbility();
}


void CombatantStateExecutingAbility::Render()
{
	context->combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	context->combatantObject->render();
	context->combatantObject->playSoundTriggers();
}


void CombatantStateExecutingAbility::ExecuteAbility()
{
	if (ability->name =="Indianer!")
	{
		for (int i = 0; i < 3; i++)
			context->battle->AddEnemy(CombatantID::Indianer);
	}
	else
		ApplyAbilityEffect();

	context->ReverseScaleForAbilityAnimation();
	StopTargetsAttackedAnimation();

	if (dynamic_cast<PlayerMarkus*>(context))
		dynamic_cast<PlayerMarkus*>(context)->ResetFistOfRevenge();

	CombatantStateIdle *newState = new CombatantStateIdle(context);
	context->ChangeState(newState);
}


void CombatantStateExecutingAbility::ApplyAbilityEffect()
{
	for (Combatant *t : context->selectedTargets)
	{
		if (context->IsAlly(t) && !context->actsInConfusion || !context->IsAlly(t) && context->actsInConfusion)
		{
			float additionalDamage = ability->effectFriendly.lessTargetsMoreDamage * (ability->possibleAims.howMany - context->selectedTargets.size());
			t->ApplyAbilityEffect(context, ability->effectFriendly, additionalDamage);
		}
		else if (t->GetAbilityStatus() != dodging)
		{
			float additionalDamage = ability->effectHostile.lessTargetsMoreDamage * (ability->possibleAims.howMany - context->selectedTargets.size());
			t->ApplyAbilityEffect(context, ability->effectHostile, additionalDamage);
		}
	}
}


void CombatantStateExecutingAbility::StartAbilityAnimation()
{
	context->ScaleForAbilityAnimation();
	context->SetAnimation(ability->animation, ABILITY_ANIMATION_SPEED);

	if (context->IsAlly(context->selectedTargets[0]) && !context->actsInConfusion || !context->IsAlly(context->selectedTargets[0]) && context->actsInConfusion)
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(ability->effectAnimationFriendly);
	else
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(ability->effectAnimationHostile);

	g_pSpritePool->abilityEffectsAnimation->setCurrentTime(0);
}


void CombatantStateExecutingAbility::AttackTargets()
{
	for (Combatant *c : context->selectedTargets)
	{
		if (c == context)
			continue;

		if (dynamic_cast<PlayerMarkus*>(c) != nullptr)
			dynamic_cast<PlayerMarkus*>(c)->AttackedBy(context->battlePosition);

		StartAttackedAnimation(c);
	}
}


void CombatantStateExecutingAbility::StartAttackedAnimation(Combatant *_combatant)
{
	if (context->IsAlly(_combatant) && !context->actsInConfusion)
		_combatant->StartFriendlyAttackedAnimation();
	else
	{
		if (CheckForDodging(context, _combatant))
			_combatant->StartDodgingAnimation();
		else
			_combatant->StartAttackedAnimation();
	}	
}


bool CombatantStateExecutingAbility::CheckForDodging(Combatant *_attacker, Combatant *_defender)
{
	int difference = _defender->status.GetDodge() - (_attacker->Status().GetPrecision() + ability->precisionModificator);
	if (rand() % 100 < difference * 2)
		return true;

	return false;
}


void CombatantStateExecutingAbility::StopTargetsAttackedAnimation()
{
	for (Combatant *c : context->selectedTargets)
	{
		if (c != context)
			c->StopAttackedAnimation();
	}
}