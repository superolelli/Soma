#pragma once

#include "../Gameengine.hpp"

class NotificationRenderer
{
public:

	void Update();
	void Render(sf::RenderTarget &_target);

	void AddNotification(std::string _notification, sf::Font &_font, sf::Vector2f _position, float _duration, sf::Color _fillColor = sf::Color::Black, sf::Color _outlineColor = sf::Color::White, int _characterSize = 50, bool up = true);

private:

	struct Notification {
		sf::Text text;
		float duration;
		bool up;
	};

	std::vector<Notification> notifications;
};