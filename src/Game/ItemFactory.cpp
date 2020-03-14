#include "ItemFactory.hpp"



Item ItemFactory::CreateItemForLootable(int _level, LootableID _lootableID)
{
	Item item;
	item.color = GetRandomColor();
	item.id = GetRandomItemIDForLootable(_lootableID, _level);
	item.number = GetRandomItemNumber(item.id, _level);
	return item;
}

Item ItemFactory::CreateBattleRewardItem(int _level, Enemy* _enemies[4])
{
	Item item;
	item.color = GetRandomColor();
	item.id = GetRandomItemIDForBattleReward(_enemies, _level);
	item.number = GetRandomItemNumber(item.id, _level);
	return item;
}


Item ItemFactory::CreateShopItem(int _level)
{
	Item item;
	item.color = GetRandomColor();
	item.id = GetRandomItemIDForShop(_level);
	item.number = 1;
	return item;
}


ItemID ItemFactory::GetRandomItemIDForShop(int _level)
{
	ItemID itemID;
	do {
		itemID = GetRandomItemIDForLevel(_level);
	} while (itemID <= ItemID::dice || itemID >= CONSUMABLE_ITEMS_START);

	return itemID;
}


ItemID ItemFactory::GetRandomItemIDForBattleReward(Enemy* _enemies[4], int _level)
{
	std::vector<ItemID> possibleItems;

	for (int i = 0; i < 4; i++)
	{
		if (_enemies[i] != nullptr)
		{
			for (auto &l : g_pObjectProperties->enemyLoot[_enemies[i]->GetID()])
			{
				if (g_pObjectProperties->getItemStats(l).level <= _level && std::find(possibleItems.begin(), possibleItems.end(), l) == possibleItems.end())
					possibleItems.push_back(l);
			}
		}
	}
	possibleItems.push_back(ItemID::cards);

	return possibleItems[rand() % possibleItems.size()];
}


ItemID ItemFactory::GetRandomItemIDForLootable(LootableID _lootableID, int _level)
{
	if (!g_pObjectProperties->lootableProperties[_lootableID].possibleItems.empty())
	{
		std::vector<ItemID> possibleItems;

		for (auto &i : g_pObjectProperties->lootableProperties[_lootableID].possibleItems)
		{
			if (g_pObjectProperties->getItemStats(i).level <= _level)
				possibleItems.push_back(i);
		}

		return possibleItems[rand() % possibleItems.size()];
	}
	else
		return GetRandomItemIDForLevel(_level);
}




sf::Color ItemFactory::GetRandomColor()
{
	int randomNumber = rand() % 4;

	if (randomNumber == 0)
		return sf::Color(128, 0, 0);
	else if (randomNumber == 1)
		return sf::Color(0, 128, 0);
	else if (randomNumber == 2)
		return sf::Color(0, 0, 128);
	else
		return sf::Color(128, 128, 0);
}

int ItemFactory::GetRandomItemNumber(ItemID _id, int _level)
{
	if (_id == ItemID::cards)
		return rand() % 4 * std::ceil((float)_level / 2.0) + 1;
	else if (_id == ItemID::dice)
		return rand() % 2 * std::ceil((float)_level / 2.0) + 1;
	else if (_id >= CONSUMABLE_ITEMS_START)
		return rand() % 3 + 1;

	return 1;
}


int ItemFactory::GetNumberOfPossibleItemsForLevel(int _level)
{
	int numberOfPossibleItems = 0;
	for (int i = 0; i < _level; i++)
		numberOfPossibleItems += g_pObjectProperties->itemsByLevel[i].size();
	return numberOfPossibleItems;
}


ItemID ItemFactory::GetRandomItemIDForLevel(int _level)
{
	int itemID = rand() % GetNumberOfPossibleItemsForLevel(_level);;

	int i;
	for (i = 0; i < _level; i++)
	{
		if (itemID >= g_pObjectProperties->itemsByLevel[i].size())
			itemID -= g_pObjectProperties->itemsByLevel[i].size();
		else
			break;
	}

	return g_pObjectProperties->itemsByLevel[i][itemID];
}
