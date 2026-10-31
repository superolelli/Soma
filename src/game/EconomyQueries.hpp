#pragma once

#include "content/economy/EconomyContent.hpp"
#include "game/CampaignState.hpp"

#include <vector>

namespace soma::game
{
    enum class ItemSort
    {
        name,
        color
    };

    enum class SkillAvailability
    {
        locked,
        available,
        acquired
    };

    struct InventoryEntry
    {
        gameplay::ItemInstance item;
        std::string name;
        gameplay::AssetId icon;
        int salePrice{};
    };

    struct ConsumableEntry
    {
        gameplay::ItemId item;
        std::string name;
        gameplay::AssetId icon;
        int quantity{};
        int salePrice{};
    };

    struct SkillEntry
    {
        gameplay::SkillId skill;
        std::string name;
        std::string description;
        gameplay::AssetId icon;
        int slot{};
        int cost{};
        SkillAvailability availability{};
    };

    /// Provides read-only, UI-ready views of inventory, skills, consumables, and shop state.
    class EconomyQueries
    {
      public:
        EconomyQueries(const CampaignState& state, const content::EconomyContent& content);

        [[nodiscard]] std::vector<InventoryEntry> inventory(ItemSort order) const;
        [[nodiscard]] std::vector<ConsumableEntry> consumables(ItemSort order) const;
        [[nodiscard]] const content::AbilitySetDefinition* abilitySet(const gameplay::ContentDefinitionId& player,
                                                                      std::size_t abilitySlot) const;
        [[nodiscard]] std::vector<SkillEntry> skills(const gameplay::ContentDefinitionId& player,
                                                     std::size_t abilitySlot,
                                                     std::size_t version) const;

      private:
        [[nodiscard]] SkillAvailability skillAvailability(const content::SkillDefinition& skill) const;

        const CampaignState& state_;
        const content::EconomyContent& content_;
    };
} // namespace soma::game
