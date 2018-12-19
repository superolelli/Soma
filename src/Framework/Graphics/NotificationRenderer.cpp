#include "NotificationRenderer.hpp"


void NotificationRenderer::Update()
{
	for (auto it = notifications.begin(); it != notifications.end();)
	{
		it->duration -= (float)(MS_PER_UPDATE) / 1000.0f;
		it->text.move(0.0f, -3.0f);

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

void NotificationRenderer::AddNotification(std::string _notification, sf::Font &_font, sf::Vector2f _position, float _duration)
{
	Notification newNotification;
	newNotification.text.setFont(_font);
	newNotification.text.setString(_notification);
	newNotification.text.setPosition(_position);
	newNotification.text.setFillColor(sf::Color::Black);
	newNotification.text.setOutlineThickness(3);
	newNotification.text.setOutlineColor(sf::Color::White);
	newNotification.duration = _duration;

	notifications.push_back(newNotification);
}
