#include "CombatantStateIdle.hpp"



CombatantStateIdle::CombatantStateIdle(Combatant *_context)
	:CombatantState(_context)
{
	context->SetAnimation("idle", IDLE_ANIMATION_SPEED);
}

void CombatantStateIdle::Update()
{

}

void CombatantStateIdle::Render()
{
	context->RenderShadow();
	CombatantState::Render();
}
