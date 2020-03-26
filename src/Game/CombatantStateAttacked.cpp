#include "CombatantStateAttacked.hpp"

CombatantStateAttacked::CombatantStateAttacked(Combatant *_context)
	:CombatantState(_context){}


void CombatantStateAttacked::Update()
{
}

void CombatantStateAttacked::Render()
{
	context->combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	context->combatantObject->render();
	context->combatantObject->playSoundTriggers();

	context->RenderAbilityEffects();
}
