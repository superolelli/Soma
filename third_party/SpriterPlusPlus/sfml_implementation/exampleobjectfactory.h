#ifndef EXAMPLEOBJECTFACTORY_H
#define EXAMPLEOBJECTFACTORY_H

#include "SFML/Graphics/RenderWindow.hpp"

#include "../override/objectfactory.h"

namespace SpriterEngine
{

	class ExampleObjectFactory : public ObjectFactory
	{
	public:
		ExampleObjectFactory(sf::RenderTarget *validRenderWindow);
		
		PointInstanceInfo *newPointInstanceInfo() override;

		BoxInstanceInfo *newBoxInstanceInfo(point size) override;

		BoneInstanceInfo *newBoneInstanceInfo(point size) override;

	private:
		sf::RenderTarget *renderWindow;
	};

}

#endif // EXAMPLEOBJECTFACTORY_H
