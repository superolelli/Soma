#pragma once

#include "Lootable.hpp"
#include "Enemy.hpp"


class ItemFactory
{
public:

	static Item CreateItemForLootable(LevelType _levelType, int _level, LootableID _lootableID);
	static Item CreateBattleRewardItem(int _level, std::array<Enemy*, 4> &_enemies);
	static Item CreateShopItem(int _bangLevel, int _kutschfahrtLevel);

private:
	static ItemID GetRandomItemIDForShop(int _bangLevel, int _kutschfahrtLevel);
	static ItemID GetRandomItemIDForBattleReward(std::array<Enemy*, 4> &_enemies, int _level);
	static ItemID GetRandomItemIDForLootable(LootableID _lootableID, LevelType _levelType, int _level);
	static sf::Color GetRandomColor();
	static int GetRandomItemNumber(ItemID _id, int _level);

	static int GetNumberOfPossibleItemsForLevel(LevelType _levelType, int _level);
	static ItemID GetRandomItemIDForLevel(LevelType _levelType, int _level);
};