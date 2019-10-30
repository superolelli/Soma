#pragma once

#include "../Framework/Graphics/Sprite.hpp"
#include "../Framework/Gameengine.hpp"
#include "DialogManager.hpp"
#include "LootableDialog.hpp"


enum LootableID { barrel, cabinet, chest, cup, keg, rubbish, sack, skeleton, stonepile, table, wagon, numberOfLootables };

class Lootable
{
public:
	void Init(LootableID _id, DialogManager *_dialogManager, GameStatus *_gameStatus);
	void Render(sf::RenderTarget &_target);
	void Update(CGameEngine *_engine);
	void SetPos(int _x, int _y);

	void AddItem(Item &_item);

private:
	DialogManager *dialogManager;
	GameStatus *gameStatus;

	CSprite sprite;
	bool wasLooted;

	LootableID id;
	sf::IntRect currentBoundingBox;

	std::vector<Item> items;
};