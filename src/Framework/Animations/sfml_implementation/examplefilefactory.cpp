#include "examplefilefactory.h"

#include "../override/imagefile.h"
#include "../override/soundfile.h"

// #define __USE_PUGIXML
#ifndef __USE_PUGIXML
#include "tinyxmlspriterfiledocumentwrapper.h"
#else
#include "pugixmlspriterfiledocumentwrapper.h"
#endif

#include "jsonspriterfiledocumentwrapper.h"

#include "sfmlimagefile.h"
#include "sfmlsoundfile.h"

namespace SpriterEngine
{

	ExampleFileFactory::ExampleFileFactory(sf::RenderWindow *validRenderWindow) :
		renderWindow(validRenderWindow)
	{
	}

	ImageFile * ExampleFileFactory::newImageFile(const std::string &initialFilePath, point initialDefaultPivot, atlasdata atlasData)
	{
		return new SfmlImageFile(initialFilePath, initialDefaultPivot, renderWindow);
	}

	SoundFile * ExampleFileFactory::newSoundFile(const std::string & initialFilePath)
	{
		return new SfmlSoundFile(initialFilePath);
	}

	SpriterFileDocumentWrapper * ExampleFileFactory::newScmlDocumentWrapper()
	{
#ifndef __USE_PUGIXML
		return new TinyXmlSpriterFileDocumentWrapper();
#else
		return new PugiXmlSpriterFileDocumentWrapper();		
#endif
	}

	SpriterFileDocumentWrapper *ExampleFileFactory::newSconDocumentWrapper()
	{
		return new JSONSpriterFileDocumentWrapper();
	}
}
