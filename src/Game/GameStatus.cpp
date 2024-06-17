#include "GameStatus.hpp"
#include "Resources\ObjectPropertiesManager.hpp"
#include <fstream>

GameStatus::GameStatus()
{
	//Reset("");
}

void GameStatus::Reset(const std::string &filepath)
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
		equipmentStats[i].stats.Reset();
		equipmentStats[i].missOnHighDamage = false;
		diamondStats[i].Reset();
	}

	levels[0] = 1;
	levels[1] = 1;
	levels[2] = 1;

	for (int i = CONSUMABLE_ITEMS_START; i < ItemID::numberOfItems; i++)
		consumablesAvailability[static_cast<ItemID>(i)] = false;

	savegamePath = filepath;
}

void GameStatus::LoadFromFile(const std::string &filepath)
{
	Reset(filepath);
	std::ifstream input(filepath);

	input >> cards;
	input >> dice;
	input >> levels[0];
	input >> levels[1];
	input >> levels[2];

	bool skill;
	for (int p = 0; p < 4; p++) {
		for (int a = 0; a < 4; a++) {
			for (int s = 0; s < 8; s++) {
				input >> skill;
				if (skill == true)
					AcquireSkill(p, a, s);
			}
		}
	}

	int currentID, r, g, b;
	input >> currentID;
	while (currentID != ItemID::empty) {
		Item item;
		input >> r;
		input >> g;
		input >> b;
		item.color.r = static_cast<sf::Uint8>(r);
		item.color.g = static_cast<sf::Uint8>(g);
		item.color.b = static_cast<sf::Uint8>(b);
		input >> item.number;
		item.id = static_cast<ItemID>(currentID);
		AddItem(item);
		input >> currentID;
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			Item item;
			int id;
			input >> id;
			input >> r;
			input >> g;
			input >> b;
			item.color.r = static_cast<sf::Uint8>(r);
			item.color.g = static_cast<sf::Uint8>(g);
			item.color.b = static_cast<sf::Uint8>(b);
			input >> item.number;
			item.id = static_cast<ItemID>(id);
			if (item.id != ItemID::empty) {
				AddEquipment(i, j, item);
			}
		}
	}

	input >> currentID;
	while (currentID != ItemID::empty) {
		int available;
		input >> available;
		itemAvailability.insert(static_cast<ItemID>(available));
		input >> currentID;
	}

	input >> currentID;
	while (currentID != ItemID::empty) {
		bool available;
		input >> available;
		consumablesAvailability[static_cast<ItemID>(currentID)] = available;
		input >> currentID;
	}

	input.close();
}

void GameStatus::StoreToFile()
{
	std::ofstream output(savegamePath);

	output << cards << "\n";
	output << dice << "\n";
	output << levels[0] << "\n";
	output << levels[1] << "\n";
	output << levels[2] << "\n";

	for (auto& player : skillAcquired)
		for (auto& ability : player)
			for (auto& skill : ability)
				output << skill << "\n";

	for (auto& item : items) {
		output << static_cast<int>(item.id) << "\n";
		output << static_cast<int>(item.color.r) << "\n";
		output << static_cast<int>(item.color.g) << "\n";
		output << static_cast<int>(item.color.b) << "\n";
		output << item.number << "\n";
	}

	for (auto& item : consumables) {
		output << static_cast<int>(item.id) << "\n";
		output << static_cast<int>(item.color.r) << "\n";
		output << static_cast<int>(item.color.g) << "\n";
		output << static_cast<int>(item.color.b) << "\n";
		output << item.number << "\n";
	}
	output << static_cast<int>(ItemID::empty) << "\n";

	for (auto& player : equipment) {
		for (auto& item : player) {
			output << static_cast<int>(item.id) << "\n";
			output << static_cast<int>(item.color.r) << "\n";
			output << static_cast<int>(item.color.g) << "\n";
			output << static_cast<int>(item.color.b) << "\n";
			output << item.number << "\n";
		}
	}

	for (auto& item : itemAvailability) {
		output << static_cast<int>(item) << "\n";
	}

	output << static_cast<int>(ItemID::empty) << "\n";

	for (auto& item : consumablesAvailability) {
		output << static_cast<int>(item.first) << "\n";
		output << item.second << "\n";
	}

	output << static_cast<int>(ItemID::empty);
	output.close();
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
			Notify(ObserverNotificationGameStatus{ gameStatusEvents::consumableAdded, _item });

			if(consumablesAvailability[_item.id] == false)
				Notify(ObserverNotificationGameStatus{ gameStatusEvents::itemUnlocked, _item });
			consumablesAvailability[_item.id] = true;
		}
	}
	else {
		items.push_back(_item);
		Notify(ObserverNotificationGameStatus{ gameStatusEvents::equipmentAdded, _item });

		if (itemAvailability.find(_item.id) == itemAvailability.end())
			Notify(ObserverNotificationGameStatus{ gameStatusEvents::itemUnlocked, _item });
		itemAvailability.insert(_item.id);
	}
}


void GameStatus::RemoveItem(Item const &_item, bool _removeOne)
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
	equipmentStats[_player].stats += g_pObjectProperties->equipmentStats[_item.id].stats;

	if (g_pObjectProperties->equipmentStats[_item.id].missOnHighDamage)
		equipmentStats[_player].missOnHighDamage = true;

	if (g_pObjectProperties->equipmentStats[_item.id].healOnPass)
		equipmentStats[_player].healOnPass = true;
}

void GameStatus::RemoveEquipment(int _player, int _slot)
{
	equipmentStats[_player].stats -= g_pObjectProperties->equipmentStats[equipment[_player][_slot].id].stats;
	if (g_pObjectProperties->equipmentStats[equipment[_player][_slot].id].missOnHighDamage)
	{
		equipmentStats[_player].missOnHighDamage = false;
		for (int i = 0; i < 4; i++)
		{
			if (equipment[_player][i].id != ItemID::empty && i != _slot && g_pObjectProperties->equipmentStats[equipment[_player][i].id].missOnHighDamage)
				equipmentStats[_player].missOnHighDamage = true;
		}
	}
	if (g_pObjectProperties->equipmentStats[equipment[_player][_slot].id].healOnPass)
	{
		equipmentStats[_player].healOnPass = false;
		for (int i = 0; i < 4; i++)
		{
			if (equipment[_player][i].id != ItemID::empty && i != _slot && g_pObjectProperties->equipmentStats[equipment[_player][i].id].healOnPass)
				equipmentStats[_player].healOnPass = true;
		}
	}

	equipment[_player][_slot].id = ItemID::empty;
}

EquipmentProperties & GameStatus::GetEquipmentStats(int _player)
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