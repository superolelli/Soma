#include "ItemTooltip.hpp"

void ItemTooltip::Init()
{
	Tooltip::Init();

	itemId = ItemID::empty;
	equipmentStats = nullptr;
}

void ItemTooltip::SetItem(ItemID _id)
{
	itemId = _id;
}


void ItemTooltip::SetEquipmentStats(EquipmentProperties *_equipmentStats)
{
	equipmentStats = _equipmentStats;
}


void ItemTooltip::GenerateTooltipString(std::string & _tooltip)
{
	if (itemId < CONSUMABLE_ITEMS_START || itemId == ItemID::empty && equipmentStats != nullptr)
		GenerateTooltipStringEquipment(_tooltip);
	else
		GenerateTooltipStringConsumable(_tooltip);
}


void ItemTooltip::GenerateTooltipStringEquipment(std::string & _tooltip)
{
	if (itemId == ItemID::empty && equipmentStats == nullptr)
		return;

	EquipmentProperties itemProperties;
	if (equipmentStats != nullptr)
		itemProperties = *equipmentStats;
	else
		itemProperties = g_pObjectProperties->equipmentStats[itemId];

	_tooltip.append("*#ffa500 " + itemProperties.name + "*");

	if (itemProperties.name == "Karten" || itemProperties.name == "W�rfel")
		return;

	_tooltip.append("\n#white ");

	if (itemProperties.stats.initiative != 0)
		_tooltip.append(std::to_string(itemProperties.stats.initiative) + " Initiative\n");

	if (itemProperties.stats.armour != 0)
		_tooltip.append(std::to_string(itemProperties.stats.armour) + " R�stung\n");

	if (itemProperties.stats.damageMax != 0)
		_tooltip.append(std::to_string(itemProperties.stats.damageMax) + " Schaden\n");

	if (itemProperties.stats.criticalHit != 0)
		_tooltip.append(std::to_string(itemProperties.stats.criticalHit) + " Kritische Trefferchance\n");

	if (itemProperties.stats.dodge != 0)
		_tooltip.append(std::to_string(itemProperties.stats.dodge) + " Ausweichen\n");

	if (itemProperties.stats.precision != 0)
		_tooltip.append(std::to_string(itemProperties.stats.precision) + " Pr�zision\n");

	if (itemProperties.stats.maxHealth != 0)
		_tooltip.append(std::to_string(itemProperties.stats.maxHealth) + " Maximales Leben\n");

	if (_tooltip.back() == '\n')
		_tooltip.pop_back();
}



void ItemTooltip::GenerateTooltipStringConsumable(std::string & _tooltip)
{
	if (itemId == ItemID::empty)
		return;

	auto itemProperties = g_pObjectProperties->consumableStats[itemId - CONSUMABLE_ITEMS_START];

	_tooltip.append("*#ffa500 " + itemProperties.name + "*");

	_tooltip.append("\n#white ");

	if (itemProperties.heal != 0)
		_tooltip.append("#00aa00 Heilt " + std::to_string(itemProperties.heal) + " Leben\n");

	if (itemProperties.buff.duration != 0)
	{
		_tooltip.append("#white F�r " + std::to_string(itemProperties.buff.duration) + " Runden:\n");

		std::string sign("+");

		if (itemProperties.buff.stats.initiative != 0)
			_tooltip.append("\t#aaaadd " + sign + std::to_string(itemProperties.buff.stats.initiative) + " Initiative\n");

		if (itemProperties.buff.stats.armour != 0)
			_tooltip.append("\t#aaaadd " + sign + std::to_string(itemProperties.buff.stats.armour) + " R�stung\n");

		if (itemProperties.buff.stats.damageMax != 0)
			_tooltip.append("\t#aaaadd " + sign + std::to_string(itemProperties.buff.stats.damageMax) + " Schaden\n");

		if (itemProperties.buff.stats.criticalHit != 0)
			_tooltip.append("\t#aaaadd " + sign + std::to_string(itemProperties.buff.stats.criticalHit) + " Kritische Trefferchance\n");

		if (itemProperties.buff.stats.dodge != 0)
			_tooltip.append("\t#aaaadd " + sign + std::to_string(itemProperties.buff.stats.dodge) + " Ausweichen\n");

		if (itemProperties.buff.stats.precision != 0)
			_tooltip.append("\t#aaaadd " + sign + std::to_string(itemProperties.buff.stats.precision) + " Pr�zision\n");

		if (itemProperties.buff.stats.maxHealth != 0)
			_tooltip.append("\t#aaaadd " + sign + std::to_string(itemProperties.buff.stats.maxHealth) + " Maximales Leben\n");
	}

	if (_tooltip.back() == '\n')
		_tooltip.pop_back();
}
