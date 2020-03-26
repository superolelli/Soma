#include "CombatantStateDodging.hpp"

CombatantStateDodging::CombatantStateDodging(Combatant *_context)
	:CombatantState(_context) {}

void CombatantStateDodging::Update()
{
}

void CombatantStateDodging::Render()
{
	context->combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	context->combatantObject->render();
	context->combatantObject->playSoundTriggers();

	context->RenderAbilityEffects();
}
