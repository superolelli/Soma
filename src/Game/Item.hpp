#pragma once

#include <unordered_map>
#include <SFML\Graphics.hpp>

enum ItemID { empty = -1, cards, dice, iron_plate, sombrero, binocular, bible, cowboyhat, lasso, ammo, knife, tomahawk, sherrifstar, boot, horseshoe, canteen, pepperbox,
	beer, tequila, whisky, numberOfItems };

const int EQUIPMENT_ITEMS_START = 2;
const int CONSUMABLE_ITEMS_START = int(beer);

//just for convenience
const int NUMBER_OF_EQUIPMENT_ITEMS = CONSUMABLE_ITEMS_START;
const int NUMBER_OF_CONSUMABLE_ITEMS = numberOfItems - CONSUMABLE_ITEMS_START;

static std::unordered_map<std::string, ItemID> itemIdentifierMap; //is filled in ObjectPropertiesManager

const int EQUIPMENT_PRICE_PER_LEVEL = 40;

struct Item
{
	ItemID id;
	sf::Color color;
	int number;
};