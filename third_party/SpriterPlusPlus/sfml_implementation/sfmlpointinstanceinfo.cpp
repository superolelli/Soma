#include "sfmlpointinstanceinfo.h"

#include "../global/settings.h"

namespace SpriterEngine
{

	SfmlPointInstanceInfo::SfmlPointInstanceInfo(sf::RenderTarget *validRenderWindow) :
		renderWindow(validRenderWindow),
		circle(10)
	{
		circle.setFillColor(sf::Color::Red);
		circle.setOrigin({5.f, 5.f});
	}

	void SfmlPointInstanceInfo::render()
	{
		if (Settings::renderDebugPoints)
		{
			circle.setPosition({static_cast<float>(getPosition().x), static_cast<float>(getPosition().y)});
			circle.setRotation(sf::degrees(toDegrees(getAngle())));
			renderWindow->draw(circle);
		}
	}

}
