#include "NotificationRenderer.hpp"
#include <iostream>


void NotificationRenderer::Update()
{
	for (auto it = notifications.begin(); it != notifications.end();)
	{
		it->duration -= (float)(MS_PER_UPDATE) / 1000.0f;
		it->text.move(0.0f, it->up ? -2.0f : 2.0f);

		if (it->duration <= 0.0f)
			it = notifications.erase(it);
		else
			++it;
	}
}


void NotificationRenderer::Render(sf::RenderTarget & _target)
{
	for (const auto &n : notifications)
		_target.draw(n.text);
}

void NotificationRenderer::AddNotification(std::string _notification, sf::Font &_font, sf::Vector2f _position, float _duration, sf::Color _fillColor, sf::Color _outlineColor, int _characterSize, bool up)
{
	Notification newNotification;
	newNotification.text.setFont(_font);
	newNotification.text.setCharacterSize(_characterSize);
	newNotification.text.setString(_notification);
	newNotification.text.setFillColor(_fillColor);
	newNotification.text.setOutlineThickness(3);
	newNotification.text.setOutlineColor(_outlineColor);
	newNotification.text.setPosition(_position - sf::Vector2f(newNotification.text.getLocalBounds().width / 2, 0.0));
	newNotification.duration = _duration;
	newNotification.up = up;

	notifications.push_back(newNotification);
}
