#pragma once

#include "../Framework/Animations/sfml_implementation/exampleobjectfactory.h"
#include "../Framework/Animations/sfml_implementation/examplefilefactory.h"

// include to use Spriter animations
#include "../Framework/Animations/spriterengine.h"
#include "../Framework/Animations/global/settings.h"

#include "../Framework/Gameengine.hpp"

#include "Resources\Resources.hpp"

class Player
{
public:

	void Init();
	void Quit();
	void Update();
	void Render();

	void SetPos(int _x, int _y);

private:

	SpriterEngine::EntityInstance *playerObject;
};