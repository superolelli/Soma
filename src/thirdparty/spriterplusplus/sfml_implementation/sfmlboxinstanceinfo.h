#ifndef SFMLBOXINSTANCEINFO_H
#define SFMLBOXINSTANCEINFO_H

#include "SFML/Graphics/RenderWindow.hpp"

#include "SFML/Graphics/RectangleShape.hpp"

#include "../objectinfo/boxinstanceinfo.h"

namespace SpriterEngine
{

	class SfmlBoxInstanceInfo : public BoxInstanceInfo
	{
	public:
		SfmlBoxInstanceInfo(point initialSize, sf::RenderTarget *validRenderWindow);

		void render() override;

	private:
		sf::RenderTarget *renderWindow;

		sf::RectangleShape rectangle;
	};

}

#endif // SFMLBOXINSTANCEINFO_H
