#pragma once

#include "content/economy/EconomyContent.hpp"
#include "content/world/WorldContent.hpp"
#include "content/rules/RuleContent.hpp"
#include "gameplay/random/IRandom.hpp"

namespace soma::simulation
{
    enum class RuleRewardSource { lootable, battle };

    struct RuleLootContext
    {
        RuleRewardSource source{};
        std::string campaign;
        gameplay::LevelId level;
        std::vector<gameplay::ContentDefinitionId> enemies;
    };

    struct ItemGrant
    {
        gameplay::ItemInstance item;
        int quantity{1};

        auto operator<=>(const ItemGrant&) const = default;
    };

    struct LootBundle
    {
        std::vector<ItemGrant> items;
        std::optional<gameplay::RuleFragmentId> ruleFragment;
    };

    /// Draws item, currency, and rule-fragment rewards from inline loot definitions and campaign-default pools.
    class LootGenerator
    {
      public:
        LootGenerator(const content::EconomyContent& economy,
                      const content::RuleContent& rules,
                      gameplay::IRandom& random);

        [[nodiscard]] LootBundle generate(const content::LootTableDefinition& loot,
                                          std::string_view campaign,
                                          int difficulty,
                                          const RuleLootContext& ruleContext) const;
        [[nodiscard]] std::optional<gameplay::RuleFragmentId> rollRuleFragment(const RuleLootContext& context) const;

      private:
        [[nodiscard]] const gameplay::ItemDefinition& rollCampaignItem(
            const std::vector<const content::LootEntryDefinition*>& entries) const;
        [[nodiscard]] gameplay::ItemColor rollColor() const;
        [[nodiscard]] int rollQuantity(const gameplay::ItemDefinition& item, int difficulty) const;
        [[nodiscard]] static bool availableFrom(const content::RuleFragmentDefinition& fragment, const RuleLootContext& context);

        const content::EconomyContent& economy_;
        const content::RuleContent& rules_;
        gameplay::IRandom& random_;
    };
} // namespace soma::simulation
