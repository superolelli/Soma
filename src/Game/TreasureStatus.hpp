#pragma once

#include "Resources\TextureManager.hpp"
#include "Resources\FontManager.hpp"

#include "../Framework/Gameengine.hpp"
#include "../Framework/Graphics/Sprite.hpp"

class TreasureStatus
{
public:

	void Init(CGameEngine *_engine);
	void RenderStatusBar();

	void SetPos(int _x, int _y);
	int GetDiceAmount();
	int GetCardsAmount();
	void AddDice(int _amount);
	void AddCards(int _amount);
	void RemoveDice(int _amount);
	void RemoveCards(int _amount);

private:
	CGameEngine *engine;

	CSprite statusBar;
	sf::Text diceAmountText;
	sf::Text cardsAmountText;

	int dice;
	int cards;

	void PositionComponentsRelativeToPanel();

};