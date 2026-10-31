#include "simulation/rewards/LootGenerator.hpp"

#include "gameplay/items/ItemRarity.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

namespace soma::simulation
{
    LootGenerator::LootGenerator(const content::EconomyContent& economy,
                                 const content::RuleContent& rules,
                                 gameplay::IRandom& random)
        : economy_(economy), rules_(rules), random_(random)
    {
    }

    LootBundle LootGenerator::generate(const content::LootTableDefinition& loot,
                                       std::string_view campaign,
                                       int difficulty,
                                       const RuleLootContext& ruleContext) const
    {
        std::vector<content::LootEntryDefinition> defaultEntries;
        if (loot.entries.empty())
        {
            defaultEntries.reserve(economy_.items.size());
            for (const auto& [id, item] : economy_.items)
            {
                if (std::ranges::find(item.campaigns, campaign) != item.campaigns.end())
                {
                    defaultEntries.push_back({id, 1});
                }
            }
            std::ranges::sort(defaultEntries, {}, [](const auto& entry) { return entry.item.value(); });
        }
        const auto& configuredEntries = loot.entries.empty() ? defaultEntries : loot.entries;

        std::vector<const content::LootEntryDefinition*> eligible;
        std::vector<const content::LootEntryDefinition*> eligibleItems;
        int totalWeight{};
        int itemWeight{};
        for (const auto& entry : configuredEntries)
        {
            const auto& item = economy_.items.at(entry.item);
            if (std::ranges::find(item.campaigns, campaign) == item.campaigns.end())
            {
                continue;
            }
            eligible.push_back(&entry);
            totalWeight += entry.weight;
            if (item.category != gameplay::ItemCategory::currency)
            {
                eligibleItems.push_back(&entry);
                itemWeight += entry.weight;
            }
        }
        if (eligible.empty())
        {
            throw std::runtime_error("Loot table has no entries eligible for this campaign");
        }

        auto rolls = loot.minimumRolls;
        while (rolls < loot.maximumRolls && random_.chance(loot.additionalRollChance))
        {
            ++rolls;
        }
        LootBundle result;
        result.items.reserve(static_cast<std::size_t>(rolls));
        for (auto roll = 0; roll < rolls; ++roll)
        {
            auto selectedWeight = random_.between(1, totalWeight);
            const gameplay::ItemDefinition* selected{};
            if (selectedWeight <= itemWeight)
            {
                selected = &rollCampaignItem(eligibleItems);
            }
            else
            {
                selectedWeight -= itemWeight;
                for (const auto* entry : eligible)
                {
                    const auto& item = economy_.items.at(entry->item);
                    if (item.category != gameplay::ItemCategory::currency)
                    {
                        continue;
                    }
                    selectedWeight -= entry->weight;
                    if (selectedWeight <= 0)
                    {
                        selected = &item;
                        break;
                    }
                }
            }
            if (selected == nullptr)
            {
                throw std::runtime_error("Could not select a loot entry");
            }
            const auto& item = *selected;
            result.items.push_back({{item.id, item.category == gameplay::ItemCategory::equipment ? rollColor() : gameplay::ItemColor{}},
                                    rollQuantity(item, difficulty)});
        }
        result.ruleFragment = rollRuleFragment(ruleContext);
        return result;
    }

    const gameplay::ItemDefinition& LootGenerator::rollCampaignItem(
        const std::vector<const content::LootEntryDefinition*>& entries) const
    {
        std::array<std::vector<const content::LootEntryDefinition*>, gameplay::maximumItemRarityLevel> candidatesByLevel;
        for (const auto* entry : entries)
        {
            const auto& item = economy_.items.at(entry->item);
            candidatesByLevel.at(static_cast<std::size_t>(item.level - gameplay::minimumItemRarityLevel)).push_back(entry);
        }
        for (auto& candidates : candidatesByLevel)
        {
            std::ranges::sort(candidates, {}, [](const auto* entry) { return entry->item.value(); });
        }

        auto totalWeight = 0;
        for (auto level = gameplay::minimumItemRarityLevel; level <= gameplay::maximumItemRarityLevel; ++level)
        {
            if (!candidatesByLevel.at(static_cast<std::size_t>(level - 1)).empty())
            {
                totalWeight += gameplay::itemRarityWeight(level);
            }
        }
        if (totalWeight == 0)
        {
            throw std::runtime_error("Campaign has no items eligible for loot");
        }

        auto selectedWeight = random_.between(1, totalWeight);
        for (auto level = gameplay::minimumItemRarityLevel; level <= gameplay::maximumItemRarityLevel; ++level)
        {
            auto& candidates = candidatesByLevel.at(static_cast<std::size_t>(level - 1));
            if (candidates.empty())
            {
                continue;
            }
            selectedWeight -= gameplay::itemRarityWeight(level);
            if (selectedWeight <= 0)
            {
                auto entryWeight = 0;
                for (const auto* entry : candidates)
                {
                    entryWeight += entry->weight;
                }
                auto selectedEntryWeight = random_.between(1, entryWeight);
                for (const auto* entry : candidates)
                {
                    selectedEntryWeight -= entry->weight;
                    if (selectedEntryWeight <= 0)
                    {
                        return economy_.items.at(entry->item);
                    }
                }
            }
        }
        throw std::runtime_error("Could not select an item rarity");
    }

    std::optional<gameplay::RuleFragmentId> LootGenerator::rollRuleFragment(const RuleLootContext& context) const
    {
        std::vector<gameplay::RuleFragmentId> eligible;
        for (const auto& [id, fragment] : rules_.fragments)
            if (availableFrom(fragment, context)) eligible.push_back(id);
        if (eligible.empty() || !random_.chance(rules_.lootChance)) return std::nullopt;
        std::ranges::sort(eligible, {}, &gameplay::RuleFragmentId::value);
        return eligible.at(static_cast<std::size_t>(random_.between(0, static_cast<int>(eligible.size()) - 1)));
    }

    bool LootGenerator::availableFrom(const content::RuleFragmentDefinition& fragment, const RuleLootContext& context)
    {
        const auto& source = context.source == RuleRewardSource::lootable ? fragment.acquisition.lootables
                                                                          : fragment.acquisition.battles;
        if (!source.enabled) return false;
        const auto campaignMatches = source.campaigns.empty() ||
                                     std::ranges::find(source.campaigns, context.campaign) != source.campaigns.end();
        const auto levelMatches = source.levels.empty() || std::ranges::find(source.levels, context.level) != source.levels.end();
        const auto enemyMatches = source.enemies.empty() || std::ranges::any_of(context.enemies, [&source](const auto& enemy) {
            return std::ranges::find(source.enemies, enemy) != source.enemies.end();
        });
        return campaignMatches && levelMatches && enemyMatches;
    }

    gameplay::ItemColor LootGenerator::rollColor() const
    {
        switch (random_.between(0, 7))
        {
        case 0: return {128, 0, 0};
        case 1: return {0, 128, 0};
        case 2: return {0, 0, 128};
        case 3: return {128, 128, 0};
        default: return {};
        }
    }

    int LootGenerator::rollQuantity(const gameplay::ItemDefinition& item, int difficulty) const
    {
        const auto factor = static_cast<int>(std::ceil(static_cast<double>(difficulty) / 2.0));
        if (item.id == gameplay::ItemId{"currency.cards"})
        {
            return random_.between(0, 3) * factor + 1;
        }
        if (item.id == gameplay::ItemId{"currency.dice"})
        {
            return random_.between(0, 1) * factor + 1;
        }
        if (item.category == gameplay::ItemCategory::consumable)
        {
            return random_.between(1, 3);
        }
        return 1;
    }
} // namespace soma::simulation
