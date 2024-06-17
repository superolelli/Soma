#include "Ole.hpp"


PlayerOle::PlayerOle(CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Player(CombatantID::Ole, _engine, _notificationRenderer, g_pModels->GetNewCombatantModelInstance(CombatantID::Ole))
{
}