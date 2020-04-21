#include "CombatantStateAttacked.hpp"

CombatantStateAttacked::CombatantStateAttacked(Combatant *_context, bool _dodged)
	:CombatantState(_context)
{
	dodged = _dodged;
}

abilityPhase CombatantStateAttacked::GetStateID()
{
	if (dodged)
		return dodging;
	else
		return attacked;
}


void CombatantStateAttacked::Update()
{
}

void CombatantStateAttacked::Render()
{
	context->combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	context->combatantObject->render();
	context->combatantObject->playSoundTriggers();

	RenderAbilityEffects();
}


void CombatantStateAttacked::RenderAbilityEffects()
{
	ReloadAbilityEffectPoint();

	g_pSpritePool->abilityEffectsAnimation->setPosition(SpriterEngine::point(0, 0));
	g_pSpritePool->abilityEffectsAnimation->reprocessCurrentTime();
	SpriterEngine::UniversalObjectInterface* pointObj = g_pSpritePool->abilityEffectsAnimation->getObjectInstance("attaching_point");

	//auto pointObjPlayer = g_pSpritePool->abilityEffectsAnimation->objectIfExistsOnCurrentFrame("attaching_point_player");

	//if (pointObjPlayer != nullptr && context->IsPlayer())
	//	g_pSpritePool->abilityEffectsAnimation->setPosition(SpriterEngine::point(abilityEffectPoint.x - pointObjPlayer->getPosition().x, abilityEffectPoint.y - pointObjPlayer->getPosition().y));
	//else
		g_pSpritePool->abilityEffectsAnimation->setPosition(SpriterEngine::point(abilityEffectPoint.x - pointObj->getPosition().x, abilityEffectPoint.y - pointObj->getPosition().y));


	if (Combatant::setElapsedTimeForAbilityEffect == false)
	{
		g_pSpritePool->abilityEffectsAnimation->setTimeElapsed((float)g_pTimer->GetElapsedTime().asMilliseconds() * ABILITY_EFFECT_ANIMATION_SPEED);
		Combatant::setElapsedTimeForAbilityEffect = true;
	}
	else
		g_pSpritePool->abilityEffectsAnimation->setTimeElapsed(0);

	if (!dodged)
	{
		g_pSpritePool->abilityEffectsAnimation->render();
		g_pSpritePool->abilityEffectsAnimation->playSoundTriggers();
	}
}


void CombatantStateAttacked::ReloadAbilityEffectPoint()
{
	context->combatantObject->reprocessCurrentTime();
	SpriterEngine::UniversalObjectInterface* pointObj = context->combatantObject->getObjectInstance("ability_effect_point");
	abilityEffectPoint = pointObj->getPosition();
}