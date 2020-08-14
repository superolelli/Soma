#include "GameStatus.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

void GameStatus::Init()
{
	dice = 0;
	cards = 0;
	items.clear();
	consumables.clear();

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int a = 0; a < 8; a++)
				skillAcquired[i][j][a] = false;

			equipment[i][j].id = ItemID::empty;
		}
		equipmentStats[i].Reset();
		diamondStats[i].Reset();
	}

	levels[0] = 1;
	levels[1] = 1;
	levels[2] = 1;

	for (int i = CONSUMABLE_ITEMS_START; i < ItemID::numberOfItems; i++)
		consumablesAvailability[static_cast<ItemID>(i)] = false;

	//consumablesAvailability[ItemID::beer] = true;

	fatigue = 0;
	maxFatigue = MAX_FATIGUE_VALUE;
}


bool GameStatus::IsSkillAcquired(int player, int ability, int skill)
{
	return skillAcquired[player][ability][skill];
}

void GameStatus::AcquireSkill(int player, int ability, int skill)
{
	if (!skillAcquired[player][ability][skill])
	{
		skillAcquired[player][ability][skill] = true;
		g_pObjectProperties->playerAbilities[player][ability].applySkill(g_pObjectProperties->skills[player][ability][skill]);
	}
}

void GameStatus::AddItem(Item _item)
{
	if (_item.id == ItemID::cards)
	{
		AddCards(_item.number);
		return;
	}

	if (_item.id == ItemID::dice)
	{
		AddDice(_item.number);
		return;
	}

	if (_item.id >= CONSUMABLE_ITEMS_START)
	{
		for (auto &c : consumables)
		{
			if (c.id == _item.id)
			{
				c.number += _item.number;
				Notify(ObserverNotificationGameStatus{ gameStatusEvents::consumableAdded, _item });
				return;
			}
		}

		if (consumables.size() < CONSUMABLE_ITEMS_LIMIT)
		{
			consumables.push_back(_item);
			consumablesAvailability[_item.id] = true;
			Notify(ObserverNotificationGameStatus{ gameStatusEvents::consumableAdded, _item });
		}
	}
	else {
		items.push_back(_item);
		Notify(ObserverNotificationGameStatus{ gameStatusEvents::equipmentAdded, _item });
	}
}


void GameStatus::RemoveItem(Item &_item, bool _removeOne)
{
	if (_item.id < CONSUMABLE_ITEMS_START)
	{
		for (auto it = items.begin(); it != items.end(); it++)
		{
			if (it->id == _item.id && it->color == _item.color)
			{
				items.erase(it);
				return;
			}
		}
	}
	else {
		for (auto it = consumables.begin(); it != consumables.end(); it++)
		{
			if (it->id == _item.id)
			{
				if (_removeOne)
					it->number--;
				else
					it->number -= _item.number;

				if(it->number <= 0)
					consumables.erase(it);
				return;
			}
		}
	}
}

void GameStatus::AddEquipment(int _player, int _slot, Item _item)
{
	equipment[_player][_slot] = _item;
	equipmentStats[_player] += g_pObjectProperties->equipmentStats[_item.id].stats;
}

void GameStatus::RemoveEquipment(int _player, int _slot)
{
	equipmentStats[_player] -= g_pObjectProperties->equipmentStats[equipment[_player][_slot].id].stats;
	equipment[_player][_slot].id = ItemID::empty;
}

CombatantAttributes & GameStatus::GetEquipmentStats(int _player)
{
	return equipmentStats[_player];
}

void GameStatus::SetDiamondStats(int _player, CombatantAttributes & _stats)
{
	diamondStats[_player] = _stats;
}

CombatantAttributes & GameStatus::GetDiamondStats(int _player)
{
	return diamondStats[_player];
}

int GameStatus::GetDiceAmount()
{
	return dice;
}

int GameStatus::GetCardsAmount()
{
	return cards;
}

void GameStatus::AddDice(int _amount)
{
	dice += _amount;
}

void GameStatus::AddCards(int _amount)
{
	cards += _amount;
}

void GameStatus::RemoveDice(int _amount)
{
	dice = std::max(0, dice - _amount);
}

void GameStatus::RemoveCards(int _amount)
{
	cards = std::max(0, cards - _amount);
}

void GameStatus::AddFatigue(int _fatigue)
{
	fatigue = std::min(fatigue + _fatigue, maxFatigue);
}

void GameStatus::RemoveFatigue(int _fatigue)
{
	fatigue = std::max(0, fatigue - _fatigue);
}