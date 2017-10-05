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

	void Init(int _id);
	void Quit();
	void Update(int _xMove, bool _is_walking);
	void Render();

	void SetPos(int _x, int _y);

private:

	SpriterEngine::EntityInstance *playerObject;
	bool is_walking;

};