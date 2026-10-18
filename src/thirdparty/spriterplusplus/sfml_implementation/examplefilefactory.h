#ifndef EXAMPLEFILEFACTORY_H
#define EXAMPLEFILEFACTORY_H

#include "../override/filefactory.h"

#include "SFML/Graphics.hpp"

namespace SpriterEngine
{
	struct SfmlRenderSettings;

	class ExampleFileFactory : public FileFactory
	{
	public:
		ExampleFileFactory(sf::RenderTarget *validRenderWindow, const SfmlRenderSettings *renderSettings = nullptr);
		
		ImageFile *newImageFile(const std::string &initialFilePath, point initialDefaultPivot, atlasdata atlasData) override;

		SoundFile *newSoundFile(const std::string &initialFilePath) override;

		SpriterFileDocumentWrapper *newScmlDocumentWrapper() override;

		SpriterFileDocumentWrapper *newSconDocumentWrapper() override;

	private:
		sf::RenderTarget *renderWindow;
		const SfmlRenderSettings *renderSettings;
	};

}

#endif // EXAMPLEFILEFACTORY_H
