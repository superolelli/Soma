#pragma once

#include "../Gameengine.hpp"

class NotificationRenderer
{
public:

	void Update();
	void Render(sf::RenderTarget &_target);

	void AddNotification(std::string _notification, sf::Font &_font, sf::Vector2f _position, float _duration);

private:

	struct Notification {
		sf::Text text;
		float duration;
	};

	std::vector<Notification> notifications;
};