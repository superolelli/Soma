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

const int CONSUMABLE_ITEMS_LIMIT = 5;
const int MAX_FATIGUE_VALUE = 100;

class GameStatus : public CSubject
{
public:

	void Init();

	int GetDiceAmount();
	int GetCardsAmount();
	void AddDice(int _amount);
	void AddCards(int _amount);
	void RemoveDice(int _amount);
	void RemoveCards(int _amount);

	void AddFatigue(int _fatigue);
	void RemoveFatigue(int _fatigue);

	int* GetFatiguePtr() { return &fatigue; }
	int* GetMaxFatiguePtr() { return &maxFatigue; }

	bool IsSkillAcquired(int player, int ability, int skill);
	void AcquireSkill(int player, int ability, int skill);

	void AddItem(Item _item);
	void RemoveItem(Item &_item, bool _removeOne = true);
	const std::vector<Item> &GetItems() { return items; }
	const std::vector<Item> &GetConsumables() { return consumables; }

	void AddEquipment(int _player, int _slot, Item _item);
	void RemoveEquipment(int _player, int _slot);
	CombatantAttributes &GetEquipmentStats(int _player);

	void SetDiamondStats(int _player, CombatantAttributes &_stats);
	CombatantAttributes &GetDiamondStats(int _player);

	std::unordered_map<ItemID, bool> &GetConsumablesAvailability() { return consumablesAvailability; }

	int bangLevel;
	int kutschfahrtLevel;
	int tichuLevel;

private:
	int dice;
	int cards;

	int fatigue;
	int maxFatigue;

	bool skillAcquired[4][4][8];

	std::vector<Item> items;
	std::vector<Item> consumables;

	Item equipment[4][4];
	CombatantAttributes equipmentStats[4];

	CombatantAttributes diamondStats[4];

	std::unordered_map<ItemID, bool> consumablesAvailability;
};