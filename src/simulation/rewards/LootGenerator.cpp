#include "simulation/rewards/LootGenerator.hpp"

#include "gameplay/items/ItemRarity.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

namespace soma::simulation
{
    LootGenerator::LootGenerator(const content::WorldContent& world,
                                 const content::EconomyContent& economy,
                                 const content::RuleContent& rules,
                                 gameplay::IRandom& random)
        : world_(world), economy_(economy), rules_(rules), random_(random)
    {
    }

    LootBundle LootGenerator::generate(const gameplay::LootTableId& tableId,
                                       std::string_view campaign,
                                       int difficulty,
                                       const RuleLootContext& ruleContext) const
    {
        const auto& table = world_.lootTables.at(tableId);
        std::vector<const content::LootEntryDefinition*> eligible;
        int totalWeight{};
        for (const auto& entry : table.entries)
        {
            const auto& item = economy_.items.at(entry.item);
            if (std::ranges::find(item.campaigns, campaign) == item.campaigns.end())
            {
                continue;
            }
            eligible.push_back(&entry);
            totalWeight += entry.weight;
        }
        if (eligible.empty())
        {
            throw std::runtime_error("Loot table has no entries eligible for this campaign");
        }

        auto rolls = table.minimumRolls;
        while (rolls < table.maximumRolls && random_.chance(table.additionalRollChance))
        {
            ++rolls;
        }
        LootBundle result;
        result.items.reserve(static_cast<std::size_t>(rolls));
        for (auto roll = 0; roll < rolls; ++roll)
        {
            auto selectedWeight = random_.between(1, totalWeight);
            const content::LootEntryDefinition* selected{};
            for (const auto* entry : eligible)
            {
                selectedWeight -= entry->weight;
                if (selectedWeight <= 0)
                {
                    selected = entry;
                    break;
                }
            }
            const auto& selectedItem = economy_.items.at(selected->item);
            const auto& item = selectedItem.category == gameplay::ItemCategory::currency ? selectedItem : rollCampaignItem(campaign);
            result.items.push_back({{item.id, item.category == gameplay::ItemCategory::equipment ? rollColor() : gameplay::ItemColor{}},
                                    rollQuantity(item, difficulty)});
        }
        result.ruleFragment = rollRuleFragment(ruleContext);
        return result;
    }

    const gameplay::ItemDefinition& LootGenerator::rollCampaignItem(std::string_view campaign) const
    {
        std::array<std::vector<const gameplay::ItemDefinition*>, gameplay::maximumItemRarityLevel> candidatesByLevel;
        for (const auto& [id, item] : economy_.items)
        {
            static_cast<void>(id);
            const auto supportsCampaign = std::ranges::find(item.campaigns, campaign) != item.campaigns.end();
            if (item.category == gameplay::ItemCategory::currency || !supportsCampaign)
            {
                continue;
            }
            candidatesByLevel.at(static_cast<std::size_t>(item.level - gameplay::minimumItemRarityLevel)).push_back(&item);
        }
        for (auto& candidates : candidatesByLevel)
        {
            std::ranges::sort(candidates, {}, [](const auto* item) { return item->id.value(); });
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
                const auto index = random_.between(0, static_cast<int>(candidates.size()) - 1);
                return *candidates.at(static_cast<std::size_t>(index));
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
