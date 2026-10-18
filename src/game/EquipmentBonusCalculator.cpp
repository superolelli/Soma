#include "game/EquipmentBonusCalculator.hpp"

#include <algorithm>

namespace soma::game
{
    namespace
    {
        constexpr std::array<std::string_view, 4> slotNames{"north", "east", "south", "west"};

        void add(gameplay::AttributeBonuses& destination, const gameplay::AttributeBonuses& source)
        {
            destination.armour += source.armour;
            destination.maximumHealth += source.maximumHealth;
            destination.damageMinimum += source.damageMinimum;
            destination.damageMaximum += source.damageMaximum;
            destination.initiative += source.initiative;
            destination.criticalHitChance += source.criticalHitChance;
            destination.dodge += source.dodge;
            destination.precision += source.precision;
            destination.healing += source.healing;
            destination.sleepResistance += source.sleepResistance;
            destination.confusionResistance += source.confusionResistance;
            destination.debuffResistance += source.debuffResistance;
            destination.decayResistance += source.decayResistance;
        }

        gameplay::ItemColor diamondColor(const std::array<EquipmentConnection, 4>& connections)
        {
            int red{};
            int green{};
            int blue{};
            for (const auto& connection : connections)
            {
                if (!connection.active)
                {
                    continue;
                }
                red = std::min(255, red + connection.color.red / 2);
                green = std::min(255, green + connection.color.green / 2);
                blue = std::min(255, blue + connection.color.blue / 2);
            }
            return {static_cast<std::uint8_t>(red), static_cast<std::uint8_t>(green), static_cast<std::uint8_t>(blue)};
        }
    } // namespace

    EquipmentBonusCalculator::EquipmentBonusCalculator(const content::EconomyContent& content)
        : content_(content)
    {
    }

    EquipmentBonusSummary EquipmentBonusCalculator::calculate(const CampaignState& state,
                                                                const gameplay::ContentDefinitionId& player) const
    {
        EquipmentBonusSummary result;
        std::array<const gameplay::ItemInstance*, 4> equipped{};
        for (std::size_t slot = 0; slot < equipped.size(); ++slot)
        {
            equipped.at(slot) = state.equipped({player, gameplay::EquipmentSlotId{"equipment." + std::string{slotNames.at(slot)}}});
            if (!equipped.at(slot))
            {
                continue;
            }
            const auto& definition = content_.items.at(equipped.at(slot)->definition);
            const auto& equipment = std::get<gameplay::EquipmentItem>(definition.properties);
            add(result.equipmentBonuses, equipment.bonuses);
            result.addsMissOnHighDamage = result.addsMissOnHighDamage || equipment.addsMissOnHighDamage;
            result.healsOnPass = result.healsOnPass || equipment.healsOnPass;
            if (equipment.battleEffect.has_value())
            {
                result.battleEffects.push_back({definition.id, *equipment.battleEffect});
            }
        }
        for (std::size_t slot = 0; slot < equipped.size(); ++slot)
        {
            const auto right = (slot + 1) % equipped.size();
            if (equipped.at(slot) && equipped.at(right) && gameplay::isSupportedItemColor(equipped.at(slot)->color) &&
                equipped.at(slot)->color == equipped.at(right)->color)
            {
                result.connections.at(slot) = {true, equipped.at(slot)->color};
            }
        }
        result.diamondColor = diamondColor(result.connections);
        const auto matchingBonus = std::ranges::find_if(content_.diamondBonuses, [&result](const auto& definition) {
            if (definition.color != result.diamondColor)
            {
                return false;
            }
            return !definition.requiredConnectionColor ||
                   std::ranges::any_of(result.connections, [&definition](const auto& connection) {
                       return connection.active && connection.color == *definition.requiredConnectionColor;
                   });
        });
        if (matchingBonus != content_.diamondBonuses.end())
        {
            result.diamondBonuses = matchingBonus->bonuses;
        }
        result.combinedBonuses = result.equipmentBonuses;
        add(result.combinedBonuses, result.diamondBonuses);
        return result;
    }
} // namespace soma::game
