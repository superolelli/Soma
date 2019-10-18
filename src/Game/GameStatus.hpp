#pragma once

#include "Resources\TextureManager.hpp"
#include "Resources\FontManager.hpp"

#include "../Framework/Gameengine.hpp"
#include "../Framework/Graphics/Sprite.hpp"

#include <functional>

enum ItemID {iron_plate, sombrero};

struct Item
{
	ItemID id;
	sf::Color color;
};

class GameStatus
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

	bool IsSkillAcquired(int player, int ability, int skill);
	void AcquireSkill(int player, int ability, int skill);

	void AddItem(Item &&_item);
	const std::vector<Item> &GetItems() { return items; }
	void SetOnItemAddedCallback(std::function<void(Item)> _callback);

	int bangLevel;
	int kutschfahrtLevel;
	int tichuLevel;

private:
	CGameEngine *engine;

	CSprite statusBar;
	sf::Text diceAmountText;
	sf::Text cardsAmountText;

	int dice;
	int cards;

	bool skillAcquired[4][4][8];

	std::vector<Item> items;
	std::function<void(Item)> OnItemAddedCallback;

	void PositionComponentsRelativeToPanel();
};