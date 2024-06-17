#include "Simon.hpp"


PlayerSimon::PlayerSimon(CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Player(CombatantID::Simon, _engine, _notificationRenderer, g_pModels->GetNewCombatantModelInstance(CombatantID::Simon))
{
}
