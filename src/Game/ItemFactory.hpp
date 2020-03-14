#pragma once

#include "Lootable.hpp"
#include "Enemy.hpp"

class ItemFactory
{
public:

	static Item CreateItemForLootable(int _level, LootableID _lootableID);
	static Item CreateBattleRewardItem(int _level, Enemy* _enemies[4]);
	static Item CreateShopItem(int _level);

private:
	static ItemID GetRandomItemIDForShop(int _level);
	static ItemID GetRandomItemIDForBattleReward(Enemy* _enemies[4], int _level);
	static ItemID GetRandomItemIDForLootable(LootableID _lootableID, int _level);
	static sf::Color GetRandomColor();
	static int GetRandomItemNumber(ItemID _id, int _level);

	static int GetNumberOfPossibleItemsForLevel(int _level);
	static ItemID GetRandomItemIDForLevel(int _level);
};