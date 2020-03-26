#include "EnemyState.hpp"

EnemyState::EnemyState(Enemy * _context)
	:CombatantState(_context)
{
	enemyContext = _context;
}
