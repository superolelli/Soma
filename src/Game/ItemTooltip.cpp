#include "ItemTooltip.hpp"

void ItemTooltip::Init()
{
	Tooltip::Init();
	itemProperties = nullptr;
}

void ItemTooltip::SetItem(ItemProperties *_itemProperties)
{
	itemProperties = _itemProperties;
}


void ItemTooltip::GenerateTooltipString(std::string & _tooltip)
{
	if (itemProperties == nullptr)
		return;

	_tooltip.append("*#ffa500 " + itemProperties->name + "*");

	if (itemProperties->name == "Karten" || itemProperties->name == "Würfel")
		return;

	_tooltip.append("\n#white ");

	if (itemProperties->stats.initiative != 0)
		_tooltip.append(std::to_string(itemProperties->stats.initiative) + " Initiative\n");

	if (itemProperties->stats.armour != 0)
		_tooltip.append(std::to_string(itemProperties->stats.armour) + " Rüstung\n");

	if (itemProperties->stats.damageMax != 0)
		_tooltip.append(std::to_string(itemProperties->stats.damageMax) + " Schaden\n");

	if (itemProperties->stats.criticalHit != 0)
		_tooltip.append(std::to_string(itemProperties->stats.criticalHit) + " Kritische Trefferchance\n");

	if (itemProperties->stats.dodge != 0)
		_tooltip.append(std::to_string(itemProperties->stats.dodge) + " Ausweichen\n");

	if (itemProperties->stats.precision != 0)
		_tooltip.append(std::to_string(itemProperties->stats.precision) + " Präzision\n");

	if (itemProperties->stats.maxHealth != 0)
		_tooltip.append(std::to_string(itemProperties->stats.maxHealth) + " Maximales Leben\n");

	if (itemProperties->stats.attributes.constitution != 0)
		_tooltip.append(std::to_string(itemProperties->stats.attributes.constitution) + " Konstitution\n");

	if (itemProperties->stats.attributes.dexterity != 0)
		_tooltip.append(std::to_string(itemProperties->stats.attributes.dexterity) + " Geschicklichkeit\n");

	if (itemProperties->stats.attributes.strength != 0)
		_tooltip.append(std::to_string(itemProperties->stats.attributes.strength) + " Stärke\n");

	if (itemProperties->stats.attributes.speed != 0)
		_tooltip.append(std::to_string(itemProperties->stats.attributes.speed) + " Geschwindigkeit\n");

	_tooltip.pop_back();
}
