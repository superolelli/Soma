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

	bangLevel = 1;
	kutschfahrtLevel = 1;
	tichuLevel = 1;
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

void GameStatus::AddItem(Item _item, bool _triggerCallback)
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
				if (_triggerCallback)
					OnConsumableAddedCallback(c, true);
				return;
			}

		}

		if (consumables.size() < CONSUMABLE_ITEMS_LIMIT)
		{
			consumables.push_back(_item);
			if (_triggerCallback)
				OnConsumableAddedCallback(_item, false);
		}
	}
	else {
		items.push_back(_item);

		if (_triggerCallback)
			OnItemAddedCallback(items.back());
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
			if (it->id == _item.id && it->color == _item.color)
			{
				it->number--;
				if(it->number <= 0 || !_removeOne)
					consumables.erase(it);
				return;
			}
		}
	}
}

void GameStatus::SetOnItemAddedCallback(std::function<void(Item)> _callback)
{
	OnItemAddedCallback = _callback;
}

void GameStatus::SetOnConsumableAddedCallback(std::function<void(Item, bool)> _callback)
{
	OnConsumableAddedCallback = _callback;
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

CombatantStats & GameStatus::GetEquipmentStats(int _player)
{
	return equipmentStats[_player];
}

void GameStatus::SetDiamondStats(int _player, CombatantStats & _stats)
{
	diamondStats[_player] = _stats;
}

CombatantStats & GameStatus::GetDiamondStats(int _player)
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
