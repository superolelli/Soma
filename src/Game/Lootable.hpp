#pragma once

#include "../Framework/Graphics/Sprite.hpp"
#include "../Framework/Gameengine.hpp"
#include "DialogManager.hpp"
#include "LootableDialog.hpp"
#include "Resources/SoundManager.hpp"


enum LootableID { barrel, cabinet, chest, cup, keg, rubbish, sack, skeleton, stonepile, table, wagon, beautiful_chest, goblet, secret_bag, brotherhood_cupboard, order_vitrine, tree_stump, numberOfLootables };

static const soundID lootableSounds[numberOfLootables] = {
soundID::BARREL,
soundID::CABINET,
soundID::CHEST,
soundID::CUP,
soundID::KEG,
soundID::RUBBISH,
soundID::SACK,
soundID::SKELETON,
soundID::STONEPILE,
soundID::TABLE,
soundID::WAGON,
soundID::CHEST,
soundID::CUP,
soundID::SACK,
soundID::CABINET,
soundID::TABLE,
soundID::DEAD_TREE
};

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