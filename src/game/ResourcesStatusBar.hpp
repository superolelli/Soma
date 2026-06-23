#pragma once

#include "../engine/Graphics/Sprite.hpp"
#include "../engine/Gameengine.hpp"

class ResourcesStatusBar
{
public:

	ResourcesStatusBar(CGameEngine *_engine);
	void Update(int _cards, int _dice);
	void Render();

	void SetPos(int _x, int _y);

private:

	CGameEngine *engine;

	CSprite statusBar;
	sf::Text diceAmountText{SfmlCompat::defaultFont()};
	sf::Text cardsAmountText{SfmlCompat::defaultFont()};

	void PositionComponentsRelativeToPanel();
};