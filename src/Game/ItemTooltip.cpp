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

	if (itemProperties.name == "Karten" || itemProperties.name == "Würfel")
		return;

	_tooltip.append("\n#white ");

	AppendCombatantAttributesTooltip(_tooltip, itemProperties.stats, "", "", "#white ");

	if (_tooltip.back() == '\n')
		_tooltip.pop_back();
}



void ItemTooltip::GenerateTooltipStringConsumable(std::string & _tooltip)
{
	if (itemId == ItemID::empty)
		return;

	auto itemProperties = g_pObjectProperties->consumableStats[itemId - CONSUMABLE_ITEMS_START];

	_tooltip.append("*#ffa500 " + itemProperties.name + "*\n");

	if (itemProperties.heal != 0)
		_tooltip.append("#00aa00 Heilt " + std::to_string(itemProperties.heal) + " Leben\n");

	if (itemProperties.buff.duration != 0)
	{
		_tooltip.append("#white Für " + std::to_string(itemProperties.buff.duration) + " Runden:\n");
		AppendCombatantAttributesTooltip(_tooltip, itemProperties.buff.stats, "\t", "+");
	}

	if (_tooltip.back() == '\n')
		_tooltip.pop_back();
}
