#include "CombatantStatePrepareAbility.hpp"
#include <numeric>

CombatantStatePrepareAbility::CombatantStatePrepareAbility(Combatant * _context)
	:CombatantState(_context)
{
}


void CombatantStatePrepareAbility::ChooseRandomAlly()
{
	int numberOfAllies = std::accumulate((*context->allCombatants).begin(), (*context->allCombatants).end(), 0, [&](int sum, Combatant *c) {if (context->IsAlly(c) && !c->IsDying())return sum + 1; else return sum; });

	if (numberOfAllies <= 1)
		return;

	int target = rand() % (numberOfAllies - 1);

	if (numberOfAllies == 2)
		target = 0;

	for (Combatant *c : (*context->allCombatants))
	{
		if (context->IsAlly(c) && c != context && !c->IsDying())
		{
			if (target == 0)
			{
				context->selectedTargets.push_back(c);
				return;
			}
			target--;
		}
	}
}

void CombatantStatePrepareAbility::ChooseRandomOpponent()
{
	int numberOfOpponents = std::accumulate((*context->allCombatants).begin(), (*context->allCombatants).end(), 0, [&](int sum, Combatant* c) {if (!context->IsAlly(c) && !c->IsDying()) return sum + 1; else return sum; });

	if (numberOfOpponents < 1)
		return;

	int target = rand() % numberOfOpponents;
	for (Combatant* c : (*context->allCombatants))
	{
		if (!context->IsAlly(c) && !c->IsDying())
		{
			if (target == 0)
			{
				context->selectedTargets.push_back(c);
				return;
			}
			target--;
		}
	}
}