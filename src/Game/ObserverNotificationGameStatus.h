#pragma once

#include "../Framework/Patterns/Observer.hpp"
#include "Item.hpp"

enum class gameStatusEvents { equipmentAdded, consumableAdded };

class ObserverNotificationGameStatus : public ObserverNotification {
public:
	ObserverNotificationGameStatus() {};
	ObserverNotificationGameStatus(gameStatusEvents _event, Item _item) { event = _event; item = _item; }

	gameStatusEvents event;
	Item item;
};