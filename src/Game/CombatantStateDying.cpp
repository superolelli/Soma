#include "CombatantStateDying.hpp"

CombatantStateDying::CombatantStateDying(Combatant * _context)
	:CombatantState(_context)
{
	context->StartDeathAnimation();
}

void CombatantStateDying::Update()
{
}

void CombatantStateDying::Render()
{
	context->RenderShadow();

	context->combatantObject->setTimeElapsed(g_pTimer->GetElapsedTimeAsMilliseconds());
	context->combatantObject->render();
	context->combatantObject->playSoundTriggers();
}
