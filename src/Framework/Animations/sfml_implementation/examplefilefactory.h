#ifndef EXAMPLEFILEFACTORY_H
#define EXAMPLEFILEFACTORY_H

#include "../override/filefactory.h"

#include "SFML/Graphics.hpp"

namespace SpriterEngine
{

	class ExampleFileFactory : public FileFactory
	{
	public:
		ExampleFileFactory(sf::RenderWindow *validRenderWindow);
		
		ImageFile *newImageFile(const std::string &initialFilePath, point initialDefaultPivot, atlasdata atlasData) override;

		SoundFile *newSoundFile(const std::string &initialFilePath) override;

		SpriterFileDocumentWrapper *newScmlDocumentWrapper() override;

		SpriterFileDocumentWrapper *newSconDocumentWrapper() override;

	private:
		sf::RenderWindow *renderWindow;
	};

}

#endif // EXAMPLEFILEFACTORY_H
