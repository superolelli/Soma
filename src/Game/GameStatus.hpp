#pragma once

#include "Resources\TextureManager.hpp"
#include "Resources\FontManager.hpp"
#include "CombatantStatus.hpp"
#include "Item.hpp"

#include "../Framework/Gameengine.hpp"
#include "../Framework/Graphics/Sprite.hpp"

#include "../Framework/Patterns/Subject.hpp"
#include "ObserverNotificationGameStatus.h"

#include <functional>
#include <set>

const int CONSUMABLE_ITEMS_LIMIT = 5;

#define g_pGameStatus GameStatus::Get()
class GameStatus : public CSubject, public TSingleton<GameStatus>
{
public:

	GameStatus();

	void Reset(const std::string &filepath);
	void LoadFromFile(const std::string &filepath);
	void StoreToFile();

	int GetDiceAmount();
	int GetCardsAmount();
	void AddDice(int _amount);
	void AddCards(int _amount);
	void RemoveDice(int _amount);
	void RemoveCards(int _amount);

	bool IsSkillAcquired(int player, int ability, int skill);
	void AcquireSkill(int player, int ability, int skill);

	void AddItem(Item _item);
	void RemoveItem(Item const &_item, bool _removeOne = true);
	const std::vector<Item> &GetItems() { return items; }
	const std::vector<Item> &GetConsumables() { return consumables; }

	void AddEquipment(int _player, int _slot, Item _item);
	void RemoveEquipment(int _player, int _slot);
	Item &GetEquipment(int _player, int _slot) { return equipment[_player][_slot]; }
	EquipmentProperties &GetEquipmentStats(int _player);

	void SetDiamondStats(int _player, CombatantAttributes &_stats);
	CombatantAttributes &GetDiamondStats(int _player);

	std::unordered_map<ItemID, bool> &GetConsumablesAvailability() { return consumablesAvailability; }
	std::set<ItemID>& GetItemAvailability() { return itemAvailability; }


	int levels[3];

private:
	std::string savegamePath;

	int dice;
	int cards;

	bool skillAcquired[4][4][8];

	std::vector<Item> items;
	std::vector<Item> consumables;

	Item equipment[4][4];
	EquipmentProperties equipmentStats[4];

	CombatantAttributes diamondStats[4];

	std::set<ItemID> itemAvailability;
	std::unordered_map<ItemID, bool> consumablesAvailability;
};