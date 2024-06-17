#pragma once

#include "../Framework/Graphics/Sprite.hpp"
#include "../Framework/Gameengine.hpp"

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
	sf::Text diceAmountText;
	sf::Text cardsAmountText;

	void PositionComponentsRelativeToPanel();
};