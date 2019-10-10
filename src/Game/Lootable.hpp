#pragma once

#include "../Framework/Graphics/Sprite.hpp"
#include "../Framework/Gameengine.hpp"


enum LootableID { barrel, cabinet, chest, cup, keg, rubbish, sack, skeleton, stonepile, table, wagon, numberOfLootables };

class Lootable
{
public:
	void Init(LootableID _id);
	void Render(sf::RenderTarget &_target);
	void Update(CGameEngine *_engine);
	void SetPos(int _x, int _y);

private:
	CSprite sprite;
	bool wasLooted;

	LootableID id;
	sf::IntRect currentBoundingBox;

};