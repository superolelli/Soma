#include "CombatantState.hpp"

CombatantState::CombatantState(Combatant * _context)
{
	context = _context;
}


void CombatantState::Render()
{
	context->combatantObject->setTimeElapsed(g_pTimer->GetElapsedTimeAsMilliseconds());
	context->combatantObject->render();
	context->combatantObject->playSoundTriggers();
}


void CombatantState::RenderStatusBar()
{
	context->statusBar.Render();
}