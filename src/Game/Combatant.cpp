#include "Combatant.hpp"
#include "Markus.hpp"
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"

bool Combatant::setElapsedTimeForAbilityEffect;



void Combatant::Init(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer)
{
	engine = _engine;
	notificationRenderer = _notificationRenderer;

	SetAnimation("idle", IDLE_ANIMATION_SPEED);
	Scale(COMBATANT_NORMAL_SCALE, COMBATANT_NORMAL_SCALE);
	combatantObject->reprocessCurrentTime();

	ReloadHitbox();

	status.Init(this, notificationRenderer);
	status.Reset();

	statusBar.Init(&status, engine);

	actsInConfusion = false;
	dying = false;

	abilityStatus = finished;

}

void Combatant::SetPos(int _x, int _y)
{
	combatantObject->setPosition(SpriterEngine::point(_x, _y));

	ReloadHitbox();
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
	hitbox.width = hitboxObj->getSize().x  * hitboxObj->getScale().x;
	hitbox.height = hitboxObj->getSize().y * hitboxObj->getScale().y;
}


void Combatant::ReloadAbilityEffectPoint()
{
	combatantObject->reprocessCurrentTime();

	SpriterEngine::UniversalObjectInterface* pointObj = combatantObject->getObjectInstance("ability_effect_point");

	abilityEffectPoint = pointObj->getPosition();
}



void Combatant::RenderAbilityEffects()
{
	ReloadAbilityEffectPoint();

	g_pSpritePool->abilityEffectsAnimation->setPosition(SpriterEngine::point(0, 0));
	g_pSpritePool->abilityEffectsAnimation->reprocessCurrentTime();
	SpriterEngine::UniversalObjectInterface* pointObj = g_pSpritePool->abilityEffectsAnimation->getObjectInstance("attaching_point");

	auto pointObjPlayer = g_pSpritePool->abilityEffectsAnimation->objectIfExistsOnCurrentFrame("attaching_point_player");

	if(pointObjPlayer != nullptr && this->IsPlayer())
		g_pSpritePool->abilityEffectsAnimation->setPosition(SpriterEngine::point(abilityEffectPoint.x - pointObjPlayer->getPosition().x, abilityEffectPoint.y - pointObjPlayer->getPosition().y));
	else
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


void Combatant::RenderShadow()
{
	combatantObject->reprocessCurrentTime();
	SpriterEngine::UniversalObjectInterface* pointObj = combatantObject->getObjectInstance("shadow_point");

	ReloadHitbox();

	sf::CircleShape shadow;
	shadow.setRadius(GetRect().width / 2 + 10);
	shadow.setPointCount(60);
	shadow.scale(1.0, 0.2);

	if (IsDying() && combatantObject->getCurrentTime() > 1100.0)
		shadow.setFillColor(sf::Color(0, 0, 0, 100 - (combatantObject->getCurrentTime() - 1100.0) / 9.0));
	else
		shadow.setFillColor(sf::Color(0, 0, 0, 100));

	shadow.setPosition(pointObj->getPosition().x - shadow.getLocalBounds().width / 2, pointObj->getPosition().y- shadow.getGlobalBounds().height / 2);
	engine->GetWindow().draw(shadow);
}



void Combatant::StartTargetsAttackedAnimation()
{
	for (Combatant *c : selectedTargets)
	{
		if (c != this)
		{
			if (dynamic_cast<PlayerMarkus*>(c) != nullptr)
				dynamic_cast<PlayerMarkus*>(c)->AttackedBy(this->battlePosition);

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

	Scale(COMBATANT_ABILITY_SCALE, COMBATANT_ABILITY_SCALE);
	SetPos(lastPosition.x - engine->GetViewPosition().x, 800);
}


void Combatant::ReverseScaleForAbilityAnimation()
{
	Scale(COMBATANT_NORMAL_SCALE, COMBATANT_NORMAL_SCALE);
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


void Combatant::StartDeathAnimation()
{
	SetAnimation("dying", IDLE_ANIMATION_SPEED);
	dying = true;
}


void Combatant::Quit()
{
	SAFE_DELETE(combatantObject);
}

void Combatant::Update()
{
	statusBar.Update(GetRect());
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
	int yPos = statusBar.GetRect().top + statusBar.GetRect().height + 20;
	g_pSpritePool->abilityTargetMarker.SetPos(xPos, yPos);
	g_pSpritePool->abilityTargetMarker.Render(engine->GetWindow());
}

void Combatant::RenderTurnMarker()
{
	int xPos = GetRect().left + (GetRect().width - g_pSpritePool->turnMarker.GetRect().width) / 2;
	int yPos = statusBar.GetRect().top + statusBar.GetRect().height + 20;
	g_pSpritePool->turnMarker.SetPos(xPos, yPos);
	g_pSpritePool->turnMarker.Render(engine->GetWindow());
}



void Combatant::ApplyAbilityEffectToTarget(Combatant * _target, AbilityEffect & _effect)
{
	if (_effect.damageFactor != 0) 
	{
		auto damage = status.GetDamage() * _effect.damageFactor;
		if (rand() % 100 < status.GetCriticalHit())
		{
			_target->Status().LooseHealth(damage * 2, true);
		}
		else
			_target->Status().LooseHealth(damage, false);
	}

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