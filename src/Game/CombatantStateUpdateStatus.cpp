#include "CombatantStateUpdateStatus.hpp"
#include "CombatantStateIdle.hpp"
#include "EnemyStatePrepareAbility.hpp"
#include "PlayerStatePrepareAbility.hpp"

CombatantStateUpdateStatus::CombatantStateUpdateStatus(Combatant *_context)
	:CombatantState(_context) {}

void CombatantStateUpdateStatus::Update()
{
	//Reset variables (should probably happen when GiveTurnTo() is called
	context->actsInConfusion = false;
	context->selectedTargets.clear();

	context->Status().ExecuteStatusChanges();
	if (!context->Status().IsExecutingStatusChanges())
	{
		if (context->Status().SkipRound())
		{
			CombatantStateIdle *newState = new CombatantStateIdle(context);
			context->ChangeState(newState);
			return;
		}
		else
		{
			context->HandleConfusion();

			if (context->IsPlayer())
			{
				PlayerStatePrepareAbility *newState = new PlayerStatePrepareAbility(dynamic_cast<Player*>(context));
				context->ChangeState(newState);
			}
			else 
			{
				EnemyStatePrepareAbility *newState = new EnemyStatePrepareAbility(dynamic_cast<Enemy*>(context));
				context->ChangeState(newState);
			}
		}

	}
}

void CombatantStateUpdateStatus::Render()
{
	context->RenderShadow();

	context->combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	context->combatantObject->render();
	context->combatantObject->playSoundTriggers();

	context->RenderTurnMarker();

	context->statusBar.Render();
}
