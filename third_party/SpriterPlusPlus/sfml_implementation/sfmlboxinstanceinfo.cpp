#include "sfmlboxinstanceinfo.h"

#include "../global/settings.h"

namespace SpriterEngine
{

	SfmlBoxInstanceInfo::SfmlBoxInstanceInfo(point initialSize, sf::RenderTarget *validRenderWindow) :
		BoxInstanceInfo(initialSize),
		renderWindow(validRenderWindow),
		rectangle(sf::Vector2f(initialSize.x, initialSize.y))
	{
	}

	void SfmlBoxInstanceInfo::render()
	{
		if (Settings::renderDebugBoxes)
		{
			rectangle.setPosition({static_cast<float>(getPosition().x), static_cast<float>(getPosition().y)});
			rectangle.setRotation(sf::degrees(toDegrees(getAngle())));
			rectangle.setScale({static_cast<float>(getScale().x), static_cast<float>(getScale().y)});
			rectangle.setOrigin({static_cast<float>(getPivot().x*getSize().x), static_cast<float>(getPivot().y*getSize().y)});
			renderWindow->draw(rectangle);
		}
	}

}
