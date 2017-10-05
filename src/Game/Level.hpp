#pragma once

#include "../Framework/Graphics/Sprite.hpp"
#include "../Game/Resources/Resources.hpp"



class Level
{
public:

	void Init();
	void Update(int _playerPos);
	void Render(sf::RenderTarget &_target, int _viewX);

	bool InBattle() { return battle; }

private:

	std::vector<CSprite> backgrounds;

	int battles[14];

	void RenderBackground(sf::RenderTarget &_target, int _viewX);

	bool battle;
};