#include "ItemTooltip.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

void ItemTooltip::Init(CGameEngine * _engine)
{
	Tooltip::Init(_engine);
	itemID = ItemID::iron_plate;
}

void ItemTooltip::SetItemID(ItemID _id)
{
	itemID = _id;
}


void ItemTooltip::GenerateTooltipString(std::string & _tooltip)
{
	_tooltip.append("*#ffa500 " + g_pObjectProperties->itemStats[itemID].first + "*\n");
	_tooltip.append("#white ");

	if (g_pObjectProperties->itemStats[itemID].second.initiative != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].second.initiative) + " Initiative\n");

	if (g_pObjectProperties->itemStats[itemID].second.armour != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].second.armour) + " Rüstung\n");

	if (g_pObjectProperties->itemStats[itemID].second.damageMax != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].second.damageMax) + " Schaden\n");

	if (g_pObjectProperties->itemStats[itemID].second.criticalHit != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].second.criticalHit) + " Kritische Trefferchance\n");

	if (g_pObjectProperties->itemStats[itemID].second.dodge != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].second.dodge) + " Ausweichen\n");

	if (g_pObjectProperties->itemStats[itemID].second.precision != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].second.precision) + " Präzision\n");

	if (g_pObjectProperties->itemStats[itemID].second.maxHealth != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].second.maxHealth) + " Maximales Leben\n");

	if (g_pObjectProperties->itemStats[itemID].second.attributes.constitution != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].second.attributes.constitution) + " Konstitution\n");

	if (g_pObjectProperties->itemStats[itemID].second.attributes.dexterity != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].second.attributes.dexterity) + " Geschicklichkeit\n");

	if (g_pObjectProperties->itemStats[itemID].second.attributes.strength != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].second.attributes.strength) + " Stärke\n");

	if (g_pObjectProperties->itemStats[itemID].second.attributes.speed != 0)
		_tooltip.append(std::to_string(g_pObjectProperties->itemStats[itemID].second.attributes.speed) + " Geschwindigkeit\n");

	_tooltip.pop_back();
}
