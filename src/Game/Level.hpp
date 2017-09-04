#pragma once

#include "../Framework/Graphics/Sprite.hpp"
#include "../Game/Resources/Resources.hpp"



class Level
{
public:

	void Init();
	void Update();
	void Render(sf::RenderTarget &_target, int _viewX);


private:

	std::vector<CSprite> backgrounds;
};