#include "Combatant.hpp"
#include "Markus.hpp"
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include <numeric>
#include "CombatantStateIdle.hpp"
#include "Battle.hpp"

bool Combatant::setElapsedTimeForAbilityEffect;



Combatant::Combatant(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
{
	engine = _engine;
	notificationRenderer = _notificationRenderer;

	status.Init(this, notificationRenderer);
	status.Reset();

	actsInConfusion = false;
	turnMarkerScale = 1.0;
	battle = nullptr;
}

void Combatant::Init()
{
	Scale(COMBATANT_NORMAL_SCALE, COMBATANT_NORMAL_SCALE);
	combatantObject->reprocessCurrentTime();

	ReloadHitbox();

	statusBar.Init(&status, engine);

	currentState = new CombatantStateIdle(this);
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
	currentState->Update();
}

void Combatant::ChangeState(CombatantState * _state)
{
	SAFE_DELETE(currentState);
	currentState = _state;
}

void Combatant::SetBattlePtr(Battle * _battle)
{
	battle = _battle;
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
	else if(GetID() == CombatantID::BillNoface)
		combatantObject->setScale(SpriterEngine::point(_x + -0.05, _y + -0.05));
	else
		combatantObject->setScale(SpriterEngine::point(_x, _y));
}

sf::Vector2f Combatant::GetLocalPosition() 
{
	return sf::Vector2f(combatantObject->getObjectInstance("bounding_box")->getPosition().x - combatantObject->getPosition().x, combatantObject->getPosition().y);
}

abilityPhase Combatant::GetAbilityStatus()
{
	return currentState->GetStateID();
}

bool Combatant::IsAlly(Combatant *c)
{
	return IsPlayer() == c->IsPlayer();
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
	engine->GetRenderTarget().draw(shadow);
}

void Combatant::RenderAbilityTargetMarker()
{
	int xPos = GetRect().left + (GetRect().width - g_pSpritePool->abilityTargetMarker.GetRect().width) / 2;
	int yPos = statusBar.GetRect().top + statusBar.GetRect().height + 5;
	g_pSpritePool->abilityTargetMarker.SetPos(xPos, yPos);
	g_pSpritePool->abilityTargetMarker.Render(engine->GetRenderTarget());
}

void Combatant::RenderTurnMarker()
{
	g_pSpritePool->turnMarker.SetScale(turnMarkerScale, turnMarkerScale);
	int xPos = GetRect().left + (GetRect().width - g_pSpritePool->turnMarker.GetRect().width) / 2;
	int yPos = statusBar.GetRect().top + statusBar.GetRect().height + 5;
	g_pSpritePool->turnMarker.SetPos(xPos, yPos);
	g_pSpritePool->turnMarker.Render(engine->GetRenderTarget());

	if (turnMarkerScale > 1.0)
	{
		turnMarkerScale -= g_pTimer->GetElapsedTimeAsSeconds() * 5.0 * (TURN_MARKER_ANIMATION_SCALE - 1.0);

		if (turnMarkerScale < 1.0)
			turnMarkerScale = 1.0;
	}
}


void Combatant::ScaleForAbilityAnimation(int _xPos, int _yPos)
{
	lastPosition = combatantObject->getPosition();

	Scale(COMBATANT_ABILITY_SCALE, COMBATANT_ABILITY_SCALE);
	//SetPos(lastPosition.x - engine->GetViewPosition().x, 800);
	SetPos(_xPos, _yPos);
}


void Combatant::ReverseScaleForAbilityAnimation()
{
	Scale(COMBATANT_NORMAL_SCALE, COMBATANT_NORMAL_SCALE);
	SetPos(lastPosition.x, lastPosition.y);
}

void Combatant::SetAnimation(const std::string &_animation, float _speed)
{
	combatantObject->setCurrentAnimation(_animation);
	combatantObject->setPlaybackSpeedRatio(_speed);
	combatantObject->setCurrentTime(0);
}

void Combatant::StartAttackedAnimation(int _xPos, int _yPos)
{
	ScaleForAbilityAnimation(_xPos, _yPos);
	SetAnimation("attacked", ABILITY_ANIMATION_SPEED);
	SetAbilityStatus(attacked);
}

void Combatant::StartDodgingAnimation(int _xPos, int _yPos)
{
	ScaleForAbilityAnimation(_xPos, _yPos);
	SetAnimation("attacked", ABILITY_ANIMATION_SPEED);
	ReloadHitbox();
	notificationRenderer->AddNotification("Ausgewichen!", g_pFonts->f_kingArthur, sf::Vector2f(GetRect().left + GetRect().width / 2.0f, GetRect().top), 1.0f);

	SetAbilityStatus(dodging);
	g_pSounds->PlaySound(DODGED);
}

void Combatant::StartFriendlyAttackedAnimation(int _xPos, int _yPos)
{
	ScaleForAbilityAnimation(_xPos, _yPos);
	SetAnimation("attacked_friendly", ABILITY_ANIMATION_SPEED);
	SetAbilityStatus(attacked);
}


void Combatant::StopAttackedAnimation()
{
	SetAbilityStatus(finished);

	if (status.IsAsleep())
		SetAnimation("sleeping", IDLE_ANIMATION_SPEED);
	else
		SetAnimation("idle", IDLE_ANIMATION_SPEED);

	ReverseScaleForAbilityAnimation();
}


void Combatant::StartDeathAnimation()
{
	SetAnimation("dying", IDLE_ANIMATION_SPEED);
}

void Combatant::RenderAbilityEffect(bool _dodged)
{
	auto abilityEffectPoint = GetAbilityEffectPoint();

	g_pSpritePool->abilityEffectsAnimation->setPosition(SpriterEngine::point(0, 0));
	g_pSpritePool->abilityEffectsAnimation->reprocessCurrentTime();
	SpriterEngine::UniversalObjectInterface* pointObj = g_pSpritePool->abilityEffectsAnimation->getObjectInstance("attaching_point");

	g_pSpritePool->abilityEffectsAnimation->setPosition(SpriterEngine::point(abilityEffectPoint.x - pointObj->getPosition().x, abilityEffectPoint.y - pointObj->getPosition().y));

	if (Combatant::setElapsedTimeForAbilityEffect == false)
	{
		g_pSpritePool->abilityEffectsAnimation->setTimeElapsed(g_pTimer->GetElapsedTimeAsMilliseconds() * ABILITY_EFFECT_ANIMATION_SPEED);
		Combatant::setElapsedTimeForAbilityEffect = true;
	}
	else
		g_pSpritePool->abilityEffectsAnimation->setTimeElapsed(0);

	if (!_dodged)
	{
		g_pSpritePool->abilityEffectsAnimation->render();
		g_pSpritePool->abilityEffectsAnimation->playSoundTriggers();
	}
}

SpriterEngine::point Combatant::GetAbilityEffectPoint()
{
	combatantObject->reprocessCurrentTime();
	SpriterEngine::UniversalObjectInterface* pointObj = combatantObject->getObjectInstance("ability_effect_point");
	return pointObj->getPosition();
}


void Combatant::GiveTurnTo(std::vector<Combatant*>* _targets, BattleGUI *_gui)
{
	allCombatants = _targets;
	gui = _gui;

	statusBar.SetTurnPending(false);

	SetAbilityStatus(handlingStatus);

	if (IsPlayer())
		status.UpdateStatusForNewTurn();
	else
		status.UpdateStatusForNewTurn(1.5);

	turnMarkerScale = TURN_MARKER_ANIMATION_SCALE;
}

void Combatant::SetTurnPending(bool _pending)
{
	statusBar.SetTurnPending(_pending);
}


void Combatant::ApplyAbilityEffect(Combatant * _attacker, AbilityEffect & _effect, float _additionalDamageFactor)
{
	int criticalEffectFactor = 1;
	float criticalDurationFactor = 1;

	bool isCriticalHit = rand() % 100 < _attacker->status.GetCriticalHit() + _effect.criticalHitModificator;
	if (isCriticalHit)
	{
		notificationRenderer->AddNotification("Kritisch!", g_pFonts->f_kingArthur, sf::Vector2f(GetRect().left + GetRect().width / 2.0f, GetRect().top), 1.0f);
		g_pSounds->PlaySound(CRITICAL_HIT);
		criticalEffectFactor = 2;
		criticalDurationFactor = 1.5;
	}

	if (_effect.damageFactor != 0) 
	{
		auto damage = _attacker->status.GetDamage() * (_effect.damageFactor + _additionalDamageFactor);
		Status().LooseHealth(damage * criticalEffectFactor, isCriticalHit);
	}

	if (_effect.heal != 0)
		Status().GainHealth(_effect.heal * criticalEffectFactor, isCriticalHit);

	if (_effect.healSelf != 0)
		_attacker->status.GainHealth(_effect.healSelf);

	if (_effect.confusion != 0)
	{
		if ((rand() % 100) + 1 <= (_effect.confusionProbability * criticalEffectFactor) * 100.0f)
			Status().Confuse(std::round((float)(_effect.confusion) * criticalDurationFactor));
	}

	if (_effect.damageOverTimeRounds != 0)
		Status().DoDamageOverTime(std::round((float)(_effect.damageOverTimeRounds) * criticalDurationFactor), _effect.damageOverTime);

	if (_effect.mark != 0)
		Status().Mark(std::round((float)(_effect.mark) * criticalDurationFactor));

	if (_effect.putToSleepProbability != 0.0f)
	{
		if ((rand() % 100) + 1 <= (_effect.putToSleepProbability * criticalEffectFactor) * 100.0f)
			Status().PutToSleep();
	}

	if (_effect.removeBuffs)
		Status().RemoveAllBuffs();

	if (_effect.removeDebuffs)
		Status().RemoveAllDebuffs();

	if (_effect.buff.duration != 0)
	{
		auto tempBuff = _effect.buff;
		tempBuff.duration = std::round((float)(tempBuff.duration) * criticalDurationFactor);
		if (_effect.buff.isPositive)
			Status().AddBuff(tempBuff);
		else
			Status().AddDebuff(tempBuff);
	}
}