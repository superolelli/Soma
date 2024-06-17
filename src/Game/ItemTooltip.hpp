#pragma once
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include "../Framework/Graphics/Sprite.hpp"
#include "../Framework/Gameengine.hpp"
#include "GameStatus.hpp"
#include "Tooltip.hpp"


struct EquipmentProperties;

class ItemTooltip : public Tooltip
{
public:

	ItemTooltip();

	void SetItem(ItemID _id);

	void SetEquipmentStats(EquipmentProperties *_equipmentStats);

private:
	ItemID itemId;
	EquipmentProperties *equipmentStats;

	void GenerateTooltipString(std::string &_tooltip) override;

	void GenerateTooltipStringEquipment(std::string &_tooltip);
	void GenerateTooltipStringConsumable(std::string &_tooltip);
};