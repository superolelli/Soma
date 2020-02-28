#include "LootableFactory.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

void LootableFactory::Init(DialogManager * _dialogManager, GameStatus * _gameStatus)
{
	dialogManager = _dialogManager;
	gameStatus = _gameStatus;

	numberOfPossibleItems = 0;
}

void LootableFactory::SetLevel(int _level)
{
	level = _level;
}


Lootable * LootableFactory::CreateLootable(int _roomX, int _roomY)
{
	Lootable *lootable = new Lootable;
	LootableID lootableId = LootableID(rand() % numberOfLootables);

	lootable->Init(lootableId, dialogManager, gameStatus);

	numberOfPossibleItems = 0;
	for (int i = 0; i < level; i++)
		numberOfPossibleItems += g_pObjectProperties->itemsByLevel[i].size();

	for(int i = 0; i < 9; i++)
	{
		Item item;
		item.color = GetRandomColor();
		item.id = GetRandomItemID(lootableId);
		item.number = GetRandomItemNumber(item.id);
		lootable->AddItem(item);

		if (rand() % 4 != 0)
			break;
	}


	lootable->SetPos(_roomX + g_pObjectProperties->lootableProperties[lootableId].position.x, _roomY + g_pObjectProperties->lootableProperties[lootableId].position.y);

	return lootable;
}



ItemID LootableFactory::GetRandomItemID(LootableID _lootableID)
{
	if (!g_pObjectProperties->lootableProperties[_lootableID].possibleItems.empty())
	{
		int itemID;
		do {
			itemID = rand() % g_pObjectProperties->lootableProperties[_lootableID].possibleItems.size();
			itemID = g_pObjectProperties->lootableProperties[_lootableID].possibleItems[itemID];
		} while (g_pObjectProperties->getItemStats(ItemID(itemID)).level > level);

		return ItemID(itemID);
	}
	else
	{
		int itemID = rand() % numberOfPossibleItems;

		int i;
		for (i = 0; i < level; i++)
		{
			if (itemID >= g_pObjectProperties->itemsByLevel[i].size())
				itemID -= g_pObjectProperties->itemsByLevel[i].size();
			else
				break;
		}

		return g_pObjectProperties->itemsByLevel[i][itemID];
	}
}


sf::Color LootableFactory::GetRandomColor()
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


int LootableFactory::GetRandomItemNumber(ItemID _id)
{
	if (_id == ItemID::cards)
		return rand() % 4 * std::ceil((float)level / 2.0) + 1;
	else if (_id == ItemID::dice)
		return rand() % 2 * std::ceil((float)level / 2.0) + 1;
	else if (_id >= CONSUMABLE_ITEMS_START)
		return rand() % 3 + 1;

	return 1;
}