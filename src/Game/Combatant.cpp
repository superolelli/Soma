#include "Combatant.hpp"

bool Combatant::setElapsedTimeForAbilityEffect;


void Combatant::SetPos(int _x, int _y)
{
	combatantObject->setPosition(SpriterEngine::point(_x, _y));

	ReloadHitbox();

	healthBar.SetPos(GetRect().left + GetRect().width / 2 - healthBar.GetRect().width / 2, GetRect().top + GetRect().height + 30);
}


void Combatant::Scale(float _x, float _y)
{
	combatantObject->setScale(SpriterEngine::point(_x, _y));
	ReloadHitbox();
}


void Combatant::ReloadHitbox()
{
	combatantObject->reprocessCurrentTime();

	SpriterEngine::UniversalObjectInterface* hitboxObj = combatantObject->getObjectInstance("bounding_box");

	hitbox.left = hitboxObj->getPosition().x;
	hitbox.top = hitboxObj->getPosition().y;
	hitbox.width = hitboxObj->getSize().x * combatantObject->getScale().x;
	hitbox.height = hitboxObj->getSize().y * combatantObject->getScale().y;
}



void Combatant::ReloadAbilityEffectPoint()
{
	combatantObject->reprocessCurrentTime();

	SpriterEngine::UniversalObjectInterface* pointObj = combatantObject->getObjectInstance("ability_effect_point");

	abilityEffectPoint = pointObj->getPosition();
}


void Combatant::RenderHealthBar(sf::RenderTarget & _target)
{
	healthBar.Render(_target);
	RenderStatusSymbols(_target);
}


void Combatant::RenderStatusSymbols(sf::RenderTarget & _target)
{
	int x = healthBar.GetRect().left;
	int y = healthBar.GetRect().top + healthBar.GetRect().height;

	if (status.IsAsleep())
	{
		g_pSpritePool->sleeping.SetPos(x, y);
		g_pSpritePool->sleeping.Render(_target);
		x += 20;
	}

	if (status.IsConfused())
	{
		g_pSpritePool->confused.SetPos(x, y);
		g_pSpritePool->confused.Render(_target);
		x += 20;
	}

	if (status.IsBuffed())
	{
		g_pSpritePool->buff.SetPos(x, y);
		g_pSpritePool->buff.Render(_target);
		x += 20;
	}

	if (status.IsDebuffed())
	{
		g_pSpritePool->debuff.SetPos(x, y);
		g_pSpritePool->debuff.Render(_target);
		x += 20;
	}

	if (status.IsMarked())
	{
		g_pSpritePool->marked.SetPos(x, y);
		g_pSpritePool->marked.Render(_target);
	}
}


void Combatant::RenderAbilityEffects()
{
	ReloadAbilityEffectPoint();

	g_pSpritePool->abilityEffectsAnimation->setPosition(SpriterEngine::point(0, 0));
	g_pSpritePool->abilityEffectsAnimation->reprocessCurrentTime();
	SpriterEngine::UniversalObjectInterface* pointObj = g_pSpritePool->abilityEffectsAnimation->getObjectInstance("attaching_point");

	g_pSpritePool->abilityEffectsAnimation->setPosition(SpriterEngine::point(abilityEffectPoint.x - pointObj->getPosition().x, abilityEffectPoint.y - pointObj->getPosition().y));

	if (Combatant::setElapsedTimeForAbilityEffect == false)
	{
		g_pSpritePool->abilityEffectsAnimation->setTimeElapsed((float)g_pTimer->GetElapsedTime().asMilliseconds() * ABILITY_EFFECT_ANIMATION_SPEED);
		Combatant::setElapsedTimeForAbilityEffect = true;
	}
	else
		g_pSpritePool->abilityEffectsAnimation->setTimeElapsed(0);

	if (GetAbilityStatus() != dodging)
	{
		g_pSpritePool->abilityEffectsAnimation->render();
		g_pSpritePool->abilityEffectsAnimation->playSoundTriggers();
	}
}



void Combatant::StartTargetsAttackedAnimation()
{
	for (Combatant *c : selectedTargets)
	{
		if (c != this)
		{
			if ((c->IsPlayer() ^ this->IsPlayer()) || actsInConfusion) //^ = XOR
			{
				if (c->CheckForDodging(this))
					c->StartDodgingAnimation();
				else
					c->StartAttackedAnimation();
			}
			else
				c->StartFriendlyAttackedAnimation();
		}
	}
}



void Combatant::StopTargetsAttackedAnimation()
{
	for (Combatant *c : selectedTargets)
	{
		if (c != this)
			c->StopAttackedAnimation();
	}
}

void Combatant::ScaleForAbilityAnimation()
{
	lastPosition = combatantObject->getPosition();

	Scale(0.8, 0.8);
	SetPos(lastPosition.x - engine->GetViewPosition().x, 800);
}


void Combatant::ReverseScaleForAbilityAnimation()
{
	Scale(0.6, 0.6);
	SetPos(lastPosition.x, lastPosition.y);
}

void Combatant::SetAnimation(std::string _animation, float _speed)
{
	combatantObject->setCurrentAnimation(_animation);
	combatantObject->setPlaybackSpeedRatio(_speed);
	combatantObject->setCurrentTime(0);
}


void Combatant::GiveTurnTo(std::vector<Combatant*>* _targets, BattleGUI *_gui)
{
	allCombatants = _targets;
	gui = _gui;

	abilityStatus = ready;

	if (status.IsAsleep())
	{
		status.HandleStatusChanges();
		SetAnimation("idle", IDLE_ANIMATION_SPEED);
		abilityStatus = finished;
		return;
	}

	status.HandleStatusChanges();
}



void Combatant::StartAttackedAnimation()
{
	ScaleForAbilityAnimation();
	SetAnimation("attacked", ABILITY_ANIMATION_SPEED);
	abilityStatus = attacked;
}

void Combatant::StartDodgingAnimation()
{
	ScaleForAbilityAnimation();
	SetAnimation("attacked", ABILITY_ANIMATION_SPEED);
	notificationRenderer->AddNotification("Ausgewichen!", g_pFonts->f_kingArthur, sf::Vector2f(GetRect().left + GetRect().width/2.0f, GetRect().top), 1.0f);

	abilityStatus = dodging;
}

void Combatant::StartFriendlyAttackedAnimation()
{
	ScaleForAbilityAnimation();
	SetAnimation("attacked_friendly", ABILITY_ANIMATION_SPEED);
	abilityStatus = attacked;
}


void Combatant::StopAttackedAnimation()
{
	if(status.IsAsleep())
		SetAnimation("sleeping", IDLE_ANIMATION_SPEED);
	else
		SetAnimation("idle", IDLE_ANIMATION_SPEED);

	ReverseScaleForAbilityAnimation();
	abilityStatus = finished;
}


void Combatant::Quit()
{
	SAFE_DELETE(combatantObject);
}

void Combatant::Update()
{
	healthBar.Update(g_pTimer->GetElapsedTime().asSeconds());
}




bool Combatant::AbilityEffectIsPlaying()
{
	if (g_pSpritePool->abilityEffectsAnimation->currentAnimationName() == "empty")
		return false;
	else
		return g_pSpritePool->abilityEffectsAnimation->animationIsPlaying();
}


bool Combatant::CheckForDodging(Combatant *_attacker)
{
	int difference = status.GetDodge() - _attacker->Status().GetPrecision();
	if (rand()%100 < difference * 2)
		return true;

	return false;
}


void Combatant::RenderAbilityTargetMarker()
{
	int xPos = GetRect().left + (GetRect().width - g_pSpritePool->abilityTargetMarker.GetRect().width) / 2;
	int yPos = healthBar.GetRect().top + healthBar.GetRect().height + 20;
	g_pSpritePool->abilityTargetMarker.SetPos(xPos, yPos);
	g_pSpritePool->abilityTargetMarker.Render(engine->GetWindow());
}

void Combatant::RenderTurnMarker()
{
	int xPos = GetRect().left + (GetRect().width - g_pSpritePool->turnMarker.GetRect().width) / 2;
	int yPos = healthBar.GetRect().top + healthBar.GetRect().height + 20;
	g_pSpritePool->turnMarker.SetPos(xPos, yPos);
	g_pSpritePool->turnMarker.Render(engine->GetWindow());
}



void Combatant::ApplyAbilityEffectToTarget(Combatant * _target, AbilityEffect & _effect)
{
	if (_effect.damageFactor != 0)
		_target->Status().LooseHealth(status.GetDamage() * _effect.damageFactor);

	if (_effect.heal != 0)
		_target->Status().GainHealth(_effect.heal);

	if (_effect.healSelf != 0)
		status.GainHealth(_effect.healSelf);

	if (_effect.confusion != 0)
	{
		if ((rand() % 100) + 1 <= _effect.confusionProbability * 100.0f)
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