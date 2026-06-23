#ifndef SFMLIMAGEFILE_H
#define SFMLIMAGEFILE_H

#include "../override/imagefile.h"

#include <SFML/Graphics.hpp>

namespace SpriterEngine
{
	class SfmlImageFile : public ImageFile
	{
	public:
		SfmlImageFile(std::string initialFilePath, point initialDefaultPivot, sf::RenderTarget *validRenderWindow);

		void renderSprite(UniversalObjectInterface *spriteInfo) override;

	private:
		sf::Texture texture;
		sf::Sprite sprite;

		sf::RenderTarget *renderWindow;

		void initializeFile();
	};

}

#endif // SFMLIMAGEFILE_H
