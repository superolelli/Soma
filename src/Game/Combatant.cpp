#include "Combatant.hpp"
#include "Markus.hpp"
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include <numeric>

bool Combatant::setElapsedTimeForAbilityEffect;



Combatant::Combatant(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
{
	engine = _engine;
	notificationRenderer = _notificationRenderer;

	status.Init(this, notificationRenderer);
	status.Reset();

	actsInConfusion = false;
	dying = false;

	turnMarkerScale = 1.0f;

	abilityStatus = finished;
}

void Combatant::Init()
{
	SetAnimation("idle", IDLE_ANIMATION_SPEED);
	Scale(COMBATANT_NORMAL_SCALE, COMBATANT_NORMAL_SCALE);
	combatantObject->reprocessCurrentTime();

	ReloadHitbox();

	statusBar.Init(&status, engine);
}

void Combatant::SetPos(int _x, int _y)
{
	combatantObject->setPosition(SpriterEngine::point(_x, _y));
}


void Combatant::Scale(float _x, float _y)
{
	if (GetID() == CombatantID::Indianer)
		combatantObject->setScale(SpriterEngine::point(_x + 0.1, _y + 0.1));
	else if (GetID() == CombatantID::Hilfssheriff)
		combatantObject->setScale(SpriterEngine::point(_x + 0.05, _y + 0.05));
	else if (GetID() == CombatantID::Greg)
		combatantObject->setScale(SpriterEngine::point(_x + 0.1, _y + 0.1));
	else if (GetID() == CombatantID::Apachekid)
		combatantObject->setScale(SpriterEngine::point(_x + 0.15, _y + 0.15));
	else if(GetID() == CombatantID::BigSpencer)
		combatantObject->setScale(SpriterEngine::point(_x + -0.02, _y + -0.02));
	else
		combatantObject->setScale(SpriterEngine::point(_x, _y));
}

sf::Vector2f Combatant::GetLocalPosition() 
{
	return sf::Vector2f(combatantObject->getObjectInstance("bounding_box")->getPosition().x - combatantObject->getPosition().x, combatantObject->getPosition().y);
}


void Combatant::ReloadHitbox()
{
	SpriterEngine::UniversalObjectInterface* hitboxObj;
	if (hitboxObj = combatantObject->objectIfExistsOnCurrentFrame("bounding_box"))
	{
		hitbox.left = hitboxObj->getPosition().x;
		hitbox.top = hitboxObj->getPosition().y;
		hitbox.width = hitboxObj->getSize().x  * hitboxObj->getScale().x;
		hitbox.height = hitboxObj->getSize().y * hitboxObj->getScale().y;
	}
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



void Combatant::StartTargetsAttackedAnimation(int _abilityPrecision)
{
	for (Combatant *c : selectedTargets)
	{
		if (c != this)
		{
			if (dynamic_cast<PlayerMarkus*>(c) != nullptr)
				dynamic_cast<PlayerMarkus*>(c)->AttackedBy(this->battlePosition);

			if ((c->IsPlayer() ^ this->IsPlayer()) || actsInConfusion) //^ = XOR
			{
				if (c->CheckForDodging(this, _abilityPrecision))
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

	abilityStatus = handlingStatus;

	if (IsPlayer())
		status.UpdateStatusForNewTurn();
	else
		status.UpdateStatusForNewTurn(1.5f);

	turnMarkerScale = TURN_MARKER_ANIMATION_SCALE;
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
	g_pSounds->PlaySound(DODGED);
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
	combatantObject->reprocessCurrentTime();
	ReloadHitbox();
	statusBar.Update(GetRect());
}


bool Combatant::AbilityEffectIsPlaying()
{
	if (g_pSpritePool->abilityEffectsAnimation->currentAnimationName() == "empty")
		return false;
	else
		return g_pSpritePool->abilityEffectsAnimation->animationIsPlaying();
}


bool Combatant::CheckForDodging(Combatant *_attacker, int _precisionModificator)
{
	int difference = status.GetDodge() - (_attacker->Status().GetPrecision() + _precisionModificator);
	if (rand()%100 < difference * 2)
		return true;

	return false;
}

bool Combatant::IsAlly(Combatant *c)
{
	return IsPlayer() == c->IsPlayer();
}


void Combatant::HandleConfusion()
{
	if (status.IsConfused())
	{
		if (rand() % 10 < 5)
		{
			notificationRenderer->AddNotification("Verwirrt!", g_pFonts->f_kingArthur, sf::Vector2f(GetRect().left - GetRect().width / 2.0f, GetRect().top - 20.0f), 1.0f);
			actsInConfusion = true;
		}
	}
}

void Combatant::ChooseRandomAlly()
{
	int numberOfAllies = std::accumulate((*allCombatants).begin(), (*allCombatants).end(), 0, [&](int sum, Combatant *c) {if (IsAlly(c) && !c->IsDying())return sum + 1; else return sum; });

	if (numberOfAllies <= 1)
		return;

	int target = rand() % (numberOfAllies - 1);

	if (numberOfAllies == 2)
		target = 0;

	for (Combatant *c : (*allCombatants))
	{
		if (IsAlly(c) && c != this && !c->IsDying())
		{
			if (target == 0)
			{
				selectedTargets.push_back(c);
				return;
			}
			target--;
		}
	}
}

void Combatant::ChooseRandomOpponent()
{
	int numberOfOpponents = std::accumulate((*allCombatants).begin(), (*allCombatants).end(), 0, [&](int sum, Combatant* c) {if (!IsAlly(c) && !c->IsDying()) return sum + 1; else return sum; });

	if (numberOfOpponents < 1)
		return;

	int target = rand() % numberOfOpponents;
	for (Combatant* c : (*allCombatants))
	{
		if (!IsAlly(c) && !c->IsDying())
		{
			if (target == 0)
			{
				selectedTargets.push_back(c);
				return;
			}
			target--;
		}
	}
}


void Combatant::RenderAbilityTargetMarker()
{
	int xPos = GetRect().left + (GetRect().width - g_pSpritePool->abilityTargetMarker.GetRect().width) / 2;
	int yPos = statusBar.GetRect().top + statusBar.GetRect().height + 5;
	g_pSpritePool->abilityTargetMarker.SetPos(xPos, yPos);
	g_pSpritePool->abilityTargetMarker.Render(engine->GetWindow());
}

void Combatant::RenderTurnMarker()
{
	g_pSpritePool->turnMarker.SetScale(turnMarkerScale, turnMarkerScale);
	int xPos = GetRect().left + (GetRect().width - g_pSpritePool->turnMarker.GetRect().width) / 2;
	int yPos = statusBar.GetRect().top + statusBar.GetRect().height + 5;
	g_pSpritePool->turnMarker.SetPos(xPos, yPos);
	g_pSpritePool->turnMarker.Render(engine->GetWindow());

	if (turnMarkerScale > 1.0)
	{
		turnMarkerScale -= g_pTimer->GetElapsedTime().asSeconds() * 5.0f * (TURN_MARKER_ANIMATION_SCALE - 1.0f);

		if (turnMarkerScale < 1.0)
			turnMarkerScale = 1.0;
	}
}



void Combatant::ApplyAbilityEffectToTarget(Combatant * _target, AbilityEffect & _effect, float _additionalDamageFactor)
{
	if (_effect.damageFactor != 0) 
	{
		auto damage = status.GetDamage() * (_effect.damageFactor + _additionalDamageFactor);
		if (rand() % 100 < status.GetCriticalHit() + _effect.criticalHitModificator)
		{
			g_pSounds->PlaySound(CRITICAL_HIT);
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

	if (_effect.damageOverTimeRounds != 0)
		_target->Status().DoDamageOverTime(_effect.damageOverTimeRounds, _effect.damageOverTime);

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