#ifndef SFMLIMAGEFILE_H
#define SFMLIMAGEFILE_H

#include "../override/imagefile.h"

#include <SFML/Graphics.hpp>

namespace SpriterEngine
{
	struct SfmlRenderSettings
	{
		sf::Color color{sf::Color::White};
		sf::Vector2f offset{};
		const sf::Shader *shader{};
	};

	class SfmlImageFile : public ImageFile
	{
	public:
		SfmlImageFile(std::string initialFilePath,
		              point initialDefaultPivot,
		              sf::RenderTarget *validRenderWindow,
		              const SfmlRenderSettings *renderSettings = nullptr);

		void renderSprite(UniversalObjectInterface *spriteInfo) override;

	private:
		sf::Texture texture;
		sf::Sprite sprite;

		sf::RenderTarget *renderWindow;
		const SfmlRenderSettings *renderSettings;

		void initializeFile();
	};

}

#endif // SFMLIMAGEFILE_H
