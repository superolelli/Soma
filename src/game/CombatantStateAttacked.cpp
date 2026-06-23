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
	CombatantState::Render();
	context->RenderAbilityEffect(dodged);
}