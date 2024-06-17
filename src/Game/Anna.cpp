#include "Anna.hpp"

PlayerAnna::PlayerAnna(CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Player(CombatantID::Anna, _engine, _notificationRenderer, g_pModels->GetNewCombatantModelInstance(CombatantID::Anna))
{
}