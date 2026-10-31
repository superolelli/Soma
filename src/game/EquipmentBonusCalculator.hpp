#pragma once

#include "content/economy/EconomyContent.hpp"
#include "game/CampaignState.hpp"

#include <array>

namespace soma::game
{
    struct EquipmentConnection
    {
        bool active{};
        gameplay::ItemColor color;
    };

    struct EquipmentBonusSummary
    {
        std::array<EquipmentConnection, 4> connections;
        gameplay::ItemColor diamondColor;
        gameplay::AttributeBonuses equipmentBonuses;
        gameplay::AttributeBonuses diamondBonuses;
        gameplay::AttributeBonuses combinedBonuses;
        bool addsMissOnHighDamage{};
        bool healsOnPass{};
        std::vector<gameplay::EquippedBattleEffect> battleEffects;
    };

    /// Combines equipped item bonuses and color connections into player combat attributes.
    class EquipmentBonusCalculator
    {
      public:
        explicit EquipmentBonusCalculator(const content::EconomyContent& content);

        [[nodiscard]] EquipmentBonusSummary calculate(const CampaignState& state,
                                                       const gameplay::ContentDefinitionId& player) const;

      private:
        const content::EconomyContent& content_;
    };
} // namespace soma::game
