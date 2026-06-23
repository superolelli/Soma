#include "CombatantStatePrepareAbility.hpp"
#include <numeric>

CombatantStatePrepareAbility::CombatantStatePrepareAbility(Combatant * _context)
	:CombatantState(_context)
{
}


void CombatantStatePrepareAbility::ChooseRandomAlly(bool _canChooseSelf)
{
	int numberOfAllies = std::accumulate((*context->allCombatants).begin(), (*context->allCombatants).end(), 0, [&](int sum, Combatant *c) {if (context->IsAlly(c) && !c->IsDying())return sum + 1; else return sum; });

	if (!_canChooseSelf)
		numberOfAllies -= 1;

	if (numberOfAllies <= 0)
		return;

	int target = rand() % (numberOfAllies);

	if (numberOfAllies == 1)
		target = 0;

	for (Combatant *c : (*context->allCombatants))
	{
		if (context->IsAlly(c) && !c->IsDying())
		{
			if (_canChooseSelf || c != context)
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
