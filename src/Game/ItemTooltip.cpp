#include "ItemTooltip.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

void ItemTooltip::Init()
{
	Tooltip::Init();
	itemID = ItemID::iron_plate;
}

void ItemTooltip::SetItemID(ItemID _id)
{
	itemID = _id;
}


void ItemTooltip::GenerateTooltipString(std::string & _tooltip)
{
	_tooltip.append("*#ffa500 " + g_pObjectProperties->itemStats[itemID].name + "*");

	if (itemID < EQUIPMENT_ITEMS_START)
		return;

	_tooltip.append("\n#white ");

	if (g_pObjectProperties->itemStats[itemID].stats.initiative != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].stats.initiative) + " Initiative\n");

	if (g_pObjectProperties->itemStats[itemID].stats.armour != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].stats.armour) + " Rüstung\n");

	if (g_pObjectProperties->itemStats[itemID].stats.damageMax != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].stats.damageMax) + " Schaden\n");

	if (g_pObjectProperties->itemStats[itemID].stats.criticalHit != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].stats.criticalHit) + " Kritische Trefferchance\n");

	if (g_pObjectProperties->itemStats[itemID].stats.dodge != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].stats.dodge) + " Ausweichen\n");

	if (g_pObjectProperties->itemStats[itemID].stats.precision != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].stats.precision) + " Präzision\n");

	if (g_pObjectProperties->itemStats[itemID].stats.maxHealth != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].stats.maxHealth) + " Maximales Leben\n");

	if (g_pObjectProperties->itemStats[itemID].stats.attributes.constitution != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].stats.attributes.constitution) + " Konstitution\n");

	if (g_pObjectProperties->itemStats[itemID].stats.attributes.dexterity != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].stats.attributes.dexterity) + " Geschicklichkeit\n");

	if (g_pObjectProperties->itemStats[itemID].stats.attributes.strength != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].stats.attributes.strength) + " Stärke\n");

	if (g_pObjectProperties->itemStats[itemID].stats.attributes.speed != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].stats.attributes.speed) + " Geschwindigkeit\n");

	_tooltip.pop_back();
}
