#include "ItemTooltip.hpp"
#include "Resources\ObjectPropertiesManager.hpp"
#include "Resources/Colors.hpp"

ItemTooltip::ItemTooltip()
	: Tooltip()
	, itemId(ItemID::empty)
	, equipmentStats(nullptr)
{
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

	_tooltip.append("*" + Colors::NAME + itemProperties.name + "*");

	if (itemProperties.name == "Karten" || itemProperties.name == "Würfel")
		return;

	_tooltip.append("\n#white ");

	if (itemProperties.missOnHighDamage)
		_tooltip.append("Wenn mindestens 10 Schaden\nauf einmal bekommen:\nErhalte einen Fehlschuss");

	if (itemProperties.healOnPass)
		_tooltip.append("Heile 3 Leben und 3 Müdigkeit beim Passen");

	AppendCombatantAttributesTooltip(_tooltip, itemProperties.stats, "", "", Colors::DEFAULT);

	if (_tooltip.back() == '\n')
		_tooltip.pop_back();
}



void ItemTooltip::GenerateTooltipStringConsumable(std::string & _tooltip)
{
	if (itemId == ItemID::empty)
		return;

	auto itemProperties = g_pObjectProperties->consumableStats[itemId - CONSUMABLE_ITEMS_START];

	_tooltip.append("*" + Colors::NAME + itemProperties.name + "*\n");

	if (itemProperties.heal != 0)
		_tooltip.append(Colors::HEALING + "Heilt " + std::to_string(itemProperties.heal) + " Leben\n");

	if (itemProperties.buff.duration != 0)
	{
		_tooltip.append(Colors::DEFAULT + "Für " + DurationString(itemProperties.buff.duration) + ":\n");
		AppendCombatantAttributesTooltip(_tooltip, itemProperties.buff.stats, "\t", "+");
	}

	if (_tooltip.back() == '\n')
		_tooltip.pop_back();
}
