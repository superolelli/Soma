#ifndef SFMLBONEINSTANCEINFO_H
#define SFMLBONEINSTANCEINFO_H

#include "SFML/Graphics/RenderWindow.hpp"

#include "SFML/Graphics/ConvexShape.hpp"

#include "../objectinfo/boneinstanceinfo.h"

namespace SpriterEngine
{

	class SfmlBoneInstanceInfo : public BoneInstanceInfo
	{
	public:
		SfmlBoneInstanceInfo(point initialSize, sf::RenderTarget *validRenderWindow);

		void render() override;

	private:
		sf::RenderTarget *renderWindow;

		sf::ConvexShape boneShape;
	};

}

#endif // SFMLBONEINSTANCEINFO_H
