#pragma once

#include <unordered_map>
#include <SFML\Graphics.hpp>
#include "CombatantAttributes.hpp"
#include "Buff.hpp"

enum ItemID { empty = -1, cards, dice, iron_plate, sombrero, binocular, bible, cowboyhat, lasso, ammo, knife, tomahawk, sherrifstar, boot, horseshoe, canteen, pepperbox, scope, ironmask, castingknives,
			duelPistol, monocle, poisonRing, pocketWatch, tome, topHat, crystalBall, blackPearl, gloves, brokenMirror,
			beer, tequila, whisky, drinkOfPower, absinth, flashPowder, numberOfItems };

const int EQUIPMENT_ITEMS_START = 2;
const int CONSUMABLE_ITEMS_START = int(beer);

//just for convenience
const int NUMBER_OF_EQUIPMENT_ITEMS = CONSUMABLE_ITEMS_START;
const int NUMBER_OF_CONSUMABLE_ITEMS = numberOfItems - CONSUMABLE_ITEMS_START;

extern std::unordered_map<std::string, ItemID> itemIdentifierMap; //is filled in ObjectPropertiesManager

const int EQUIPMENT_PRICE_PER_LEVEL = 40;

struct Item
{
	ItemID id;
	sf::Color color;
	int number;
};

struct ItemProperties
{
	sf::String name;
	int level;
	int price;
};

struct EquipmentProperties : public ItemProperties
{
	CombatantAttributes stats;
	bool missOnHighDamage;
	bool healOnPass;
};

struct ConsumableProperties : public ItemProperties
{
	int heal;
	Buff buff;
};