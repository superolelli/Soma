#include "PlayerState.hpp"

PlayerState::PlayerState(Player * _context)
	:CombatantState(_context)
{
	playerContext = _context;
}
