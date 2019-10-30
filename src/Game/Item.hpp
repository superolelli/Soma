#pragma once

#include <unordered_map>

enum ItemID { empty = -1, cards, dice, iron_plate, sombrero };

const int EQUIPMENT_ITEMS_START = 2;

static std::unordered_map<std::string, ItemID> itemIdentifierMap; //is filled in ObjectPropertiesManager

struct Item
{
	ItemID id;
	sf::Color color;
	int number;
};