#pragma once

#include "Player.hpp"
#include "CombatantState.hpp"

class PlayerState : public CombatantState
{
public:

	PlayerState(Player *_context);

protected:
	Player *playerContext;
};