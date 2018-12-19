#include "Player.hpp"



void Player::Init(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer)
{

	engine = _engine;
	notificationRenderer = _notificationRenderer;

	SetAnimation("idle", IDLE_ANIMATION_SPEED);

	Scale(PLAYER_SCALE, PLAYER_SCALE);
	combatantObject->reprocessCurrentTime();

	status.SetStats(g_pObjectProperties->playerStats[_id]);

	ReloadHitbox();

	healthBar.Load(g_pTextures->healthBar, g_pTextures->healthBarFrame, status.GetCurrentHealthPointer(), status.GetMaxHealthPointer());
	healthBar.SetPos(GetRect().left + GetRect().width / 2 - healthBar.GetRect().width / 2, GetRect().top + GetRect().height + 30);
	healthBar.SetSmoothTransformationTime(0.7f);

	status.Reset();

	is_walking = false;

	abilityStatus = finished;
}


int Player::NumberOfTargetsForCurrentAbility()
{
	return g_pObjectProperties->playerAbilities[GetID()][gui->GetCurrentAbility()].possibleAims.howMany;
}


void Player::Update(int _xMove, bool _is_walking)
{
	SetPos(combatantObject->getPosition().x + _xMove, combatantObject->getPosition().y);

	if (abilityStatus != executing && abilityStatus != attacked)
	{
		if (is_walking == false && _is_walking == true)
		{
			is_walking = true;
			SetAnimation("walk", WALKING_ANIMATION_SPEED);
		}
		else if (is_walking == true && _is_walking == false)
		{
			is_walking = false;
			SetAnimation("idle", IDLE_ANIMATION_SPEED);
		}
	}


	if (abilityStatus == ready && AimChosen())
	{
		abilityStatus = executing;
		StartAbilityAnimation(gui->GetCurrentAbility());
		StartTargetsAttackedAnimation();
	}
	

	if (abilityStatus == executing)
		DoCurrentAbility();
}



void Player::Render()
{
	combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	combatantObject->render();
	combatantObject->playSoundTriggers();

	setElapsedTimeForAbilityEffect = false;

	if (abilityStatus == attacked || abilityStatus == dodging)
		RenderAbilityEffects();

	if (abilityStatus == ready)
	{
		RenderTurnMarker();
		RenderAbilityTargetMarker();
	}
}


void Player::RenderAbilityTargetMarker()
{
	for (Combatant *c : (*allCombatants))
	{
		if (c->GetRect().contains(engine->GetWorldMousePos()) && CurrentAbilityCanAimAtCombatant(c))
		{
			int targetPosition = c->GetBattlePos();

			for (Combatant* c : (*allCombatants))
			{
				if (c->GetBattlePos() >= targetPosition && c->GetBattlePos() < targetPosition + NumberOfTargetsForCurrentAbility())
					c->RenderAbilityTargetMarker();				
			}

			return;
		}
	}
}


bool Player::AimChosen()
{
	if (engine->GetButtonstates(ButtonID::Left) == Keystates::Released)
	{
		for (Combatant* c : (*allCombatants))
		{
			if (CurrentAbilityCanAimAtCombatant(c) && CombatantClicked(c))
			{
				selectedTargets.push_back(c);
				SelectAdditionalTargets();

				return true;
			}
		}
	}

	return false;
}



void Player::SelectAdditionalTargets()
{
	int targetPosition = selectedTargets[0]->GetBattlePos();

	for (Combatant* c : (*allCombatants))
	{
		if (c->GetBattlePos() > targetPosition && c->GetBattlePos() < targetPosition + NumberOfTargetsForCurrentAbility())
			selectedTargets.push_back(c);
	}
}



bool Player::CurrentAbilityCanAimAtCombatant(Combatant* _combatant)
{
	return g_pObjectProperties->playerAbilities[GetID()][gui->GetCurrentAbility()].possibleAims.position[_combatant->GetBattlePos()];
}



bool Player::CombatantClicked(Combatant* _combatant)
{
	return _combatant->GetRect().contains(engine->GetWorldMousePos());
}



void Player::DoCurrentAbility()
{
	if (!combatantObject->animationIsPlaying() && !AbilityEffectIsPlaying())
	{
		HandleConfusion();

		auto &ability = g_pObjectProperties->playerAbilities[GetID()][gui->GetCurrentAbility()];

		for (Combatant *t : selectedTargets)
		{
			if (t->IsPlayer())
				ApplyAbilityEffectToTarget(t, ability.effectFriendly);
			else if(t->GetAbilityStatus() != dodging)
				ApplyAbilityEffectToTarget(t, ability.effectHostile);
		}

		SetAnimation("idle", IDLE_ANIMATION_SPEED);
		ReverseScaleForAbilityAnimation();
		StopTargetsAttackedAnimation();
		
		selectedTargets.clear();
		abilityStatus = finished;
	}
}


void Player::ApplyAbilityEffectToTarget(Combatant * _target, AbilityEffect & _effect)
{
	if (_effect.damageFactor != 0)
		_target->Status().LooseHealth(status.GetDamage() * _effect.damageFactor);

	if (_effect.heal != 0)
		_target->Status().GainHealth(_effect.heal);

	if (_effect.healSelf != 0)
		status.GainHealth(_effect.healSelf);

	if (_effect.confusion != 0)
	{
		if((rand() % 100) + 1 <= _effect.confusionProbability * 100.0f)
			_target->Status().Confuse(_effect.confusion);
	}

	if (_effect.mark != 0)
		_target->Status().Mark(_effect.mark);

	if (_effect.putToSleepProbability != 0.0f)
	{
		if ((rand() % 100) + 1 <= _effect.putToSleepProbability * 100.0f)
			_target->Status().PutToSleep();
	}

	if (_effect.removeBuffs)
		_target->Status().RemoveAllBuffs();

	if (_effect.removeDebuffs)
		_target->Status().RemoveAllDebuffs();

	if (_effect.buff.duration != 0)
	{
		if (_effect.buff.isPositive)
			_target->Status().AddBuff(_effect.buff);
		else
			_target->Status().AddDebuff(_effect.buff);
	}
}



void Player::HandleConfusion()
{
	if (status.IsConfused())
	{
		if (rand() % 4 == 0)
			status.LooseHealth(1);
	}
}



void Player::StartAbilityAnimation(int _ability)
{
	ScaleForAbilityAnimation();

	SetAnimation(g_pObjectProperties->playerAbilities[GetID()][_ability].animation, ABILITY_ANIMATION_SPEED);

	if (selectedTargets[0]->IsPlayer())
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(g_pObjectProperties->playerAbilities[GetID()][_ability].effectAnimationFriendly);
	else
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(g_pObjectProperties->playerAbilities[GetID()][_ability].effectAnimationHostile);

	g_pSpritePool->abilityEffectsAnimation->setCurrentTime(0);
}