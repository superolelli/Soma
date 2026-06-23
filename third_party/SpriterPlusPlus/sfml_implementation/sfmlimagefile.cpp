#include "sfmlimagefile.h"

#include "../global/settings.h"

#include "../objectinfo/universalobjectinterface.h"

#include <algorithm>

namespace SpriterEngine
{

	SfmlImageFile::SfmlImageFile(std::string initialFilePath, point initialDefaultPivot, sf::RenderTarget *validRenderWindow) :
		ImageFile(initialFilePath,initialDefaultPivot),
		sprite(texture),
		renderWindow(validRenderWindow)
	{
		initializeFile();
	}

	void SfmlImageFile::initializeFile()
	{
		if (texture.loadFromFile(path()))
		{
			texture.setSmooth(true);
			sprite.setTexture(texture, true);
		}
		else
		{
			Settings::Settings::error("SfmlImageFile::initializeFile - sfml texture unable to load file from path \"" + path() + "\"");
		}
	}

	void SfmlImageFile::renderSprite(UniversalObjectInterface * spriteInfo)
	{
		const auto alpha = static_cast<std::uint8_t>(std::clamp(spriteInfo->getAlpha(), 0.0, 1.0) * 255.0);
		sprite.setColor(sf::Color(255, 255, 255, alpha));
		sprite.setPosition({static_cast<float>(spriteInfo->getPosition().x), static_cast<float>(spriteInfo->getPosition().y)});
		sprite.setRotation(sf::degrees(toDegrees(spriteInfo->getAngle())));
		sprite.setScale({static_cast<float>(spriteInfo->getScale().x), static_cast<float>(spriteInfo->getScale().y)});
		sprite.setOrigin({static_cast<float>(spriteInfo->getPivot().x*texture.getSize().x), static_cast<float>(spriteInfo->getPivot().y*texture.getSize().y)});
		renderWindow->draw(sprite);
	}

}
