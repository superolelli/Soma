#pragma once

#include "Resources\TextureManager.hpp"
#include "Resources\FontManager.hpp"
#include "CombatantStatus.hpp"
#include "Item.hpp"

#include "../Framework/Gameengine.hpp"
#include "../Framework/Graphics/Sprite.hpp"

#include <functional>

const int CONSUMABLE_ITEMS_LIMIT = 5;

class GameStatus
{
public:

	void Init(CGameEngine *_engine);
	void RenderStatusBar();

	void SetPos(int _x, int _y);
	int GetDiceAmount();
	int GetCardsAmount();
	void AddDice(int _amount);
	void AddCards(int _amount);
	void RemoveDice(int _amount);
	void RemoveCards(int _amount);

	bool IsSkillAcquired(int player, int ability, int skill);
	void AcquireSkill(int player, int ability, int skill);

	void AddItem(Item _item, bool _triggerCallback = true);
	void RemoveItem(Item &_item);
	const std::vector<Item> &GetItems() { return items; }
	void SetOnItemAddedCallback(std::function<void(Item)> _callback);
	void SetOnConsumableAddedCallback(std::function<void(Item, bool)> _callback);

	void AddEquipment(int _player, int _slot, Item _item);
	void RemoveEquipment(int _player, int _slot);
	CombatantStats &GetEquipmentStats(int _player);

	void SetDiamondStats(int _player, CombatantStats &_stats);
	CombatantStats &GetDiamondStats(int _player);

	int bangLevel;
	int kutschfahrtLevel;
	int tichuLevel;

private:
	CGameEngine *engine;

	CSprite statusBar;
	sf::Text diceAmountText;
	sf::Text cardsAmountText;

	int dice;
	int cards;

	bool skillAcquired[4][4][8];

	std::vector<Item> items;
	std::vector<Item> consumables;

	std::function<void(Item)> OnItemAddedCallback;
	std::function<void(Item, bool)> OnConsumableAddedCallback;

	Item equipment[4][4];
	CombatantStats equipmentStats[4];

	CombatantStats diamondStats[4];

	void PositionComponentsRelativeToPanel();
};