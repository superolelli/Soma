#include "Greg.hpp"

GregDigger::GregDigger(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: companionDiedLastRound{false}
	, Enemy(_id, _engine, _notificationRenderer)
{
}
