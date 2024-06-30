#include "CombatantStateExecutingAbility.hpp"
#include "CombatantStateIdle.hpp"
#include "Battle.hpp"

CombatantStateExecutingAbility::CombatantStateExecutingAbility(Combatant * _context, Ability * _ability)
	:CombatantState(_context)
{
	ability = _ability;
	CalculateCombatantAnimationPositions();
	StartAbilityAnimation();
	AttackTargets();
}

void CombatantStateExecutingAbility::CalculateCombatantAnimationPositions()
{
	std::sort(context->selectedTargets.begin(), context->selectedTargets.end(), [&](Combatant* c1, Combatant* c2) {return c1->GetBattlePos() < c2->GetBattlePos();});
	int padding = 50;

	if (context->selectedTargets[0] == context && context->selectedTargets.size() == 1)
	{
		combatantAnimationPositions[context->GetBattlePos()] = sf::Vector2i(context->engine->GetWindowSize().x / 2 - context->GetRect().width * COMBATANT_SCALE / 2, 800);
	}
	else if (ability->possibleAims.attackAll)
	{
		for (int i = 0; i < context->allCombatants->size(); i++)
			combatantAnimationPositions[context->allCombatants->at(i)->GetBattlePos()] = sf::Vector2i(context->allCombatants->at(i)->GetRect().left - context->engine->GetViewPosition().x, 800);
	}
	else if (context->IsPlayer() && ability->possibleAims.attackAllPlayers && context->selectedTargets[0]->IsPlayer() || !context->IsPlayer() && ability->possibleAims.attackAllEnemies && !context->selectedTargets[0]->IsPlayer())
	{
		CalculateSelectedTargetsPositions(context->engine->GetWindowSize().x / 2 - CalculateSelectedTargetsTotalWidth(padding) / 2, padding);
	}
	else
	{
		int attackerPosition = context->engine->GetWindowSize().x / 4;
		int attackedPosition = attackerPosition * 2.5;

		if (!context->IsPlayer())
		{
			attackedPosition = context->engine->GetWindowSize().x / 4;
			attackerPosition = attackedPosition * 2.5;
		}

		combatantAnimationPositions[context->GetBattlePos()] = sf::Vector2i(attackerPosition - context->GetRect().width * COMBATANT_SCALE / 2, 800);
		CalculateSelectedTargetsPositions(attackedPosition - CalculateSelectedTargetsTotalWidth(padding) / 2, padding);
	}
}


int CombatantStateExecutingAbility::CalculateSelectedTargetsTotalWidth(int _padding)
{
	int width = 0;
	for (auto* c : context->selectedTargets)
		width += c->GetRect().width * COMBATANT_SCALE + _padding;
	width -= _padding;

	return width;
}

void CombatantStateExecutingAbility::CalculateSelectedTargetsPositions(int _startingPosition, int _padding)
{
	int xPos = _startingPosition;
	for (auto* c : context->selectedTargets)
	{
		combatantAnimationPositions[c->GetBattlePos()] = sf::Vector2i(xPos, 800);
		xPos += c->GetRect().width * COMBATANT_SCALE + _padding;
	}
}


void CombatantStateExecutingAbility::Update()
{
	if (!context->combatantObject->animationIsPlaying() && !g_pSpritePool->AbilityEffectIsPlaying())
		ExecuteAbility();
}


void CombatantStateExecutingAbility::Render()
{
	CombatantState::Render();

	for (auto t : context->selectedTargets)
	{
		if (t == context)
		{
			context->RenderAbilityEffect(false);
			break;
		}
	}

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

	CombatantStateIdle *newState = new CombatantStateIdle(context);
	context->ChangeState(newState);
}


void CombatantStateExecutingAbility::ApplyAbilityEffect()
{
	AbilityEffect *selfEffect = nullptr;

	for (Combatant *t : context->selectedTargets)
	{
		AbilityEffect* effect = nullptr;
		if (context->IsAlly(t) && !context->actsInConfusion || !context->IsAlly(t) && context->actsInConfusion)
			effect = &ability->effectFriendly;
		else if (t->GetAbilityStatus() != dodging)
			effect = &ability->effectHostile;

		if (effect != nullptr)
		{
			float additionalDamage = effect->lessTargetsMoreDamage * (ability->possibleAims.howMany - context->selectedTargets.size());
			t->ApplyAbilityEffect(context, *effect, additionalDamage);
			selfEffect = effect;
		}
	}

	// Handle self effects
	int criticalEffectFactor = 1;
	float criticalDurationFactor = 1;
	bool isCriticalHit = rand() % 100 < context->status.GetAttribute("criticalHit") + ability->effectHostile.criticalHitModificator;
	if (isCriticalHit)
	{
		context->notificationRenderer->AddNotification("Kritisch!", g_pFonts->f_blackwoodCastle, sf::Vector2f(context->GetRect().left + context->GetRect().width / 2.0f, context->GetRect().top - 40), 1.0f);
		g_pSounds->PlaySound(CRITICAL_HIT);
		criticalEffectFactor = 2;
		criticalDurationFactor = 1.5;
	}

	if (ability->fatigue != 0)
		context->battle->GetLevelStatus()->AddFatigue(ability->fatigue * criticalEffectFactor);

	if (selfEffect && selfEffect->healSelf != 0)
	{
		float healingModificator = 1.0f + static_cast<float>(context->Status().GetAttribute("healing")) / 100.0f + criticalEffectFactor;

		if (context->actsInConfusion) {
			context->selectedTargets.front()->status.GainHealth(ability->effectFriendly.healSelf * healingModificator);
		}
		else
			context->status.GainHealth(ability->effectFriendly.healSelf * healingModificator);
	}

	if (selfEffect && selfEffect->addMissSelf != 0) {
		if (context->actsInConfusion)
			context->selectedTargets.front()->status.AddMiss(selfEffect->addMissSelf);
		else
			context->status.AddMiss(selfEffect->addMissSelf);
	}
}


void CombatantStateExecutingAbility::StartAbilityAnimation()
{
	context->ScaleForAbilityAnimation(combatantAnimationPositions[context->GetBattlePos()].x, combatantAnimationPositions[context->GetBattlePos()].y);
	context->SetAnimation(ability->animation, ABILITY_ANIMATION_SPEED);

	if (context->IsAlly(context->selectedTargets[0]) && !context->actsInConfusion || !context->IsAlly(context->selectedTargets[0]) && context->actsInConfusion)
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(ability->effectAnimationFriendly);
	else
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(ability->effectAnimationHostile);

	g_pSpritePool->abilityEffectsAnimation->setCurrentTime(0);
}


void CombatantStateExecutingAbility::AttackTargets()
{
	for (int i = 0; i < context->selectedTargets.size(); i++)
	{
		if (context->selectedTargets[i] == context)
			continue;

		if (context->selectedTargets[i]->combatantID == CombatantID::Markus && 
			(!context->IsAlly(context->selectedTargets[i]) || context->actsInConfusion))
			context->Status().AddFistOfRevengeDebuff();

		StartAttackedAnimation(context->selectedTargets[i], combatantAnimationPositions[context->selectedTargets[i]->GetBattlePos()]);
	}
}


void CombatantStateExecutingAbility::StartAttackedAnimation(Combatant *_combatant, sf::Vector2i _position)
{
	if (context->IsAlly(_combatant) && !context->actsInConfusion || !context->IsAlly(_combatant) && context->actsInConfusion)
		_combatant->StartFriendlyAttackedAnimation(_position.x, _position.y);
	else
	{
		if (CheckForDodging(context, _combatant))
			_combatant->StartDodgingAnimation(_position.x, _position.y);
		else
			_combatant->StartAttackedAnimation(_position.x, _position.y);
	}	
}


bool CombatantStateExecutingAbility::CheckForDodging(Combatant *_attacker, Combatant *_defender)
{
	if (_defender->status.NumberOfMisses() > 0)
	{
		_defender->status.RemoveMiss();
		return true;
	}


	int difference = _defender->status.GetAttribute("dodge") - (_attacker->Status().GetAttribute("precision") + ability->precisionModificator);
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