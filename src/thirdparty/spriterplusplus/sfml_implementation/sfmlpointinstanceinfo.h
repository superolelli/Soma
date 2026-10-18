#ifndef SFMLPOINTINSTANCEINFO_H
#define SFMLPOINTINSTANCEINFO_H

#include "SFML/Graphics/RenderWindow.hpp"

#include "SFML/Graphics/CircleShape.hpp"

#include "../objectinfo/pointinstanceinfo.h"

namespace SpriterEngine
{

	class SfmlPointInstanceInfo : public PointInstanceInfo
	{
	public:
		SfmlPointInstanceInfo(sf::RenderTarget *validRenderWindow);

		void render() override;

	private:
		sf::RenderTarget *renderWindow;

		sf::CircleShape circle;
	};

}

#endif // SFMLPOINTINSTANCEINFO_H
