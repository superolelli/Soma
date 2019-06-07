#pragma once

#include "../Framework/Patterns/Observer.hpp"
#include "../Game/CombatantID.hpp"

enum class battleEvents { enemyDied };

class ObserverNotificationBattle : public ObserverNotification {
public:
	ObserverNotificationBattle() {};
	ObserverNotificationBattle(battleEvents _event) { event = _event; }

	battleEvents event;
};